#!/usr/bin/env python3
"""Local build and benchmark workflow for Performance Ninja labs."""

from __future__ import annotations

import argparse
import fcntl
import io
import json
import os
import re
import shutil
import statistics
import subprocess
import sys
import tarfile
from collections import defaultdict
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Iterable, Sequence


BENCHMARK_VERSION = "v1.9.5"
REPO_ROOT = Path(__file__).resolve().parents[1]
TIME_SCALES = {"s": 1.0, "ms": 1e-3, "us": 1e-6, "ns": 1e-9}
MIN_TIME_PATTERN = re.compile(r"^(?:\d+(?:\.\d+)?|\.\d+)s$")


class PnError(RuntimeError):
    """An actionable error caused by the local Perf Ninja setup or input."""


@dataclass(frozen=True)
class Measurement:
    real_seconds: float
    cpu_seconds: float


@dataclass(frozen=True)
class BenchmarkSummary:
    name: str
    baseline_median: float
    solution_median: float
    speedup: float
    improvement_percent: float
    baseline_noise_percent: float
    solution_noise_percent: float


@dataclass(frozen=True)
class LabLayout:
    repo: Path
    lab: Path

    @property
    def relative_lab(self) -> Path:
        return self.lab.relative_to(self.repo)

    @property
    def work_root(self) -> Path:
        return self.repo / ".pn" / self.relative_lab

    @property
    def solution_build(self) -> Path:
        return self.work_root / "solution"

    @property
    def baseline_build(self) -> Path:
        return self.work_root / "baseline"

    @property
    def baseline_source(self) -> Path:
        return self.work_root / "baseline-src"

    @property
    def results(self) -> Path:
        return self.work_root / "results"

    @property
    def benchmark_build(self) -> Path:
        return self.repo / "tools" / "benchmark" / "build"


def find_lab(repo: Path, start: Path) -> Path:
    """Find the nearest lab source directory at or above *start*."""
    repo = repo.resolve()
    labs_root = repo / "labs"
    candidate = start.resolve()
    if candidate.is_file():
        candidate = candidate.parent

    for directory in (candidate, *candidate.parents):
        if directory == repo.parent:
            break
        if (
            directory.is_relative_to(labs_root)
            and (directory / "CMakeLists.txt").is_file()
        ):
            return directory

    raise PnError(
        f"{start} is not inside the repository's labs directory. "
        "Run pn from a lab (or its build directory), or pass the lab path."
    )


def validate_min_time(value: str) -> str:
    if not MIN_TIME_PATTERN.fullmatch(value):
        raise PnError(
            f"Invalid benchmark duration {value!r}; use seconds such as 1s or 0.25s."
        )
    return value


@lru_cache(maxsize=1)
def cmake_major_version() -> int:
    completed = subprocess.run(
        ["cmake", "--version"], text=True, capture_output=True, check=True
    )
    match = re.search(r"cmake version (\d+)", completed.stdout)
    if not match:
        raise PnError("Cannot determine the installed CMake version.")
    return int(match.group(1))


def configure_command(
    *,
    source: Path,
    build: Path,
    benchmark_build: Path,
    min_time: str,
) -> list[str]:
    command = [
        "cmake",
        "-S",
        str(source),
        "-B",
        str(build),
        "-G",
        "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_MAKE_PROGRAM={shutil.which('ninja') or 'ninja'}",
        f"-DCMAKE_C_COMPILER={shutil.which('clang-17') or 'clang-17'}",
        f"-DCMAKE_CXX_COMPILER={shutil.which('clang++-17') or 'clang++-17'}",
        f"-Dbenchmark_DIR={benchmark_build}",
        f"-DBENCHMARK_MIN_TIME={validate_min_time(min_time)}",
    ]
    if cmake_major_version() >= 4:
        command.append("-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
    return command


def benchmark_version(repo: Path) -> str | None:
    benchmark = repo / "tools" / "benchmark"
    if not benchmark.is_dir():
        return None
    completed = subprocess.run(
        ["git", "-C", str(benchmark), "describe", "--tags", "--exact-match"],
        text=True,
        capture_output=True,
        check=False,
    )
    return completed.stdout.strip() if completed.returncode == 0 else None


def prerequisite_errors(repo: Path) -> list[str]:
    errors = []
    for command in ("cmake", "git", "ninja", "clang-17", "clang++-17"):
        if shutil.which(command) is None:
            if command == "ninja":
                errors.append("Ninja is missing; install the ninja-build package.")
            else:
                errors.append(f"Required command is missing: {command}")
    errors.extend(benchmark_checkout_errors(repo))
    return errors


def require_prerequisites(repo: Path) -> None:
    errors = prerequisite_errors(repo)
    if errors:
        raise PnError("\n".join(f"- {error}" for error in errors))


def load_benchmark_lock(repo: Path) -> dict[str, str]:
    lock_path = repo / "tools" / "google-benchmark.lock.json"
    try:
        lock = json.loads(lock_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise PnError(f"Cannot read dependency lock {lock_path}: {error}") from error
    required = {"url", "tag", "commit"}
    if not required.issubset(lock) or not all(
        isinstance(lock[key], str) for key in required
    ):
        raise PnError(f"Dependency lock {lock_path} is missing url, tag, or commit.")
    return {key: lock[key] for key in required}


def git_output(arguments: Sequence[str], *, cwd: Path) -> str:
    try:
        return subprocess.run(
            ["git", *arguments],
            cwd=cwd,
            text=True,
            capture_output=True,
            check=True,
        ).stdout.strip()
    except subprocess.CalledProcessError as error:
        details = error.stderr.strip()
        raise PnError(
            f"Git command failed: git {' '.join(arguments)}\n{details}"
        ) from error


def expected_benchmark_stamp(lock: dict[str, str]) -> dict[str, str]:
    return {
        "commit": lock["commit"],
        "compiler": shutil.which("clang++-17") or "clang++-17",
        "generator": "Ninja",
        "ninja": shutil.which("ninja") or "ninja",
    }


def benchmark_checkout_errors(repo: Path) -> list[str]:
    benchmark = repo / "tools" / "benchmark"
    build_dir = benchmark / "build"
    try:
        lock = load_benchmark_lock(repo)
    except PnError as error:
        return [str(error)]
    if not (benchmark / ".git").is_dir():
        return [f"Google Benchmark {BENCHMARK_VERSION} is missing at {benchmark}."]
    try:
        head = git_output(["rev-parse", "HEAD"], cwd=benchmark)
        dirty = git_output(["status", "--porcelain"], cwd=benchmark)
    except PnError as error:
        return [str(error)]
    if head != lock["commit"]:
        return [
            f"Google Benchmark checkout is {head}, not locked commit {lock['commit']}."
        ]
    if dirty:
        return [f"Google Benchmark checkout is dirty: {benchmark}."]
    if not (build_dir / "benchmarkConfig.cmake").is_file():
        return [f"Google Benchmark is not built at {build_dir}."]
    stamp_path = build_dir / ".pn-build.json"
    try:
        stamp = json.loads(stamp_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return [f"Google Benchmark build stamp is missing or invalid at {stamp_path}."]
    if stamp != expected_benchmark_stamp(lock):
        return ["Google Benchmark build does not match its compiler/generator lock."]
    return []


def resolve_baseline_ref(repo: Path, ref: str) -> str:
    if not ref or ref.startswith("-"):
        raise PnError(f"Invalid baseline ref {ref!r}.")
    commit = git_output(
        ["rev-parse", "--verify", "--end-of-options", f"{ref}^{{commit}}"], cwd=repo
    )
    if not re.fullmatch(r"[0-9a-fA-F]{40,64}", commit):
        raise PnError(f"Git returned an invalid commit for baseline ref {ref!r}.")
    return commit


def benchmark_configure_command(benchmark: Path, build_dir: Path) -> list[str]:
    return [
        "cmake",
        "-S",
        str(benchmark),
        "-B",
        str(build_dir),
        "-G",
        "Ninja",
        f"-DCMAKE_MAKE_PROGRAM={shutil.which('ninja') or 'ninja'}",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_CXX_COMPILER={shutil.which('clang++-17') or 'clang++-17'}",
        "-DBENCHMARK_ENABLE_TESTING=OFF",
        "-DBENCHMARK_ENABLE_GTEST_TESTS=OFF",
    ]


def run_command(
    command: Sequence[str],
    *,
    cwd: Path,
    verbose: bool = False,
    quiet: bool = False,
) -> subprocess.CompletedProcess[str]:
    if verbose:
        print("+", " ".join(command))
    try:
        return subprocess.run(
            list(command),
            cwd=cwd,
            text=True,
            capture_output=quiet,
            check=True,
        )
    except subprocess.CalledProcessError as error:
        details = "\n".join(
            part.strip() for part in (error.stdout, error.stderr) if part
        )
        suffix = f"\n{details}" if details else ""
        raise PnError(f"Command failed: {' '.join(command)}{suffix}") from error


def extract_baseline(repo: Path, ref: str, destination: Path) -> None:
    """Extract a complete committed snapshot without touching the working tree."""
    try:
        archive = subprocess.run(
            [
                "git",
                "-C",
                str(repo),
                "archive",
                "--format=tar",
                ref,
            ],
            check=True,
            capture_output=True,
        ).stdout
    except subprocess.CalledProcessError as error:
        message = error.stderr.decode(errors="replace").strip()
        raise PnError(f"Cannot read baseline {ref!r}: {message}") from error

    shutil.rmtree(destination, ignore_errors=True)
    destination.mkdir(parents=True)
    with tarfile.open(fileobj=io.BytesIO(archive), mode="r:") as tar:
        tar.extractall(destination, filter="data")


def configure(
    layout: LabLayout,
    *,
    source: Path,
    build: Path,
    min_time: str,
    verbose: bool,
) -> None:
    build.mkdir(parents=True, exist_ok=True)
    run_command(
        configure_command(
            source=source,
            build=build,
            benchmark_build=layout.benchmark_build,
            min_time=min_time,
        ),
        cwd=layout.repo,
        verbose=verbose,
    )


def build(build_dir: Path, *, verbose: bool) -> None:
    run_command(
        [
            "cmake",
            "--build",
            str(build_dir),
            "--config",
            "Release",
            "--parallel",
            str(os.cpu_count() or 1),
        ],
        cwd=build_dir,
        verbose=verbose,
    )


def run_target(
    build_dir: Path, target: str, *, verbose: bool, quiet: bool = False
) -> None:
    run_command(
        ["cmake", "--build", str(build_dir), "--config", "Release", "--target", target],
        cwd=build_dir,
        verbose=verbose,
        quiet=quiet,
    )


def load_measurements(path: Path) -> dict[str, Measurement]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise PnError(f"Cannot read benchmark result {path}: {error}") from error

    measurements: dict[str, Measurement] = {}
    for row in payload.get("benchmarks", []):
        if row.get("run_type", "iteration") != "iteration" or "aggregate_name" in row:
            continue
        if row.get("error_occurred") or row.get("skipped"):
            message = row.get("error_message", "benchmark was skipped or failed")
            raise PnError(f"Benchmark {row.get('name', '<unknown>')} failed: {message}")
        unit = row.get("time_unit")
        if unit not in TIME_SCALES:
            raise PnError(f"Unsupported benchmark time unit {unit!r} in {path}.")
        scale = TIME_SCALES[unit]
        name = row.get("run_name", row["name"])
        measurements[name] = Measurement(
            real_seconds=float(row["real_time"]) * scale,
            cpu_seconds=float(row["cpu_time"]) * scale,
        )
    if not measurements:
        raise PnError(f"No benchmark measurements found in {path}.")
    return measurements


def relative_noise(values: Sequence[float]) -> float:
    if len(values) < 2:
        return 0.0
    mean = statistics.fmean(values)
    return statistics.stdev(values) / mean * 100.0 if mean else 0.0


def summarize(
    baseline: dict[str, list[Measurement]],
    solution: dict[str, list[Measurement]],
) -> list[BenchmarkSummary]:
    if set(baseline) != set(solution):
        raise PnError("Baseline and solution produced different benchmark names.")

    summaries = []
    for name in sorted(baseline):
        baseline_times = [measurement.real_seconds for measurement in baseline[name]]
        solution_times = [measurement.real_seconds for measurement in solution[name]]
        baseline_median = statistics.median(baseline_times)
        solution_median = statistics.median(solution_times)
        speedup = baseline_median / solution_median
        summaries.append(
            BenchmarkSummary(
                name=name,
                baseline_median=baseline_median,
                solution_median=solution_median,
                speedup=speedup,
                improvement_percent=(baseline_median - solution_median)
                / baseline_median
                * 100.0,
                baseline_noise_percent=relative_noise(baseline_times),
                solution_noise_percent=relative_noise(solution_times),
            )
        )
    return summaries


def human_time(seconds: float) -> str:
    for unit, scale in (("ns", 1e-9), ("us", 1e-6), ("ms", 1e-3)):
        value = seconds / scale
        if value < 1000:
            return f"{value:.3g} {unit}"
    return f"{seconds:.3g} s"


def print_summary(summaries: Iterable[BenchmarkSummary]) -> None:
    print()
    print(
        f"{'Benchmark':<32} {'Baseline':>12} {'Solution':>12} "
        f"{'Change':>10} {'Speedup':>9} {'Noise B/S':>14}"
    )
    print("-" * 94)
    for summary in summaries:
        print(
            f"{summary.name:<32} "
            f"{human_time(summary.baseline_median):>12} "
            f"{human_time(summary.solution_median):>12} "
            f"{summary.improvement_percent:>+9.2f}% "
            f"{summary.speedup:>8.2f}x "
            f"{summary.baseline_noise_percent:>5.1f}%/"
            f"{summary.solution_noise_percent:.1f}%"
        )


def warn_about_governor() -> None:
    governors = {
        path.read_text(encoding="utf-8").strip()
        for path in Path("/sys/devices/system/cpu").glob(
            "cpu*/cpufreq/scaling_governor"
        )
        if path.is_file()
    }
    if governors and governors != {"performance"}:
        print(
            f"Warning: CPU governor is {', '.join(sorted(governors))}; "
            "performance measurements may be noisy.",
            file=sys.stderr,
        )


def prepare_solution(layout: LabLayout, min_time: str, verbose: bool) -> None:
    configure(
        layout,
        source=layout.lab,
        build=layout.solution_build,
        min_time=min_time,
        verbose=verbose,
    )
    build(layout.solution_build, verbose=verbose)


def command_build(layout: LabLayout, args: argparse.Namespace) -> None:
    prepare_solution(layout, args.min_time, args.verbose)
    print(f"Built {layout.relative_lab} with Ninja in {layout.solution_build}")


def command_validate(layout: LabLayout, args: argparse.Namespace) -> None:
    prepare_solution(layout, args.min_time, args.verbose)
    run_target(layout.solution_build, "validateLab", verbose=args.verbose)


def command_bench(layout: LabLayout, args: argparse.Namespace) -> None:
    warn_about_governor()
    prepare_solution(layout, args.min_time, args.verbose)
    run_target(layout.solution_build, "benchmarkLab", verbose=args.verbose)
    print(f"JSON result: {layout.solution_build / 'result.json'}")


def append_results(
    destination: dict[str, list[Measurement]], result_path: Path
) -> None:
    for name, measurement in load_measurements(result_path).items():
        destination[name].append(measurement)


def command_compare(layout: LabLayout, args: argparse.Namespace) -> None:
    warn_about_governor()
    baseline_commit = resolve_baseline_ref(layout.repo, args.baseline)
    print(f"Baseline: {args.baseline} ({baseline_commit[:12]})")
    extract_baseline(layout.repo, baseline_commit, layout.baseline_source)
    baseline_lab = layout.baseline_source / layout.relative_lab
    try:
        prepare_solution(layout, args.min_time, args.verbose)
        shutil.rmtree(layout.baseline_build, ignore_errors=True)
        configure(
            layout,
            source=baseline_lab,
            build=layout.baseline_build,
            min_time=args.min_time,
            verbose=args.verbose,
        )
        build(layout.baseline_build, verbose=args.verbose)

        print("Validating solution and baseline...")
        run_target(layout.solution_build, "validateLab", verbose=args.verbose)
        run_target(layout.baseline_build, "validateLab", verbose=args.verbose)

        shutil.rmtree(layout.results, ignore_errors=True)
        layout.results.mkdir(parents=True)
        measurements: dict[str, dict[str, list[Measurement]]] = {
            "baseline": defaultdict(list),
            "solution": defaultdict(list),
        }
        builds = {
            "baseline": layout.baseline_build,
            "solution": layout.solution_build,
        }

        for index in range(args.runs):
            order = (
                ("baseline", "solution") if index % 2 == 0 else ("solution", "baseline")
            )
            for variant in order:
                print(f"Run {index + 1}/{args.runs}: {variant}")
                build_dir = builds[variant]
                run_target(
                    build_dir,
                    "benchmarkLab",
                    verbose=args.verbose,
                    quiet=not args.verbose,
                )
                result = build_dir / "result.json"
                saved_result = layout.results / f"{variant}-{index + 1}.json"
                if not result.is_file():
                    raise PnError(f"Benchmark did not create {result}.")
                shutil.copy2(result, saved_result)
                append_results(measurements[variant], saved_result)

        print_summary(summarize(measurements["baseline"], measurements["solution"]))
        print(f"Raw results: {layout.results}")
    finally:
        shutil.rmtree(layout.baseline_source, ignore_errors=True)


def bootstrap_dependency(repo: Path, args: argparse.Namespace) -> None:
    missing = [
        command
        for command in ("cmake", "git", "ninja", "clang-17", "clang++-17")
        if shutil.which(command) is None
    ]
    if missing:
        raise PnError(f"Cannot bootstrap; missing commands: {', '.join(missing)}")

    lock = load_benchmark_lock(repo)
    benchmark = repo / "tools" / "benchmark"
    if benchmark.exists():
        if not (benchmark / ".git").is_dir():
            raise PnError(f"Refusing to replace non-Git path {benchmark}.")
        head = git_output(["rev-parse", "HEAD"], cwd=benchmark)
        dirty = git_output(["status", "--porcelain"], cwd=benchmark)
        if dirty:
            raise PnError(f"Refusing to modify dirty dependency checkout {benchmark}.")
        if head != lock["commit"]:
            raise PnError(
                f"Dependency checkout is {head}, expected {lock['commit']}; "
                "remove it manually before bootstrapping."
            )
    else:
        temporary = benchmark.with_name(f".benchmark-{os.getpid()}.tmp")
        shutil.rmtree(temporary, ignore_errors=True)
        try:
            run_command(
                [
                    "git",
                    "clone",
                    "--branch",
                    lock["tag"],
                    "--depth",
                    "1",
                    lock["url"],
                    str(temporary),
                ],
                cwd=repo,
                verbose=args.verbose,
            )
            head = git_output(["rev-parse", "HEAD"], cwd=temporary)
            if head != lock["commit"]:
                raise PnError(
                    f"Downloaded dependency is {head}, expected locked commit {lock['commit']}."
                )
            temporary.rename(benchmark)
        finally:
            shutil.rmtree(temporary, ignore_errors=True)

    build_dir = benchmark / "build"
    shutil.rmtree(build_dir, ignore_errors=True)
    run_command(
        benchmark_configure_command(benchmark, build_dir),
        cwd=repo,
        verbose=args.verbose,
    )
    build(build_dir, verbose=args.verbose)
    (build_dir / ".pn-build.json").write_text(
        json.dumps(expected_benchmark_stamp(lock), indent=2) + "\n", encoding="utf-8"
    )
    print(f"Google Benchmark {lock['tag']} is ready at {benchmark}")


def command_bootstrap(repo: Path, args: argparse.Namespace) -> None:
    lock_path = repo / ".pn" / "bootstrap.lock"
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("w", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file, fcntl.LOCK_EX)
        try:
            bootstrap_dependency(repo, args)
        finally:
            fcntl.flock(lock_file, fcntl.LOCK_UN)


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="pn", description="Build, validate, and compare Performance Ninja labs."
    )
    parser.add_argument(
        "--verbose", action="store_true", help="show every command and benchmark output"
    )
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser(
        "bootstrap", help="build the pinned repo-local Google Benchmark dependency"
    )

    def add_common(command: str, help_text: str) -> argparse.ArgumentParser:
        subparser = subparsers.add_parser(command, help=help_text)
        subparser.add_argument(
            "lab", nargs="?", type=Path, help="lab path; defaults to the current lab"
        )
        subparser.add_argument(
            "--min-time",
            default="1s",
            type=validate_min_time,
            help="minimum benchmark time",
        )
        return subparser

    add_common("build", "build the solution with Clang and Ninja")
    add_common("validate", "build and validate the solution")
    add_common("bench", "run the solution benchmark")
    compare = add_common(
        "compare", "compare the working solution against a committed baseline"
    )
    compare.add_argument(
        "--runs", type=int, default=5, help="number of alternating runs"
    )
    compare.add_argument(
        "--baseline", default="main", help="Git ref used as the baseline"
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = make_parser()
    args = parser.parse_args(argv)
    try:
        if args.command == "bootstrap":
            command_bootstrap(REPO_ROOT, args)
            return 0
        if getattr(args, "runs", 1) < 1:
            raise PnError("--runs must be at least 1.")
        require_prerequisites(REPO_ROOT)
        start = (Path.cwd() / args.lab).resolve() if args.lab else Path.cwd()
        layout = LabLayout(REPO_ROOT, find_lab(REPO_ROOT, start))
        commands = {
            "build": command_build,
            "validate": command_validate,
            "bench": command_bench,
            "compare": command_compare,
        }
        commands[args.command](layout, args)
        return 0
    except PnError as error:
        print(f"pn: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
