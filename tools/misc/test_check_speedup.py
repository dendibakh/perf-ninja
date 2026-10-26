"""Check stable measurement dispatch separately from compilation and validation."""
from contextlib import redirect_stderr, redirect_stdout
import importlib.util
import io
from pathlib import Path
import subprocess
import sys
import types
import unittest
from unittest import mock

import test_lab_wrappers

spec = importlib.util.spec_from_file_location("check_speedup", Path(__file__).parent.parent / "check_speedup.py")
check_speedup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(check_speedup)


class CheckSpeedupTests(unittest.TestCase):
    def setUp(self):
        self.wrapper = test_lab_wrappers.LabWrapperTests()
        self.wrapper.setUp()
        self.addCleanup(self.wrapper.doCleanups)
        self.root = self.wrapper.root / "comparison with spaces"
        self.root.mkdir()

    def test_settings_match_run_lab_for_defaults_overrides_and_empty_values(self):
        cases = [{}, {"CPU": "3", "GOVERNOR": "powersave", "TURBO": "keep",
                      "PLATFORM_PROFILE": "keep", "CPU_FREQ_KHZ": "keep", "ISOLATE": "off",
                      "SMT": "keep", "ASLR": "keep"},
                 {"CPU_FREQ_KHZ": "3000000", "CPU_FREQ_SCOPE": "selected"},
                 {"CPU": "", "CPU_FREQ_KHZ": "", "PLATFORM_PROFILE": "", "TURBO": ""}]
        for env in cases:
            with self.subTest(environment=env):
                result, captured = self.wrapper.run_wrapper("run_lab.sh", **env)
                self.assertEqual(result.returncode, 0, result.stderr)
                args = captured["argv"]
                self.assertEqual(check_speedup.stable_run_options(env), args[:args.index("--")])

    def test_only_benchmark_target_is_stabilized_and_build_cwd_is_preserved(self):
        options = check_speedup.stable_run_options({})
        original_cwd = Path.cwd()
        with mock.patch.object(check_speedup, "callWrapper") as call:
            with redirect_stdout(io.StringIO()):
                result = check_speedup.buildAndRunBench(0, "solution", ["-DCMAKE_CXX_FLAGS=-DSOLUTION"],
                    self.wrapper.lab, self.root, options)
        self.assertTrue(result)
        self.assertEqual(Path.cwd(), original_cwd)
        commands = [entry.args[0] for entry in call.call_args_list]
        self.assertEqual(len(commands), 4)
        self.assertEqual(commands[0][-2:], ["-S", str(self.wrapper.lab)])
        self.assertIn("-DCMAKE_CXX_FLAGS=-DSOLUTION", commands[0])
        self.assertIn("-DCMAKE_CXX_COMPILER=clang++", commands[0])
        self.assertEqual(commands[1], ["cmake", "--build", ".", "--config", "Release", "--parallel", "8"])
        self.assertEqual(commands[2][-2:], ["--target", "validateLab"])
        self.assertEqual(commands[3], [sys.executable, str(check_speedup.STABLE_RUN), *options, "--",
            "cmake", "--build", ".", "--config", "Release", "--target", "benchmarkLab"])
        for entry in call.call_args_list:
            self.assertEqual(entry.kwargs["cwd"], self.root / "solutionBuild0")
        self.assertTrue(call.call_args_list[-1].kwargs["benchmark"])

    def test_validator_failure_never_starts_stable_run(self):
        def fail(command, *args, **kwargs):
            if command[-1] == "validateLab":
                raise subprocess.CalledProcessError(7, command)
        with mock.patch.object(check_speedup, "callWrapper", side_effect=fail) as call:
            with redirect_stderr(io.StringIO()):
                result = check_speedup.buildAndRunBench(0, "baseline", [], self.wrapper.lab,
                    self.root, check_speedup.stable_run_options({}))
        self.assertFalse(result)
        self.assertEqual(call.call_count, 3)

    def test_stable_run_failure_is_not_reported_as_a_successful_measurement(self):
        def fail(command, *args, **kwargs):
            if str(check_speedup.STABLE_RUN) in command:
                raise subprocess.CalledProcessError(1, command)
        with mock.patch.object(check_speedup, "callWrapper", side_effect=fail):
            with redirect_stderr(io.StringIO()):
                self.assertFalse(check_speedup.buildAndRunBench(0, "baseline", [], self.wrapper.lab,
                    self.root, check_speedup.stable_run_options({})))

    def run_main(self, builds):
        # These dispatch tests do not need SciPy or an actual comparison report.
        package = types.ModuleType("gbench")
        package.report = object()
        with mock.patch.dict(sys.modules, {"gbench": package}):
            with mock.patch.object(check_speedup.Path, "cwd", return_value=self.root):
                with mock.patch.object(check_speedup.os, "geteuid", return_value=1000):
                    with mock.patch.object(check_speedup, "buildAndRunBench", side_effect=builds) as build:
                        with mock.patch.object(check_speedup, "compareResults") as compare:
                            result = check_speedup.main(["-lab_path", str(self.wrapper.lab), "-num_runs", "2"])
        return result, build, compare

    def test_failed_variant_stops_remaining_runs_and_comparison(self):
        for builds in ([False], [True, False], [True, True, False]):
            with self.subTest(build_results=builds):
                result, build, compare = self.run_main(builds)
                self.assertEqual(result, 1)
                self.assertEqual(build.call_count, len(builds))
                compare.assert_not_called()

    def test_both_variants_use_identical_settings_and_compare_separately(self):
        result, build, compare = self.run_main([True] * 4)
        self.assertEqual(result, 0)
        self.assertEqual([c.args[1] for c in build.call_args_list], ["baseline", "solution"] * 2)
        self.assertEqual([c.args[2] for c in build.call_args_list], [[], ["-DCMAKE_CXX_FLAGS=-DSOLUTION"]] * 2)
        self.assertTrue(all(c.args[5] == build.call_args_list[0].args[5] for c in build.call_args_list))
        self.assertEqual(compare.call_args_list, [mock.call(0, self.root), mock.call(1, self.root)])

    def test_invalid_stability_settings_are_rejected(self):
        for env in ({"CPU": "-1"}, {"CPU_FREQ_KHZ": "0"}, {"CPU_FREQ_SCOPE": "invalid"},
                    {"PLATFORM_PROFILE": "balanced"}, {"ISOLATE": "invalid"}, {"ASLR": "on"}):
            with self.subTest(environment=env), self.assertRaises(ValueError):
                check_speedup.stable_run_options(env)


if __name__ == "__main__":
    unittest.main()
