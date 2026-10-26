r"""Build and compare the baseline and SOLUTION variants with stable measurements.

Guard the optimized implementation with #ifdef SOLUTION and keep the baseline
under #else. Run from a separate build/results directory, as your normal user:

  python3 /path/to/perf-ninja/tools/check_speedup.py \
    -lab_path /path/to/perf-ninja/labs/misc/warmup -num_runs 3 -v

Each benchmarkLab target runs through stable_run.py with run_lab.sh's defaults
and environment overrides. Configuration, compilation and validation run normally.
"""

import argparse
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys


STABLE_RUN = Path(__file__).resolve().parent / "misc/stable_run.py"


class bcolors:
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'


def stable_run_options(environment=None):
    """Match run_lab.sh, including its empty-variable-as-default behavior."""
    env = os.environ if environment is None else environment
    defaults = {
        "CPU": "2", "GOVERNOR": "performance", "TURBO": "on",
        "PLATFORM_PROFILE": "performance", "CPU_FREQ_KHZ": "3500000",
        "CPU_FREQ_SCOPE": "all", "ISOLATE": "on", "SMT": "off", "ASLR": "off",
    }
    values = {key: env.get(key) or default for key, default in defaults.items()}
    choices = {
        "TURBO": ("off", "on", "keep"), "PLATFORM_PROFILE": ("performance", "keep"),
        "CPU_FREQ_SCOPE": ("selected", "all"), "ISOLATE": ("on", "off"),
        "SMT": ("off", "keep"), "ASLR": ("off", "keep"),
    }
    for key, allowed in choices.items():
        if values[key] not in allowed:
            raise ValueError(f"{key} must be one of: {', '.join(allowed)}")
    if not re.fullmatch(r"[0-9]+", values["CPU"]):
        raise ValueError("CPU must be a non-negative integer")
    frequency = values["CPU_FREQ_KHZ"]
    if frequency != "keep" and not re.fullmatch(r"[1-9][0-9]*", frequency):
        raise ValueError("CPU_FREQ_KHZ must be a positive integer in kHz, or keep")
    options = ["--cpu", values["CPU"], "--governor", values["GOVERNOR"],
               "--turbo", values["TURBO"], "--smt", values["SMT"],
               "--aslr", values["ASLR"], "--platform-profile", values["PLATFORM_PROFILE"]]
    if frequency != "keep":
        options += ["--frequency-khz", frequency, "--frequency-scope", values["CPU_FREQ_SCOPE"]]
    if values["ISOLATE"] == "on":
        options.append("--isolate")
    return options


def callWrapper(command, verbose=False, *, cwd=None, benchmark=False):
    if verbose:
        print(bcolors.OKCYAN + "Running: " + shlex.join(command) + bcolors.ENDC, flush=True)
    subprocess.check_call(command, cwd=cwd,
                          stdout=None if verbose else subprocess.DEVNULL,
                          # Always show helper diagnostics, including failed recovery.
                          stderr=None if verbose or benchmark else subprocess.DEVNULL)


def buildAndRunBench(iterNumber, variant, cmakeFlags, labRootPath, saveCWD,
                     stableOptions, verbose=False):
    buildPath = Path(saveCWD) / f"{variant}Build{iterNumber}"
    try:
        if buildPath.is_dir():
            shutil.rmtree(buildPath)
        buildPath.mkdir()
        callWrapper(["cmake", "-B", ".", "-DCMAKE_POLICY_VERSION_MINIMUM=3.5", "-G", "Ninja",
                     "-DCMAKE_CXX_COMPILER=clang++", "-DCMAKE_C_COMPILER=clang",
                     "-DCMAKE_BUILD_TYPE=Release", *cmakeFlags, "-S", str(labRootPath)],
                    verbose, cwd=buildPath)
        callWrapper(["cmake", "--build", ".", "--config", "Release", "--parallel", "8"],
                    verbose, cwd=buildPath)
        callWrapper(["cmake", "--build", ".", "--config", "Release", "--target", "validateLab"],
                    verbose, cwd=buildPath)
        # Preserve the lab's CMake arguments, minimum duration and result.json path.
        callWrapper([sys.executable, str(STABLE_RUN), *stableOptions, "--",
                     "cmake", "--build", ".", "--config", "Release", "--target", "benchmarkLab"],
                    verbose, cwd=buildPath, benchmark=True)
    except (OSError, subprocess.CalledProcessError) as error:
        print(bcolors.FAIL + f"{variant}: iteration {iterNumber} - Failed: {error}" + bcolors.ENDC,
              file=sys.stderr)
        return False
    print(bcolors.OKGREEN + f"{variant}: iteration {iterNumber} - Done" + bcolors.ENDC, flush=True)
    return True


def compareResults(iterNumber, saveCWD):
    from gbench import util, report

    baselineResultPath = Path(saveCWD) / f"baselineBuild{iterNumber}/result.json"
    solutionResultPath = Path(saveCWD) / f"solutionBuild{iterNumber}/result.json"
    outJsonSolution = util.load_benchmark_results(solutionResultPath)
    outJsonBaseline = util.load_benchmark_results(baselineResultPath)
    diff_report = report.get_difference_report(outJsonBaseline, outJsonSolution, True)
    for line in report.print_difference_report(diff_report, False, True, 0.05, True):
        print(line)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-num_runs", type=int, help="Number of baseline/solution build pairs", default=5)
    parser.add_argument("-lab_path", type=Path, required=True, help="Path to the root of the lab")
    parser.add_argument("-D", default="SOLUTION", help="Preprocessor macro for the solution variant")
    parser.add_argument("-v", help="Show build and benchmark output", action="store_true")
    args = parser.parse_args(argv)
    if args.num_runs <= 0:
        parser.error("-num_runs must be positive")
    if hasattr(os, "geteuid") and os.geteuid() == 0:
        parser.error("run as your normal user; stable_run.py requests sudo for settings")
    labRootPath = args.lab_path.resolve()
    if not (labRootPath / "CMakeLists.txt").is_file():
        parser.error("-lab_path must point to a lab containing CMakeLists.txt")
    if not STABLE_RUN.is_file():
        parser.error(f"stable_run.py not found: {STABLE_RUN}")
    try:
        options = stable_run_options()
    except ValueError as error:
        parser.error(str(error))
    # Check the existing comparison dependency before spending time on builds.
    from gbench import report

    saveCWD = Path.cwd()
    try:
        for i in range(args.num_runs):
            if not buildAndRunBench(i, "baseline", [], labRootPath, saveCWD, options, args.v):
                return 1
            if not buildAndRunBench(i, "solution", ["-DCMAKE_CXX_FLAGS=-D" + args.D],
                                    labRootPath, saveCWD, options, args.v):
                return 1
        for i in range(args.num_runs):
            compareResults(i, saveCWD)
    except KeyboardInterrupt:
        print("Interrupted; stable_run.py restores settings or retains its recovery snapshot.", file=sys.stderr)
        return 130
    except (OSError, ValueError) as error:
        print(f"Comparison failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
