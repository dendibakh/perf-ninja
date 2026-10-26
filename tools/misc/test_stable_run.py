#!/usr/bin/env python3
"""Exercise restoration against temporary files, without changing real sysfs."""

from contextlib import redirect_stderr, redirect_stdout
import fcntl
import io
import os
from pathlib import Path
import signal
import sys
import tempfile
import unittest
from unittest import mock

import stable_run


def make_topology(root):
  (root / "online").write_text("0-7\n")
  for cpu in range(8):
    directory = root / f"cpu{cpu}"
    (directory / "topology").mkdir(parents=True)
    core = cpu % 4
    (directory / "topology/thread_siblings_list").write_text(f"{core},{core + 4}\n")
    if cpu:
      (directory / "online").write_text("1\n")


class StableRunTests(unittest.TestCase):
  def setUp(self):
    self.temporary = tempfile.TemporaryDirectory(prefix="stable-run-test-")
    self.addCleanup(self.temporary.cleanup)
    self.root = Path(self.temporary.name)
    self.cpu_root = self.root / "cpu"
    self.lock_path = self.root / "settings.lock"
    for index, governor in enumerate(("powersave", "performance")):
      policy = self.cpu_root / f"cpufreq/policy{index}"
      policy.mkdir(parents=True)
      (policy / "scaling_governor").write_text(governor + "\n")
      (policy / "scaling_available_governors").write_text("powersave performance\n")
    self.no_turbo = self.cpu_root / "intel_pstate/no_turbo"
    self.no_turbo.parent.mkdir()
    self.no_turbo.write_text("0\n")
    make_topology(self.cpu_root)

  def values(self):
    return [path.read_text().strip() for path in self.governor_paths()] + [
      self.no_turbo.read_text().strip()]

  def governor_paths(self):
    return sorted(self.cpu_root.glob("cpufreq/policy*/scaling_governor"))

  def test_mixed_governors_and_turbo_are_restored_exactly(self):
    settings = stable_run.CpuSettings(self.cpu_root)
    with redirect_stderr(io.StringIO()):
      settings.apply(settings.plan("performance", "off"))
    self.assertEqual(self.values(), ["performance", "performance", "1"])
    self.assertEqual(settings.restore(), [])
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_unavailable_governor_is_rejected_before_any_change(self):
    settings = stable_run.CpuSettings(self.cpu_root)
    with self.assertRaisesRegex(RuntimeError, "unavailable"):
      settings.plan("missing", "off")
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_partial_setup_failure_rolls_back(self):
    settings = stable_run.CpuSettings(self.cpu_root)
    real_write = settings.write

    def fail_turbo_write(path, value):
      if path == self.no_turbo and value == "1":
        # Simulate an error after the write has already changed the setting.
        real_write(path, value)
        raise OSError("injected write failure")
      real_write(path, value)

    with mock.patch.object(settings, "write", side_effect=fail_turbo_write):
      with redirect_stderr(io.StringIO()):
        with self.assertRaises(OSError):
          settings.apply(settings.plan("performance", "off"))
      self.assertEqual(settings.restore(), [])
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_restoration_attempts_other_files_after_an_error(self):
    settings = stable_run.CpuSettings(self.cpu_root)
    with redirect_stderr(io.StringIO()):
      settings.apply(settings.plan("performance", "off"))
    self.no_turbo.unlink()
    self.assertEqual(len(settings.restore()), 1)
    self.assertEqual([p.read_text().strip() for p in self.governor_paths()],
                     ["powersave", "performance"])

  def test_generic_boost_control_and_keep_modes(self):
    self.no_turbo.unlink()
    boost = self.cpu_root / "cpufreq/boost"
    boost.write_text("1\n")
    settings = stable_run.CpuSettings(self.cpu_root)
    with redirect_stderr(io.StringIO()):
      settings.apply(settings.plan("keep", "off"))
    self.assertEqual(boost.read_text().strip(), "0")
    self.assertEqual(settings.restore(), [])
    self.assertEqual(boost.read_text().strip(), "1")
    self.assertEqual(settings.plan("keep", "keep"), [])

  def test_helper_restores_on_controller_eof(self):
    class ReadyOutput(io.StringIO):
      def write(output, text):
        self.assertEqual(self.values(), ["performance", "performance", "1"])
        return super(ReadyOutput, output).write(text)

    output = ReadyOutput()
    with mock.patch.object(sys, "stdin", io.StringIO("")):
      with redirect_stdout(output), redirect_stderr(io.StringIO()):
        result = stable_run.serve_settings("performance", "off", self.cpu_root,
                                           self.lock_path)
    self.assertEqual(result, 0)
    self.assertEqual(output.getvalue(), stable_run.READY)
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_concurrent_session_cannot_change_settings(self):
    with self.lock_path.open("a") as lock:
      fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
      with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
        result = stable_run.serve_settings("performance", "off", self.cpu_root,
                                           self.lock_path)
    self.assertEqual(result, 1)
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def fake_helper_command(self, governor, turbo, **options):
    program = (
      "import sys, signal; from pathlib import Path; "
      f"sys.path.insert(0, {str(Path(stable_run.__file__).parent)!r}); "
      "import stable_run; "
      "signal.signal(signal.SIGINT, signal.SIG_IGN); "
      "signal.signal(signal.SIGTERM, signal.SIG_IGN); "
      "signal.signal(signal.SIGHUP, signal.SIG_IGN); "
      f"sys.exit(stable_run.serve_settings({governor!r}, {turbo!r}, "
      f"Path({str(self.cpu_root)!r}), Path({str(self.lock_path)!r}), **{options!r}))"
    )
    return [sys.executable, "-c", program]

  def run_command(self, command, options=()):
    with mock.patch.object(stable_run, "helper_command", self.fake_helper_command):
      with mock.patch.object(os, "geteuid", return_value=1000):
        return stable_run.main([*options, "--", *command])

  def test_command_keeps_user_environment_cwd_and_arguments(self):
    program = (
      "import os, sys; from pathlib import Path; "
      f"assert os.getuid() == {os.getuid()}; "
      f"assert os.getcwd() == {os.getcwd()!r}; "
      "assert os.environ['STABLE_RUN_TEST_VALUE'] == 'kept'; "
      "assert sys.argv[1] == 'argument with spaces'; "
      f"assert Path({str(self.no_turbo)!r}).read_text().strip() == '1'"
    )
    with mock.patch.dict(os.environ, {"STABLE_RUN_TEST_VALUE": "kept"}):
      self.assertEqual(self.run_command([sys.executable, "-c", program,
                                         "argument with spaces"]), 0)
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_command_failure_and_missing_command_restore_settings(self):
    self.assertEqual(self.run_command([sys.executable, "-c", "raise SystemExit(23)"]), 23)
    self.assertEqual(self.values(), ["powersave", "performance", "0"])
    self.assertEqual(self.run_command([str(self.root / "missing-command")]), 127)
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_preparation_failure_does_not_start_the_command(self):
    marker = self.root / "command-started"
    program = f"from pathlib import Path; Path({str(marker)!r}).touch()"
    result = self.run_command([sys.executable, "-c", program],
                              options=["--governor", "missing"])
    self.assertEqual(result, 1)
    self.assertFalse(marker.exists())
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_restoration_failure_is_returned_to_the_caller(self):
    program = f"from pathlib import Path; Path({str(self.no_turbo)!r}).unlink()"
    self.assertEqual(self.run_command([sys.executable, "-c", program]), 1)
    self.assertEqual([p.read_text().strip() for p in self.governor_paths()],
                     ["powersave", "performance"])

  def test_optional_cpu_affinity_does_not_change_the_controller(self):
    original = os.sched_getaffinity(0)
    cpu = min(original)
    program = f"import os; assert os.sched_getaffinity(0) == {{{cpu}}}"
    result = self.run_command([sys.executable, "-c", program],
                              options=["--cpu", str(cpu)])
    self.assertEqual(result, 0)
    self.assertEqual(os.sched_getaffinity(0), original)
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_controller_defers_cpu_availability_checks_to_the_settings_helper(self):
    cpu = min(os.sched_getaffinity(0))
    program = f"import os; assert os.sched_getaffinity(0) == {{{cpu}}}"
    # A stale partition may hide this CPU until the helper performs recovery.
    with mock.patch.object(os, "sched_getaffinity", return_value=set()) as affinity:
      self.assertEqual(self.run_command([sys.executable, "-c", program],
                                       options=["--cpu", str(cpu)]), 0)
      affinity.assert_not_called()

  def test_interrupt_and_termination_restore_settings(self):
    for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
      with self.subTest(signal=signum):
        program = (
          "import os, signal, time; "
          f"os.kill(os.getppid(), {int(signum)}); time.sleep(0.2)"
        )
        self.assertEqual(self.run_command([sys.executable, "-c", program]),
                         128 + signum)
        self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_smt_siblings_are_offlined_and_restored_before_governors(self):
    settings = stable_run.CpuSettings(self.cpu_root)
    changes = settings.plan("performance", "off", {2, 3}, "off")
    writes = []
    real_write = settings.write

    def record(path, value):
      writes.append((path.name, value))
      real_write(path, value)

    with mock.patch.object(settings, "write", side_effect=record), redirect_stderr(io.StringIO()):
      settings.apply(changes)
      for sibling in (6, 7):
        self.assertEqual((self.cpu_root / f"cpu{sibling}/online").read_text(), "0\n")
      self.assertEqual((self.cpu_root / "cpu2/online").read_text(), "1\n")
      writes.clear()
      self.assertEqual(settings.restore(), [])
    self.assertEqual(writes[:2], [("online", "1"), ("online", "1")])
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_offline_smt_sibling_stays_offline(self):
    path = self.cpu_root / "cpu6/online"
    path.write_text("0\n")
    settings = stable_run.CpuSettings(self.cpu_root)
    self.assertEqual(settings.plan("keep", "keep", {2}, "off"), [])
    self.assertEqual(settings.restore(), [])
    self.assertEqual(path.read_text(), "0\n")

  def test_smt_topology_errors_are_rejected_before_changes(self):
    for cpus in ({2, 6}, {4}):
      with self.subTest(cpus=cpus):
        with self.assertRaises(RuntimeError):
          stable_run.CpuSettings(self.cpu_root).plan("performance", "off", cpus, "off")
        self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_partial_smt_failure_restores_every_setting(self):
    settings = stable_run.CpuSettings(self.cpu_root)
    real_write = settings.write

    def fail_after_offlining(path, value):
      real_write(path, value)
      if path == self.cpu_root / "cpu7/online" and value == "0":
        raise OSError("injected hotplug failure")

    with mock.patch.object(settings, "write", side_effect=fail_after_offlining):
      with redirect_stderr(io.StringIO()), self.assertRaises(OSError):
        settings.apply(settings.plan("performance", "off", {2, 3}, "off"))
      self.assertEqual(settings.restore(), [])
    for sibling in (6, 7):
      self.assertEqual((self.cpu_root / f"cpu{sibling}/online").read_text(), "1\n")
    self.assertEqual(self.values(), ["powersave", "performance", "0"])

  def test_smt_is_restored_after_command_failure_and_signals(self):
    candidates = os.sched_getaffinity(0) & {0, 1, 2, 3}
    if not candidates:
      self.skipTest("needs a CPU represented in the fake topology")
    cpu = min(candidates)
    options = ["--cpu", str(cpu), "--smt", "off"]
    program = (f"from pathlib import Path; assert "
               f"Path({str(self.cpu_root / f'cpu{cpu + 4}/online')!r}).read_text() == '0\\n'; "
               "raise SystemExit(23)")
    with redirect_stderr(io.StringIO()):
      self.assertEqual(self.run_command([sys.executable, "-c", program], options), 23)
      for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        program = f"import os, time; os.kill(os.getppid(), {int(signum)}); time.sleep(0.2)"
        self.assertEqual(self.run_command([sys.executable, "-c", program], options), 128 + signum)
        self.assertEqual((self.cpu_root / f"cpu{cpu + 4}/online").read_text(), "1\n")

  def test_aslr_probe_failure_does_not_start_the_helper(self):
    denied = type("Probe", (), {"returncode": 1, "stderr": "personality denied"})()
    with mock.patch.object(stable_run.subprocess, "run", return_value=denied):
      with mock.patch.object(stable_run, "helper_command") as helper:
        with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
          stable_run.main(["--aslr", "off", "--", "true"])
        helper.assert_not_called()

  def test_aslr_wraps_the_command_without_changing_the_controller(self):
    wrapper = self.root / "setarch"
    wrapper.write_text("#!/bin/sh\n[ \"$1\" = --addr-no-randomize ] || exit 81\n"
                       "shift\nexport STABLE_RUN_ASLR_TEST=off\nexec \"$@\"\n")
    wrapper.chmod(0o755)
    original_which = stable_run.shutil.which

    def which(name):
      return str(wrapper) if name == "setarch" else original_which(name)

    program = "import os; assert os.environ['STABLE_RUN_ASLR_TEST'] == 'off'"
    with mock.patch.object(stable_run.shutil, "which", side_effect=which):
      self.assertEqual(self.run_command([sys.executable, "-c", program],
                                       ["--aslr", "off"]), 0)
    self.assertNotIn("STABLE_RUN_ASLR_TEST", os.environ)

  def test_cli_requires_cpus_for_isolation_and_checks_cpu_list(self):
    for options in (["--isolate"], ["--smt", "off"], ["--cpus", "2-1"],
                    ["--cpu", "0", "--cpus", "1", "--isolate"]):
      with self.subTest(options=options):
        with mock.patch.object(stable_run, "helper_command") as helper:
          with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            stable_run.main([*options, "--", "true"])
          helper.assert_not_called()


if __name__ == "__main__":
  unittest.main()
