"""Exercise cgroup v2 setup and rollback without touching the host cgroups."""

from contextlib import redirect_stderr, redirect_stdout
import io
import os
from pathlib import Path
import shutil
import select
import signal
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import stable_run
from test_stable_run import make_topology


class FakeCpuset(stable_run.CpusetIsolation):
  """Model kernel-created files, controller toggles and PID migration."""

  def make_group(self):
    self.group.mkdir()
    for name, value in {"cpuset.cpus": "", "cpuset.mems": "",
                        "cpuset.cpus.effective": "", "cpuset.cpus.exclusive": "",
                        "cpuset.cpus.partition": "member", "cgroup.procs": ""}.items():
      (self.group / name).write_text(value + "\n")

  def remove_group(self):
    if (self.group / "cgroup.procs").read_text().strip():
      raise OSError("cgroup is populated")
    shutil.rmtree(self.group)

  def write(self, path, value):
    if path.name == "cgroup.subtree_control":
      current = set(path.read_text().split())
      if value == "+cpuset":
        current.add("cpuset")
      elif value == "-cpuset":
        current.discard("cpuset")
      else:
        raise AssertionError(value)
      super().write(path, " ".join(sorted(current)))
    elif path.name == "cgroup.procs":
      for procs in self.root.rglob("cgroup.procs"):
        pids = set(procs.read_text().split())
        pids.discard(value)
        procs.write_text("\n".join(sorted(pids)))
      super().write(path, "\n".join(sorted(set(path.read_text().split()) | {value})))
    else:
      super().write(path, value)
      if path.name == "cpuset.cpus.partition" and value == "isolated":
        online = stable_run.parse_cpu_list((self.cpu_root / "online").read_text())
        for cpu in list(online):
          state = self.cpu_root / f"cpu{cpu}/online"
          if state.exists() and state.read_text().strip() == "0":
            online.remove(cpu)
        effective = stable_run.parse_cpu_list((self.group / "cpuset.cpus").read_text()) & online
        (self.group / "cpuset.cpus.effective").write_text(stable_run.cpu_list(effective))


class IsolationTests(unittest.TestCase):
  def setUp(self):
    temporary = tempfile.TemporaryDirectory(prefix="cpuset-isolation-test-")
    self.addCleanup(temporary.cleanup)
    self.root = Path(temporary.name)
    self.cpu = self.root / "cpu"
    self.cpu.mkdir()
    make_topology(self.cpu)
    self.cg = self.root / "cgroup"
    self.cg.mkdir()
    for name, value in {"cgroup.controllers": "cpuset cpu memory",
                        "cgroup.subtree_control": "cpu memory",
                        "cpuset.cpus.effective": "0-7", "cpuset.mems.effective": "0",
                        "cpuset.cpus.isolated": ""}.items():
      (self.cg / name).write_text(value + "\n")
    self.original = self.cg / "user.slice/session.scope"
    self.original.mkdir(parents=True)
    self.pid = os.getpid()
    (self.original / "cgroup.procs").write_text(str(self.pid) + "\n")
    self.proc = self.root / "proc"
    (self.proc / str(self.pid)).mkdir(parents=True)
    (self.proc / str(self.pid) / "cgroup").write_text("0::/user.slice/session.scope\n")
    (self.proc / str(self.pid) / "stat").write_text(Path(f"/proc/{self.pid}/stat").read_text())
    self.isolation = FakeCpuset({2, 3}, self.pid, self.cpu, self.cg, self.proc)

  def prepare(self):
    self.isolation.plan()
    with redirect_stderr(io.StringIO()):
      self.isolation.apply()

  def assert_restored(self):
    self.assertFalse(self.isolation.group.exists())
    self.assertEqual(set((self.cg / "cgroup.subtree_control").read_text().split()),
                     {"cpu", "memory"})
    self.assertIn(str(self.pid), (self.original / "cgroup.procs").read_text().split())

  def test_partition_reserves_whole_cores_and_returns_the_controller(self):
    self.prepare()
    group = self.isolation.group
    self.assertEqual(stable_run.parse_cpu_list((group / "cpuset.cpus").read_text()), {2, 3, 6, 7})
    self.assertEqual((group / "cpuset.cpus.partition").read_text().strip(), "isolated")
    self.assertEqual((group / "cgroup.procs").read_text().strip(), str(self.pid))
    self.assertEqual((self.original / "cgroup.procs").read_text().strip(), "")
    self.assertEqual(self.isolation.restore(), [])
    self.assert_restored()

  def test_existing_cpuset_controller_is_left_enabled(self):
    (self.cg / "cgroup.subtree_control").write_text("cpu memory cpuset\n")
    self.prepare()
    self.assertEqual(self.isolation.restore(), [])
    self.assertIn("cpuset", (self.cg / "cgroup.subtree_control").read_text().split())

  def test_daemonized_descendants_are_returned_to_the_original_cgroup(self):
    self.prepare()
    self.isolation.write(self.isolation.group / "cgroup.procs", "99999")
    self.assertEqual(self.isolation.restore(), [])
    self.assertEqual(set((self.original / "cgroup.procs").read_text().split()),
                     {str(self.pid), "99999"})
    self.assert_restored()

  def test_invalid_partition_is_rejected_and_removed(self):
    self.isolation.plan()
    real_write = self.isolation.write

    def invalid(path, value):
      real_write(path, value)
      if path.name == "cpuset.cpus.partition" and value == "isolated":
        path.write_text("isolated invalid (test)\n")

    with mock.patch.object(self.isolation, "write", side_effect=invalid):
      with self.assertRaisesRegex(RuntimeError, "CPU isolation failed"):
        self.isolation.apply()
      self.assertEqual(self.isolation.restore(), [])
    self.assert_restored()

  def test_failed_pid_migration_rolls_back_partition_and_controller(self):
    self.isolation.plan()
    real_write = self.isolation.write

    def fail_after_move(path, value):
      real_write(path, value)
      if path == self.isolation.group / "cgroup.procs":
        raise OSError("injected migration failure")

    with mock.patch.object(self.isolation, "write", side_effect=fail_after_move):
      with self.assertRaises(OSError):
        self.isolation.apply()
      self.assertEqual(self.isolation.restore(), [])
    self.assert_restored()

  def test_no_housekeeping_core_or_overlapping_partition_is_rejected(self):
    self.isolation.cpus = {0, 1, 2, 3}
    with self.assertRaisesRegex(RuntimeError, "at least one physical core"):
      self.isolation.plan()
    self.isolation.cpus = {2, 3}
    (self.cg / "cpuset.cpus.effective").write_text("0-5\n")
    with self.assertRaisesRegex(RuntimeError, "reserved by another"):
      self.isolation.plan()

  def test_existing_cgroup_is_never_overwritten(self):
    self.isolation.group.mkdir()
    with self.assertRaisesRegex(RuntimeError, "already exists"):
      self.isolation.plan()
    self.assertEqual(self.isolation.restore(), [])
    self.assertTrue(self.isolation.group.exists())

  def test_concurrent_cpuset_changes_are_preserved(self):
    self.prepare()
    (self.cg / "user.slice/cpuset.cpus").write_text("0-1\n")
    errors = self.isolation.restore()
    self.assertEqual(len(errors), 1)
    self.assertIn("Concurrent cpuset changes", errors[0])
    self.assertEqual((self.cg / "user.slice/cpuset.cpus").read_text(), "0-1\n")
    self.assertIn("cpuset", (self.cg / "cgroup.subtree_control").read_text().split())
    self.assertFalse(self.isolation.group.exists())

  def test_controller_affinity_is_restored_only_for_the_same_process(self):
    self.prepare()
    self.assertEqual(self.isolation.restore(), [])
    with mock.patch.object(os, "sched_setaffinity") as restore:
      self.assertEqual(self.isolation.restore_affinity(), [])
      restore.assert_called_once_with(self.pid, self.isolation.original_affinity)
      restore.reset_mock()
      with mock.patch.object(self.isolation, "process_start", return_value="different"):
        self.assertEqual(self.isolation.restore_affinity(), [])
      restore.assert_not_called()

  def test_unavailable_isolation_never_changes_power_or_smt(self):
    (self.cg / "cpuset.cpus.isolated").unlink()
    policy = self.cpu / "cpufreq/policy0"
    policy.mkdir(parents=True)
    governor = policy / "scaling_governor"
    governor.write_text("powersave\n")
    (policy / "scaling_available_governors").write_text("powersave performance\n")
    turbo = self.cpu / "intel_pstate/no_turbo"
    turbo.parent.mkdir()
    turbo.write_text("0\n")
    with redirect_stdout(io.StringIO()) as output, redirect_stderr(io.StringIO()):
      result = stable_run.serve_settings("performance", "off", self.cpu,
        self.root / "lock", cpus={2, 3}, smt="off", isolate=True,
        controller_pid=self.pid, cgroup_root=self.cg, proc_root=self.proc)
    self.assertEqual(result, 1)
    self.assertEqual(output.getvalue(), "")
    self.assertEqual(governor.read_text(), "powersave\n")
    self.assertEqual(turbo.read_text(), "0\n")
    self.assertEqual((self.cpu / "cpu6/online").read_text(), "1\n")

  def test_helper_eof_restores_isolation_smt_and_affinity_in_order(self):
    order = []
    real_restore = stable_run.CpuSettings.restore
    isolation = self.isolation

    def restore_power(settings):
      self.assertFalse(isolation.group.exists())
      order.append("power")
      return real_restore(settings)

    with mock.patch.object(stable_run, "CpusetIsolation", return_value=isolation):
      with mock.patch.object(stable_run.CpuSettings, "restore", restore_power):
        with mock.patch.object(os, "sched_setaffinity", side_effect=lambda *args: order.append("affinity")):
          with mock.patch.object(stable_run.sys, "stdin", io.StringIO("")):
            with redirect_stdout(io.StringIO()) as output, redirect_stderr(io.StringIO()):
              result = stable_run.serve_settings("keep", "keep", self.cpu,
                self.root / "lock", cpus={2, 3}, smt="off", isolate=True,
                controller_pid=self.pid, cgroup_root=self.cg, proc_root=self.proc)
    self.assertEqual(result, 0)
    self.assertEqual(output.getvalue(), stable_run.READY)
    self.assertEqual(order, ["power", "affinity"])
    self.assertEqual((self.cpu / "cpu6/online").read_text(), "1\n")
    self.assertEqual((self.cpu / "cpu7/online").read_text(), "1\n")
    self.assert_restored()

  def test_isolation_setup_failure_also_restores_smt(self):
    isolation = self.isolation
    real_write = isolation.write

    def fail_after_move(path, value):
      real_write(path, value)
      if path == isolation.group / "cgroup.procs":
        raise OSError("injected migration failure")

    with mock.patch.object(stable_run, "CpusetIsolation", return_value=isolation):
      with mock.patch.object(isolation, "write", side_effect=fail_after_move):
        with redirect_stdout(io.StringIO()) as output, redirect_stderr(io.StringIO()):
          result = stable_run.serve_settings("keep", "keep", self.cpu,
            self.root / "lock", cpus={2, 3}, smt="off", isolate=True,
            controller_pid=self.pid, cgroup_root=self.cg, proc_root=self.proc)
    self.assertEqual(result, 1)
    self.assertEqual(output.getvalue(), "")
    self.assertEqual((self.cpu / "cpu6/online").read_text(), "1\n")
    self.assertEqual((self.cpu / "cpu7/online").read_text(), "1\n")
    self.assert_restored()

  def recover(self, lock):
    with mock.patch.object(stable_run, "CpusetIsolation", FakeCpuset):
      with redirect_stderr(io.StringIO()):
        return stable_run.restore_saved_state(self.cpu, lock, cgroup_root=self.cg,
                                              proc_root=self.proc)

  def test_sigkill_helper_can_recover_the_partition_smt_and_controller(self):
    lock = self.root / "settings.lock"
    program = f'''
import sys
from pathlib import Path
sys.path.insert(0, {str(Path(stable_run.__file__).parent)!r})
import stable_run
from test_cpu_isolation import FakeCpuset
stable_run.CpusetIsolation = FakeCpuset
sys.exit(stable_run.serve_settings("keep", "keep", Path({str(self.cpu)!r}),
  Path({str(lock)!r}), cpus={{2, 3}}, smt="off", isolate=True,
  controller_pid={self.pid}, cgroup_root=Path({str(self.cg)!r}),
  proc_root=Path({str(self.proc)!r})))
'''
    helper = subprocess.Popen([sys.executable, "-c", program], stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
    try:
      self.assertTrue(select.select([helper.stdout], [], [], 5)[0])
      self.assertEqual(helper.stdout.readline(), stable_run.READY)
      helper.kill()
      self.assertEqual(helper.wait(timeout=5), -signal.SIGKILL)
      self.assertEqual((self.isolation.group / "cpuset.cpus.partition").read_text().strip(), "isolated")
      self.assertEqual((self.cpu / "cpu6/online").read_text(), "0\n")
      self.assertTrue(lock.with_suffix(".json").exists())
      self.assertEqual(self.recover(lock), 0)
      self.assertFalse(lock.with_suffix(".json").exists())
      self.assertEqual((self.cpu / "cpu6/online").read_text(), "1\n")
      self.assertEqual((self.cpu / "cpu7/online").read_text(), "1\n")
      self.assert_restored()
    finally:
      if helper.poll() is None:
        helper.kill()
      helper.wait(timeout=5)
      try:
        helper.stdin.close()
      except BrokenPipeError:
        pass
      helper.stdout.close()

  def test_recovery_after_cgroup_was_removed_but_smt_was_not_restored(self):
    lock = self.root / "settings.lock"
    journal = stable_run.StateJournal(lock.with_suffix(".json"))
    settings = stable_run.CpuSettings(self.cpu)
    changes = settings.plan("keep", "keep", {2, 3}, "off")
    self.isolation.plan()
    journal.save(stable_run.environment_snapshot(changes, self.isolation, self.cpu))
    with redirect_stderr(io.StringIO()):
      settings.apply(changes)
      self.isolation.apply()
    self.assertEqual(self.isolation.restore(), [])
    self.assertFalse(self.isolation.group.exists())
    self.assertEqual((self.cpu / "cpu6/online").read_text(), "0\n")
    self.assertEqual(self.recover(lock), 0)
    self.assertEqual((self.cpu / "cpu6/online").read_text(), "1\n")
    self.assert_restored()

  def test_recovery_handles_a_dead_controller_and_disappeared_original_cgroup(self):
    lock = self.root / "settings.lock"
    journal = stable_run.StateJournal(lock.with_suffix(".json"))
    self.isolation.plan()
    journal.save(stable_run.environment_snapshot([], self.isolation, self.cpu))
    with redirect_stderr(io.StringIO()):
      self.isolation.apply()
    shutil.rmtree(self.proc / str(self.pid))
    shutil.rmtree(self.original)
    (self.cg / "cgroup.procs").write_text("")
    with mock.patch.object(os, "sched_setaffinity") as restore_affinity:
      self.assertEqual(self.recover(lock), 0)
      restore_affinity.assert_not_called()
    self.assertIn(str(self.pid), (self.cg / "cgroup.procs").read_text().split())
    self.assertFalse(self.isolation.group.exists())
    self.assertNotIn("cpuset", (self.cg / "cgroup.subtree_control").read_text().split())

  def test_next_run_recovers_old_partition_before_selecting_its_offline_siblings(self):
    lock = self.root / "settings.lock"
    journal = stable_run.StateJournal(lock.with_suffix(".json"))
    settings = stable_run.CpuSettings(self.cpu)
    changes = settings.plan("keep", "keep", {2, 3}, "off")
    self.isolation.plan()
    journal.save(stable_run.environment_snapshot(changes, self.isolation, self.cpu))
    with redirect_stderr(io.StringIO()):
      settings.apply(changes)
      self.isolation.apply()
    test = self
    checks = []

    def affinity(pid):
      test.assertEqual(pid, test.pid)
      # Simulate the kernel's availability changes from stale hotplug/cpuset.
      online = {cpu for cpu in range(8) if cpu == 0 or
                (test.cpu / f"cpu{cpu}/online").read_text().strip() == "1"}
      if test.isolation.group.exists():
        online -= {2, 3, 6, 7}
      checks.append(online)
      return online

    class ReadyOutput(io.StringIO):
      def write(output, text):
        test.assertEqual(text, stable_run.READY)
        saved = journal.load()
        test.assertEqual(saved["isolation"]["cpus"], [6, 7])
        test.assertEqual(saved["isolation"]["original_affinity"], list(range(8)))
        test.assertEqual([entry["path"] for entry in saved["cpu_settings"]],
                         ["cpu2/online", "cpu3/online"])
        for cpu in (2, 3):
          test.assertEqual((test.cpu / f"cpu{cpu}/online").read_text(), "0\n")
        for cpu in (6, 7):
          test.assertEqual((test.cpu / f"cpu{cpu}/online").read_text(), "1\n")
        return super().write(text)

    with mock.patch.object(stable_run, "CpusetIsolation", FakeCpuset):
      with mock.patch.object(os, "sched_getaffinity", side_effect=affinity):
        with mock.patch.object(os, "sched_setaffinity"):
          with mock.patch.object(sys, "stdin", io.StringIO("")):
            with redirect_stderr(io.StringIO()), redirect_stdout(ReadyOutput()) as output:
              result = stable_run.serve_settings("keep", "keep", self.cpu, lock,
                cpus={6, 7}, smt="off", isolate=True, controller_pid=self.pid,
                cgroup_root=self.cg, proc_root=self.proc)
    self.assertEqual(result, 0)
    self.assertEqual(output.getvalue(), stable_run.READY)
    self.assertTrue(checks)
    self.assertTrue(all(available == set(range(8)) for available in checks))
    self.assertFalse(journal.exists())
    for cpu in (2, 3, 6, 7):
      self.assertEqual((self.cpu / f"cpu{cpu}/online").read_text(), "1\n")
    self.assert_restored()


if __name__ == "__main__":
  unittest.main()
