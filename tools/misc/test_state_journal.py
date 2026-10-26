"""Kill real helpers while using fake sysfs, then recover from their journal."""

from contextlib import redirect_stderr, redirect_stdout
import copy
import io
import json
import os
from pathlib import Path
import select
import signal
import stat
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import stable_run
from test_stable_run import make_topology


class StateJournalTests(unittest.TestCase):
  def setUp(self):
    temporary = tempfile.TemporaryDirectory(prefix="stable-run-journal-test-")
    self.addCleanup(temporary.cleanup)
    self.root = Path(temporary.name)
    self.cpu = self.root / "cpu"
    self.cpu.mkdir()
    make_topology(self.cpu)
    for index, value in enumerate(("powersave", "performance")):
      policy = self.cpu / f"cpufreq/policy{index}"
      policy.mkdir(parents=True)
      (policy / "scaling_governor").write_text(value + "\n")
      (policy / "scaling_available_governors").write_text("powersave performance\n")
    self.turbo = self.cpu / "intel_pstate/no_turbo"
    self.turbo.parent.mkdir()
    self.turbo.write_text("0\n")
    self.lock = self.root / "settings.lock"
    self.path = self.lock.with_suffix(".json")
    self.journal = stable_run.StateJournal(self.path)
    self.settings = stable_run.CpuSettings(self.cpu)
    self.changes = self.settings.plan("performance", "off", {2, 3}, "off")
    self.snapshot = stable_run.environment_snapshot(self.changes, None, self.cpu)

  def values(self):
    return [(self.cpu / path).read_text().strip() for path in (
      "cpufreq/policy0/scaling_governor", "cpufreq/policy1/scaling_governor",
      "intel_pstate/no_turbo", "cpu6/online", "cpu7/online")]

  def restore(self):
    with redirect_stderr(io.StringIO()):
      return stable_run.restore_saved_state(self.cpu, self.lock)

  def apply(self):
    with redirect_stderr(io.StringIO()):
      self.settings.apply(self.changes)

  def start_helper(self, phase="ready"):
    program = f'''
import os, signal, sys
from pathlib import Path
sys.path.insert(0, {str(Path(stable_run.__file__).parent)!r})
import stable_run
root = Path({str(self.cpu)!r})
write = stable_run.CpuSettings.write
def interrupted_write(path, value):
    state = root.parent / "settings.json"
    assert state.exists(), "state was not saved before a setting changed"
    write(path, value)
    if path == root / "cpu7/online" and (
        ({phase!r} == "setup" and value == "0") or
        ({phase!r} == "cleanup" and value == "1")):
        os.kill(os.getpid(), signal.SIGKILL)
stable_run.CpuSettings.write = staticmethod(interrupted_write)
sys.exit(stable_run.serve_settings("performance", "off", root,
  Path({str(self.lock)!r}), cpus={{2, 3}}, smt="off"))
'''
    helper = subprocess.Popen([sys.executable, "-c", program], stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)

    def cleanup():
      if helper.poll() is None:
        helper.kill()
      helper.wait(timeout=5)
      for stream in (helper.stdin, helper.stdout):
        try:
          stream.close()
        except BrokenPipeError:
          pass

    self.addCleanup(cleanup)
    self.assertTrue(select.select([helper.stdout], [], [], 5)[0], "helper did not become ready")
    line = helper.stdout.readline()
    self.assertEqual(line, "" if phase == "setup" else stable_run.READY)
    return helper

  def test_sigkill_during_setup_command_and_cleanup_can_be_recovered(self):
    for phase in ("setup", "ready", "cleanup"):
      with self.subTest(phase=phase):
        helper = self.start_helper(phase)
        if phase == "ready":
          helper.kill()
        elif phase == "cleanup":
          helper.stdin.close()
        self.assertEqual(helper.wait(timeout=5), -signal.SIGKILL)
        self.assertTrue(self.path.exists())
        self.assertEqual(stat.S_IMODE(self.path.stat().st_mode), 0o600)
        self.assertNotEqual(self.values(), ["powersave", "performance", "0", "1", "1"])
        saved = self.journal.load()
        self.assertEqual(saved["cpu_settings"], self.snapshot["cpu_settings"])
        self.assertEqual(self.restore(), 0)
        self.assertEqual(self.values(), ["powersave", "performance", "0", "1", "1"])
        self.assertFalse(self.path.exists())
        self.assertEqual(self.restore(), 0)  # Repeating recovery is harmless.

  def test_live_helper_rejects_recovery_and_normal_exit_removes_snapshot(self):
    helper = self.start_helper()
    before = self.values()
    self.assertEqual(self.restore(), 1)
    self.assertTrue(self.path.exists())
    self.assertEqual(self.values(), before)
    helper.stdin.close()
    self.assertEqual(helper.wait(timeout=5), 0)
    self.assertFalse(self.path.exists())
    self.assertEqual(self.values(), ["powersave", "performance", "0", "1", "1"])

  def test_new_session_recovers_before_planning_and_saving_fresh_state(self):
    self.journal.save(self.snapshot)
    self.apply()
    checked = []
    test = self
    plan = stable_run.CpuSettings.plan

    def check_before_plan(settings, *args):
      test.assertEqual(test.values(), ["powersave", "performance", "0", "1", "1"])
      test.assertFalse(test.path.exists())
      checked.append("plan")
      return plan(settings, *args)

    class ReadyOutput(io.StringIO):
      def write(output, text):
        test.assertEqual(text, stable_run.READY)
        fresh = test.journal.load()
        test.assertNotEqual(fresh["cpu_settings"], test.snapshot["cpu_settings"])
        test.assertEqual([entry["original"] for entry in fresh["cpu_settings"]],
                         ["powersave", "performance", "0"])
        test.assertEqual(test.values(), ["powersave", "powersave", "0", "1", "1"])
        checked.append("ready")
        return super().write(text)

    with mock.patch.object(stable_run.CpuSettings, "plan", check_before_plan):
      with mock.patch.object(sys, "stdin", io.StringIO("")):
        with redirect_stderr(io.StringIO()), redirect_stdout(ReadyOutput()) as output:
          result = stable_run.serve_settings("powersave", "on", self.cpu, self.lock)
    self.assertEqual(result, 0)
    self.assertEqual(checked, ["plan", "ready"])
    self.assertEqual(output.getvalue(), stable_run.READY)
    self.assertEqual(self.values(), ["powersave", "performance", "0", "1", "1"])
    self.assertFalse(self.path.exists())

  def test_next_helper_automatically_recovers_after_sigkill(self):
    for phase in ("setup", "ready", "cleanup"):
      with self.subTest(phase=phase):
        old = self.start_helper(phase)
        if phase == "ready":
          old.kill()
        elif phase == "cleanup":
          old.stdin.close()
        self.assertEqual(old.wait(timeout=5), -signal.SIGKILL)
        self.assertEqual(self.journal.load()["helper_pid"], old.pid)
        new = self.start_helper()
        saved = self.journal.load()
        self.assertEqual(saved["helper_pid"], new.pid)
        self.assertEqual(saved["cpu_settings"], self.snapshot["cpu_settings"])
        new.stdin.close()
        self.assertEqual(new.wait(timeout=5), 0)
        self.assertEqual(self.values(), ["powersave", "performance", "0", "1", "1"])
        self.assertFalse(self.path.exists())

  def test_failed_auto_recovery_retains_old_snapshot_and_starts_no_new_setup(self):
    self.journal.save(self.snapshot)
    self.apply()
    before = self.path.read_bytes()
    write = stable_run.CpuSettings.write

    def denied(path, value):
      if path == self.turbo:
        raise OSError("injected recovery error")
      write(path, value)

    with mock.patch.object(stable_run.CpuSettings, "write", side_effect=denied):
      with mock.patch.object(stable_run.CpuSettings, "plan") as plan:
        with redirect_stderr(io.StringIO()), redirect_stdout(io.StringIO()) as output:
          result = stable_run.serve_settings("powersave", "on", self.cpu, self.lock)
        plan.assert_not_called()
    self.assertEqual(result, 1)
    self.assertEqual(output.getvalue(), "")
    self.assertEqual(self.path.read_bytes(), before)
    self.assertEqual(self.values(), ["powersave", "performance", "1", "1", "1"])
    self.assertEqual(self.restore(), 0)

  def test_corrupt_snapshot_is_not_overwritten_by_an_automatic_recovery(self):
    self.journal.save(self.snapshot)
    self.path.write_text("{broken")
    before = self.values()
    with mock.patch.object(stable_run.CpuSettings, "plan") as plan:
      with redirect_stderr(io.StringIO()), redirect_stdout(io.StringIO()) as output:
        result = stable_run.serve_settings("performance", "off", self.cpu, self.lock)
      plan.assert_not_called()
    self.assertEqual(result, 1)
    self.assertEqual(output.getvalue(), "")
    self.assertEqual(self.path.read_text(), "{broken")
    self.assertEqual(self.values(), before)

  def test_active_session_blocks_auto_recovery(self):
    helper = self.start_helper()
    before, snapshot = self.values(), self.path.read_bytes()
    with redirect_stderr(io.StringIO()), redirect_stdout(io.StringIO()) as output:
      result = stable_run.serve_settings("powersave", "on", self.cpu, self.lock)
    self.assertEqual(result, 1)
    self.assertEqual(output.getvalue(), "")
    self.assertEqual(self.path.read_bytes(), snapshot)
    self.assertEqual(self.values(), before)
    helper.stdin.close()
    self.assertEqual(helper.wait(timeout=5), 0)

  def test_failed_restore_keeps_snapshot_for_retry_and_restores_other_values(self):
    self.journal.save(self.snapshot)
    self.apply()
    write = stable_run.CpuSettings.write

    def denied(path, value):
      if path == self.turbo:
        raise OSError("injected write error")
      write(path, value)

    with mock.patch.object(stable_run.CpuSettings, "write", side_effect=denied):
      self.assertEqual(self.restore(), 1)
    self.assertTrue(self.path.exists())
    self.assertEqual(self.values(), ["powersave", "performance", "1", "1", "1"])
    self.assertEqual(self.restore(), 0)
    self.assertFalse(self.path.exists())
    self.assertEqual(self.values(), ["powersave", "performance", "0", "1", "1"])

  def test_failed_snapshot_publication_does_not_change_settings(self):
    with mock.patch.object(stable_run.os, "link", side_effect=OSError("publication failed")):
      with mock.patch.object(stable_run.CpuSettings, "write") as write:
        with redirect_stderr(io.StringIO()), redirect_stdout(io.StringIO()):
          result = stable_run.serve_settings("performance", "off", self.cpu, self.lock)
        write.assert_not_called()
    self.assertEqual(result, 1)
    self.assertFalse(self.path.exists())
    self.assertEqual(list(self.root.glob(".settings.json.*")), [])

  def test_publication_never_replaces_an_existing_snapshot(self):
    self.journal.save(self.snapshot)
    before = self.path.read_bytes()
    with self.assertRaises(RuntimeError):
      stable_run.StateJournal(self.path).save({"different": "snapshot"})
    self.assertEqual(self.path.read_bytes(), before)

  def test_invalid_snapshot_is_retained_without_any_restore_writes(self):
    self.journal.save(self.snapshot)
    self.apply()
    before = self.values()
    invalid_states = ["{truncated"]
    for key, value in (("version", 2), ("boot_id", "another boot"), ("helper_pid", False)):
      data = copy.deepcopy(self.snapshot)
      data[key] = value
      invalid_states.append(json.dumps(data))
    data = copy.deepcopy(self.snapshot)
    del data["isolation"]
    invalid_states.append(json.dumps(data))
    for relative in ("../victim", "/etc/victim", "cpu2/topology/core_id"):
      data = copy.deepcopy(self.snapshot)
      data["cpu_settings"][-1]["path"] = relative
      invalid_states.append(json.dumps(data))
    for raw in invalid_states:
      with self.subTest(raw=raw[:80]):
        self.path.write_text(raw)
        self.assertEqual(self.restore(), 1)
        self.assertTrue(self.path.exists())
        self.assertEqual(self.values(), before)

  def test_symlink_public_file_and_wrong_owner_are_rejected(self):
    other = self.root / "other.json"
    other.write_text(json.dumps(self.snapshot))
    self.path.symlink_to(other)
    self.assertEqual(self.restore(), 1)
    self.assertTrue(self.path.is_symlink())
    self.path.unlink()
    self.journal.save(self.snapshot)
    self.path.chmod(0o644)
    self.assertEqual(self.restore(), 1)
    self.path.chmod(0o600)
    with mock.patch.object(os, "geteuid", return_value=os.geteuid() + 1):
      self.assertEqual(self.restore(), 1)
    self.assertEqual(self.restore(), 0)

  def test_cli_restore_uses_sudo_without_starting_a_benchmark(self):
    with mock.patch.object(os, "geteuid", return_value=1000):
      with mock.patch.object(stable_run.subprocess, "call", return_value=23) as call:
        self.assertEqual(stable_run.main(["--restore"]), 23)
        self.assertEqual(call.call_args.args[0], ["sudo", "--", sys.executable,
                                                str(Path(stable_run.__file__).resolve()), "--_restore"])
      with mock.patch.object(stable_run.subprocess, "call") as call:
        with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
          stable_run.main(["--restore", "--", "true"])
        call.assert_not_called()


if __name__ == "__main__":
  unittest.main()
