import json
import subprocess
import sys
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock

import pytest


TOOLS_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS_DIR))

import pn  # noqa: E402


@pytest.fixture
def repo(tmp_path: Path) -> Path:
    (tmp_path / "tools" / "benchmark" / "build").mkdir(parents=True)
    (tmp_path / "tools" / "benchmark" / ".git").mkdir()
    (tmp_path / "tools" / "benchmark" / "build" / "benchmarkConfig.cmake").write_text(
        "# test package\n", encoding="utf-8"
    )
    (tmp_path / "labs" / "misc" / "warmup" / "build").mkdir(parents=True)
    (tmp_path / "labs" / "misc" / "warmup" / "CMakeLists.txt").write_text(
        "project(lab)\n", encoding="utf-8"
    )
    return tmp_path


def test_find_lab_from_source_or_build_directory(repo: Path) -> None:
    expected = repo / "labs" / "misc" / "warmup"

    assert pn.find_lab(repo, expected) == expected
    assert pn.find_lab(repo, expected / "build") == expected


def test_find_lab_rejects_paths_outside_labs(repo: Path) -> None:
    with pytest.raises(pn.PnError, match="inside the repository's labs directory"):
        pn.find_lab(repo, repo / "tools")


def test_solution_configure_command_uses_versioned_clang_ninja_and_solution_define(
    repo: Path,
) -> None:
    lab = repo / "labs" / "misc" / "warmup"
    command = pn.configure_command(
        source=lab,
        build=repo / ".pn" / "solution",
        benchmark_build=repo / "tools" / "benchmark" / "build",
        min_time="1s",
        solution=True,
    )

    assert command[:6] == [
        "cmake",
        "-S",
        str(lab),
        "-B",
        str(repo / ".pn" / "solution"),
        "-G",
    ]
    assert "Ninja" in command
    assert any(argument.endswith("clang-17") for argument in command)
    assert any(argument.endswith("clang++-17") for argument in command)
    assert any(
        argument.startswith("-DCMAKE_MAKE_PROGRAM=") and argument.endswith("ninja")
        for argument in command
    )
    assert "-DCMAKE_CXX_FLAGS=-DSOLUTION" in command
    assert f"-Dbenchmark_DIR={repo / 'tools' / 'benchmark' / 'build'}" in command
    assert "-DBENCHMARK_MIN_TIME=1s" in command


def test_baseline_configure_command_does_not_define_solution(repo: Path) -> None:
    command = pn.configure_command(
        source=repo / "labs" / "misc" / "warmup",
        build=repo / ".pn" / "baseline",
        benchmark_build=repo / "tools" / "benchmark" / "build",
        min_time="0.5s",
        solution=False,
    )

    assert not any("SOLUTION" in argument for argument in command)


@pytest.mark.parametrize("value", ["1s", "0.5s", ".1s"])
def test_validate_min_time_accepts_google_benchmark_durations(value: str) -> None:
    assert pn.validate_min_time(value) == value


@pytest.mark.parametrize(
    "value", ["", "1", "seconds", "-1s", "1 s", "100ms", "100us", "100ns"]
)
def test_validate_min_time_rejects_ambiguous_values(value: str) -> None:
    with pytest.raises(pn.PnError, match="duration"):
        pn.validate_min_time(value)


def test_load_measurements_normalizes_units(tmp_path: Path) -> None:
    result = tmp_path / "result.json"
    result.write_text(
        json.dumps(
            {
                "benchmarks": [
                    {
                        "name": "bench_ns",
                        "run_type": "iteration",
                        "real_time": 500,
                        "cpu_time": 480,
                        "time_unit": "ns",
                    },
                    {
                        "name": "bench_us",
                        "run_type": "iteration",
                        "real_time": 2,
                        "cpu_time": 1.5,
                        "time_unit": "us",
                    },
                    {
                        "name": "bench_us_mean",
                        "run_type": "aggregate",
                        "aggregate_name": "mean",
                        "real_time": 2,
                        "cpu_time": 1.5,
                        "time_unit": "us",
                    },
                ]
            }
        ),
        encoding="utf-8",
    )

    measurements = pn.load_measurements(result)

    assert measurements["bench_ns"].real_seconds == pytest.approx(500e-9)
    assert measurements["bench_ns"].cpu_seconds == pytest.approx(480e-9)
    assert measurements["bench_us"].real_seconds == pytest.approx(2e-6)
    assert "bench_us_mean" not in measurements


def test_summarize_reports_median_speedup_and_noise() -> None:
    baseline = {
        "bench1": [
            pn.Measurement(10e-6, 9e-6),
            pn.Measurement(11e-6, 10e-6),
            pn.Measurement(9e-6, 8e-6),
        ]
    }
    solution = {
        "bench1": [
            pn.Measurement(5e-6, 4.5e-6),
            pn.Measurement(5.5e-6, 5e-6),
            pn.Measurement(4.5e-6, 4e-6),
        ]
    }

    summary = pn.summarize(baseline, solution)[0]

    assert summary.name == "bench1"
    assert summary.baseline_median == pytest.approx(10e-6)
    assert summary.solution_median == pytest.approx(5e-6)
    assert summary.speedup == pytest.approx(2.0)
    assert summary.improvement_percent == pytest.approx(50.0)
    assert summary.baseline_noise_percent > 0
    assert summary.solution_noise_percent > 0


def test_summarize_rejects_mismatched_benchmarks() -> None:
    baseline = {"baseline_only": [pn.Measurement(1.0, 1.0)]}
    solution = {"solution_only": [pn.Measurement(1.0, 1.0)]}

    with pytest.raises(pn.PnError, match="different benchmark names"):
        pn.summarize(baseline, solution)


def test_extract_baseline_uses_committed_snapshot_only(tmp_path: Path) -> None:
    repository = tmp_path / "repo"
    lab = repository / "labs" / "misc" / "warmup"
    (repository / "tools").mkdir(parents=True)
    lab.mkdir(parents=True)
    (repository / "tools" / "labs.cmake").write_text("committed\n", encoding="utf-8")
    (lab / "CMakeLists.txt").write_text("committed\n", encoding="utf-8")
    subprocess.run(
        ["git", "init", "-b", "main", repository], check=True, capture_output=True
    )
    subprocess.run(["git", "-C", repository, "add", "."], check=True)
    subprocess.run(
        [
            "git",
            "-C",
            repository,
            "-c",
            "user.name=Test",
            "-c",
            "user.email=test@example.invalid",
            "commit",
            "-m",
            "baseline",
        ],
        check=True,
        capture_output=True,
    )
    (lab / "CMakeLists.txt").write_text("working tree\n", encoding="utf-8")
    destination = tmp_path / "snapshot"

    pn.extract_baseline(repository, Path("labs/misc/warmup"), "main", destination)

    assert (destination / "labs" / "misc" / "warmup" / "CMakeLists.txt").read_text(
        encoding="utf-8"
    ) == "committed\n"
    assert (destination / "tools" / "labs.cmake").is_file()


def test_resolve_baseline_ref_returns_commit_without_option_parsing(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    commit = "a" * 40
    git_output = Mock(return_value=commit)
    monkeypatch.setattr(pn, "git_output", git_output)

    assert pn.resolve_baseline_ref(repo, "main") == commit
    assert "--end-of-options" in git_output.call_args.args[0]


def test_resolve_baseline_ref_rejects_option_like_input(repo: Path) -> None:
    with pytest.raises(pn.PnError, match="baseline ref"):
        pn.resolve_baseline_ref(repo, "--output=/tmp/archive")


def test_prerequisite_errors_do_not_require_global_compiler_aliases(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    available = {"cmake", "git", "ninja", "clang-17", "clang++-17"}
    monkeypatch.setattr(
        pn.shutil,
        "which",
        lambda command: f"/usr/bin/{command}" if command in available else None,
    )
    monkeypatch.setattr(pn, "benchmark_version", lambda _: pn.BENCHMARK_VERSION)

    assert pn.prerequisite_errors(repo) == []


def test_prerequisite_errors_explain_missing_local_tools(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        pn.shutil,
        "which",
        lambda command: None if command == "ninja" else f"/usr/bin/{command}",
    )
    monkeypatch.setattr(pn, "benchmark_version", lambda _: "v0.0.0")

    errors = pn.prerequisite_errors(repo)

    assert any("ninja-build" in error for error in errors)
    assert any(pn.BENCHMARK_VERSION in error for error in errors)


def test_require_prerequisites_combines_actionable_errors(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(pn, "prerequisite_errors", lambda _: ["first", "second"])

    with pytest.raises(pn.PnError, match=r"- first\n- second"):
        pn.require_prerequisites(repo)


def test_benchmark_version_handles_missing_and_tagged_checkout(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        pn.subprocess,
        "run",
        lambda *args, **kwargs: SimpleNamespace(returncode=0, stdout="v1.9.5\n"),
    )
    assert pn.benchmark_version(repo) == "v1.9.5"

    monkeypatch.setattr(pn.Path, "is_dir", lambda _: False)
    assert pn.benchmark_version(repo) is None


def test_run_command_reports_captured_failure(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    failure = subprocess.CalledProcessError(
        1, ["false"], output="stdout details", stderr="stderr details"
    )
    monkeypatch.setattr(pn.subprocess, "run", Mock(side_effect=failure))

    with pytest.raises(pn.PnError, match="stdout details"):
        pn.run_command(["false"], cwd=tmp_path, quiet=True)


def test_human_time_uses_readable_units() -> None:
    assert pn.human_time(10e-9) == "10 ns"
    assert pn.human_time(10e-6) == "10 us"
    assert pn.human_time(10e-3) == "10 ms"
    assert pn.human_time(2.0) == "2 s"


def test_print_summary_contains_speedup_and_noise(
    capsys: pytest.CaptureFixture[str],
) -> None:
    pn.print_summary(
        [
            pn.BenchmarkSummary(
                name="bench1",
                baseline_median=10e-6,
                solution_median=5e-6,
                speedup=2.0,
                improvement_percent=50.0,
                baseline_noise_percent=1.0,
                solution_noise_percent=2.0,
            )
        ]
    )

    output = capsys.readouterr().out
    assert "bench1" in output
    assert "+50.00%" in output
    assert "2.00x" in output
    assert "1.0%/2.0" in output


def test_command_build_validate_and_bench_dispatch_expected_targets(
    repo: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    layout = pn.LabLayout(repo, repo / "labs" / "misc" / "warmup")
    prepared = Mock()
    targeted = Mock()
    monkeypatch.setattr(pn, "prepare_solution", prepared)
    monkeypatch.setattr(pn, "run_target", targeted)
    monkeypatch.setattr(pn, "warn_about_governor", Mock())
    args = SimpleNamespace(min_time="1s", verbose=False)

    pn.command_build(layout, args)
    pn.command_validate(layout, args)
    pn.command_bench(layout, args)

    assert prepared.call_count == 3
    assert [call.args[1] for call in targeted.call_args_list] == [
        "validateLab",
        "benchmarkLab",
    ]
    assert "JSON result" in capsys.readouterr().out


def test_command_compare_runs_alternating_pairs_and_keeps_raw_results(
    repo: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    layout = pn.LabLayout(repo, repo / "labs" / "misc" / "warmup")
    args = SimpleNamespace(min_time="10ms", verbose=False, baseline="main", runs=2)
    run_order = []

    def fake_extract(
        _repo: Path, relative_lab: Path, _ref: str, destination: Path
    ) -> None:
        (destination / relative_lab).mkdir(parents=True)

    def fake_target(build_dir: Path, target: str, **_kwargs: object) -> None:
        if target != "benchmarkLab":
            return
        variant = "baseline" if build_dir == layout.baseline_build else "solution"
        run_order.append(variant)
        build_dir.mkdir(parents=True, exist_ok=True)
        elapsed = 10 if variant == "baseline" else 5
        (build_dir / "result.json").write_text(
            json.dumps(
                {
                    "benchmarks": [
                        {
                            "name": "bench1",
                            "run_type": "iteration",
                            "real_time": elapsed,
                            "cpu_time": elapsed,
                            "time_unit": "us",
                        }
                    ]
                }
            ),
            encoding="utf-8",
        )

    monkeypatch.setattr(pn, "extract_baseline", fake_extract)
    monkeypatch.setattr(pn, "resolve_baseline_ref", lambda *_: "a" * 40)
    monkeypatch.setattr(pn, "prepare_solution", Mock())
    monkeypatch.setattr(pn, "configure", Mock())
    monkeypatch.setattr(pn, "build", Mock())
    monkeypatch.setattr(pn, "run_target", fake_target)
    monkeypatch.setattr(pn, "warn_about_governor", Mock())

    pn.command_compare(layout, args)

    assert run_order == ["baseline", "solution", "solution", "baseline"]
    assert len(list(layout.results.glob("baseline-*.json"))) == 2
    assert len(list(layout.results.glob("solution-*.json"))) == 2
    assert not layout.baseline_source.exists()
    output = capsys.readouterr().out
    assert "+50.00%" in output
    assert "2.00x" in output


def test_main_dispatches_from_an_explicit_lab(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    called = Mock()
    monkeypatch.setattr(pn, "REPO_ROOT", repo)
    monkeypatch.setattr(pn, "require_prerequisites", Mock())
    monkeypatch.setattr(pn, "command_build", called)

    assert pn.main(["build", str(repo / "labs" / "misc" / "warmup")]) == 0
    assert called.call_count == 1


def test_main_returns_actionable_error_for_invalid_runs(
    repo: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    monkeypatch.setattr(pn, "REPO_ROOT", repo)

    assert (
        pn.main(["compare", str(repo / "labs" / "misc" / "warmup"), "--runs", "0"]) == 2
    )
    assert "--runs must be at least 1" in capsys.readouterr().err


def test_main_bootstrap_does_not_require_an_existing_benchmark_checkout(
    repo: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    bootstrap = Mock()
    monkeypatch.setattr(pn, "REPO_ROOT", repo)
    monkeypatch.setattr(pn, "command_bootstrap", bootstrap)
    full_prerequisites = Mock(side_effect=AssertionError("must not run"))
    monkeypatch.setattr(pn, "require_prerequisites", full_prerequisites)

    assert pn.main(["bootstrap"]) == 0
    assert bootstrap.call_count == 1


def test_bootstrap_configure_command_disables_all_dependency_tests(
    tmp_path: Path,
) -> None:
    benchmark = tmp_path / "benchmark"
    command = pn.benchmark_configure_command(benchmark, benchmark / "build")

    assert "-G" in command and "Ninja" in command
    assert "-DBENCHMARK_ENABLE_TESTING=OFF" in command
    assert "-DBENCHMARK_ENABLE_GTEST_TESTS=OFF" in command
    assert any(argument.endswith("clang++-17") for argument in command)
