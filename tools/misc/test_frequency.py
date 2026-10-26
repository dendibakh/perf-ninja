"""Check frequency constraints and crash recovery without touching real sysfs."""

from contextlib import redirect_stderr
import io
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import stable_run
from test_stable_run import make_topology


class FrequencyTests(unittest.TestCase):
  def setUp(self):
    temporary = tempfile.TemporaryDirectory(prefix="stable-run-frequency-")
    self.addCleanup(temporary.cleanup)
    self.root = Path(temporary.name)
    self.cpu = self.root / "cpu"
    for index in range(2):
      policy = self.cpu / f"cpufreq/policy{index}"
      policy.mkdir(parents=True)
      for name, value in {
          "related_cpus": f"{index} {index + 4}",
          "cpuinfo_min_freq": "400000", "cpuinfo_max_freq": "4700000",
          "base_frequency": "1200000", "scaling_min_freq": "400000",
          "scaling_max_freq": "4700000", "scaling_governor": "powersave",
          "scaling_available_governors": "performance powersave",
      }.items():
        (policy / name).write_text(value + "\n")
    self.policy = self.cpu / "cpufreq/policy0"
    self.turbo = self.cpu / "intel_pstate/no_turbo"
    self.turbo.parent.mkdir()
    self.turbo.write_text("0\n")
    self.lock = self.root / "settings.lock"

  def bounds(self):
    return tuple(int((self.policy / name).read_text())
                 for name in ("scaling_min_freq", "scaling_max_freq"))

  def kernel_write(self, path, value):
    # Model the kernel's min <= max constraint and no_turbo clamping.
    if path.name == "scaling_min_freq":
      self.assertLessEqual(int(value), int((path.parent / "scaling_max_freq").read_text()))
    elif path.name == "scaling_max_freq":
      self.assertGreaterEqual(int(value), int((path.parent / "scaling_min_freq").read_text()))
    if path.name in ("scaling_min_freq", "scaling_max_freq") and self.turbo.read_text().strip() == "1":
      self.assertLessEqual(int(value), 1200000)
    path.write_text(value + "\n")
    if path == self.turbo and value == "1":
      for name in ("scaling_min_freq", "scaling_max_freq"):
        target = self.policy / name
        target.write_text(str(min(int(target.read_text()), 1200000)) + "\n")

  def test_shared_policy_selected_once_other_policy_untouched(self):
    settings = stable_run.CpuSettings(self.cpu)
    changes = settings.plan("keep", "keep", {0, 4}, frequency_khz=3500000)
    self.assertEqual(len(changes), 2)
    with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", self.kernel_write):
      settings.apply(changes)
      self.assertEqual(self.bounds(), (3500000, 3500000))
      self.assertEqual((self.cpu / "cpufreq/policy1/scaling_max_freq").read_text(), "4700000\n")
      self.assertEqual(settings.restore(), [])
    self.assertEqual(self.bounds(), (400000, 4700000))

  def test_ranges_above_below_and_across_target_restore(self):
    for minimum, maximum in ((400000, 1200000), (4000000, 4700000),
                             (400000, 4700000), (3500000, 3500000)):
      with self.subTest(original=(minimum, maximum)):
        (self.policy / "scaling_min_freq").write_text(str(minimum))
        (self.policy / "scaling_max_freq").write_text(str(maximum))
        settings = stable_run.CpuSettings(self.cpu)
        with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", self.kernel_write):
          settings.apply(settings.plan("keep", "on", {0}, frequency_khz=3500000))
          self.assertEqual(self.bounds(), (3500000, 3500000))
          self.assertEqual(settings.restore(), [])
        self.assertEqual(self.bounds(), (minimum, maximum))

  def test_all_scope_sets_every_policy_without_selected_cpus(self):
    settings = stable_run.CpuSettings(self.cpu)
    changes = settings.plan("keep", "on", frequency_khz=3500000, frequency_scope="all")
    with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", self.kernel_write):
      settings.apply(changes)
      for policy in self.cpu.glob("cpufreq/policy*"):
        for name in ("scaling_min_freq", "scaling_max_freq"):
          self.assertEqual((policy / name).read_text().strip(), "3500000")
      self.assertEqual(settings.restore(), [])
    for policy in self.cpu.glob("cpufreq/policy*"):
      self.assertEqual((policy / "scaling_min_freq").read_text().strip(), "400000")
      self.assertEqual((policy / "scaling_max_freq").read_text().strip(), "4700000")

  def test_all_scope_restores_offlined_sibling_policy_after_hotplug_reset(self):
    make_topology(self.cpu)
    sibling = self.cpu / "cpufreq/policy4"
    sibling.mkdir()
    for source in self.policy.iterdir():
      (sibling / source.name).write_text(source.read_text())
    (sibling / "related_cpus").write_text("4")
    (sibling / "scaling_min_freq").write_text("800000")
    (sibling / "scaling_max_freq").write_text("4200000")
    online = self.cpu / "cpu4/online"

    def hotplug_write(path, value):
      if path.parent == sibling:
        self.assertEqual(online.read_text().strip(), "1", "policy restored before its CPU was online")
      self.kernel_write(path, value)
      if path == online and value == "1":
        (sibling / "scaling_min_freq").write_text("400000")
        (sibling / "scaling_max_freq").write_text("4700000")

    settings = stable_run.CpuSettings(self.cpu)
    changes = settings.plan("keep", "on", {0}, "off", 3500000, "all")
    with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", hotplug_write):
      settings.apply(changes)
      self.assertEqual(online.read_text().strip(), "0")
      self.assertEqual(settings.restore(), [])
    self.assertEqual((sibling / "scaling_min_freq").read_text().strip(), "800000")
    self.assertEqual((sibling / "scaling_max_freq").read_text().strip(), "4200000")

  def test_all_scope_journal_recovers_after_partial_second_policy_failure(self):
    settings = stable_run.CpuSettings(self.cpu)
    changes = settings.plan("keep", "on", {0}, frequency_khz=3500000, frequency_scope="all")
    journal = stable_run.StateJournal(self.lock.with_suffix(".json"))
    journal.save(stable_run.environment_snapshot(changes, None, self.cpu))

    def fail_second_policy(path, value):
      self.kernel_write(path, value)
      if path.parent.name == "policy1":
        raise OSError("second-policy setup interrupted")

    with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", fail_second_policy):
      with self.assertRaises(OSError):
        settings.apply(changes)
    with redirect_stderr(io.StringIO()):
      with mock.patch.object(stable_run.CpuSettings, "write", side_effect=self.kernel_write):
        self.assertEqual(stable_run.restore_saved_state(self.cpu, self.lock), 0)
    for policy in self.cpu.glob("cpufreq/policy*"):
      self.assertEqual((policy / "scaling_min_freq").read_text().strip(), "400000")
      self.assertEqual((policy / "scaling_max_freq").read_text().strip(), "4700000")
    self.assertFalse(journal.exists())

  def test_all_scope_validates_every_policy_and_requires_a_frequency(self):
    settings = stable_run.CpuSettings(self.cpu)
    with self.assertRaises(ValueError):
      settings.plan("keep", "on", frequency_scope="all")
    (self.cpu / "cpufreq/policy1/cpuinfo_max_freq").write_text("3200000")
    with self.assertRaisesRegex(RuntimeError, "policy1"):
      settings.plan("performance", "on", {0}, frequency_khz=3500000, frequency_scope="all")
    self.assertEqual(self.bounds(), (400000, 4700000))
    self.assertEqual((self.policy / "scaling_governor").read_text().strip(), "powersave")

  def test_restore_turbo_before_bounds_when_toggle_clamps_them(self):
    for original_turbo, target_turbo, original_max, frequency in (
        ("0", "off", 4700000, 1000000), ("1", "on", 1200000, 3500000)):
      with self.subTest(turbo=target_turbo):
        self.turbo.write_text(original_turbo)
        (self.policy / "scaling_max_freq").write_text(str(original_max))
        settings = stable_run.CpuSettings(self.cpu)
        with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", self.kernel_write):
          settings.apply(settings.plan("keep", target_turbo, {0}, frequency_khz=frequency))
          self.assertEqual(self.bounds(), (frequency, frequency))
          self.assertEqual(settings.restore(), [])
        self.assertEqual(self.bounds(), (400000, original_max))
        self.assertEqual(self.turbo.read_text().strip(), original_turbo)

  def test_invalid_and_unsupported_frequencies_fail_without_changes(self):
    settings = stable_run.CpuSettings(self.cpu)
    for cpus, turbo, frequency in ((set(), "on", 3500000), ({0}, "on", 0),
                                  ({0}, "on", 4800000), ({0}, "off", 3500000),
                                  ({2}, "on", 3500000)):
      with self.subTest(cpus=cpus, frequency=frequency):
        with self.assertRaises((RuntimeError, ValueError)):
          settings.plan("performance", turbo, cpus, frequency_khz=frequency)
        self.assertEqual(self.bounds(), (400000, 4700000))
        self.assertEqual((self.policy / "scaling_governor").read_text().strip(), "powersave")
    (self.policy / "scaling_available_frequencies").write_text("400000 1200000 4700000")
    with self.assertRaisesRegex(RuntimeError, "unavailable"):
      settings.plan("keep", "on", {0}, frequency_khz=3500000)

  def test_driver_clamping_fails_setup_and_restores_original_range(self):
    settings = stable_run.CpuSettings(self.cpu)

    def clamp(path, value):
      self.kernel_write(path, "3400000" if value == "3500000" else value)

    with redirect_stderr(io.StringIO()), mock.patch.object(settings, "write", clamp):
      with self.assertRaisesRegex(RuntimeError, "did not accept"):
        settings.apply(settings.plan("performance", "on", {0}, frequency_khz=3500000))
      self.assertEqual(settings.restore(), [])
    self.assertEqual(self.bounds(), (400000, 4700000))
    self.assertEqual((self.policy / "scaling_governor").read_text().strip(), "powersave")

  def test_delayed_kernel_updates_are_awaited_during_setup_and_restore(self):
    settings = stable_run.CpuSettings(self.cpu)
    pending = []

    def queue_write(path, value):
      pending.append((path, value))

    def apply_queued(_delay):
      self.assertEqual(len(pending), 1, "a second bound was written before the first applied")
      self.kernel_write(*pending.pop())

    with mock.patch.object(settings, "write", queue_write):
      with mock.patch.object(stable_run.time, "sleep", apply_queued):
        with redirect_stderr(io.StringIO()):
          settings.apply(settings.plan("keep", "keep", {0}, frequency_khz=3500000))
        self.assertEqual(self.bounds(), (3500000, 3500000))
        self.assertEqual(settings.restore(), [])
    self.assertEqual(self.bounds(), (400000, 4700000))
    self.assertEqual(pending, [])

  def test_sigkill_after_either_bound_and_recovery_can_be_repeated(self):
    # Kill a real helper after each write, including recovery writes. Check the
    # saved pair recovers from any intermediate state without an invalid range.
    for original in ((400000, 1200000), (4000000, 4700000)):
      for kill_after in range(1, 5):
        with self.subTest(original=original, kill_after=kill_after):
          for name, value in zip(("scaling_min_freq", "scaling_max_freq"), original):
            (self.policy / name).write_text(str(value))
          program = f'''
import os, signal, sys
from pathlib import Path
sys.path.insert(0, {str(Path(stable_run.__file__).parent)!r})
import stable_run
count = 0
def write(path, value):
    global count
    if path.name == "scaling_min_freq":
        assert int(value) <= int((path.parent / "scaling_max_freq").read_text())
    if path.name == "scaling_max_freq":
        assert int(value) >= int((path.parent / "scaling_min_freq").read_text())
    path.write_text(value + "\\n")
    count += 1
    if count == {kill_after}:
        os.kill(os.getpid(), signal.SIGKILL)
stable_run.CpuSettings.write = staticmethod(write)
sys.exit(stable_run.serve_settings("keep", "keep", Path({str(self.cpu)!r}),
    Path({str(self.lock)!r}), cpus={{0}}, frequency_khz=3500000))
'''
          result = subprocess.run([sys.executable, "-c", program], input="", text=True,
                                  capture_output=True, timeout=5)
          self.assertEqual(result.returncode, -signal.SIGKILL, result.stderr)
          self.assertTrue(self.lock.with_suffix(".json").exists())
          with mock.patch.object(stable_run.CpuSettings, "write", side_effect=self.kernel_write):
            with redirect_stderr(io.StringIO()):
              self.assertEqual(stable_run.restore_saved_state(self.cpu, self.lock), 0)
          self.assertEqual(self.bounds(), original)
          self.assertFalse(self.lock.with_suffix(".json").exists())

  def test_snapshot_requires_valid_complete_frequency_pair(self):
    changes = stable_run.CpuSettings(self.cpu).plan("keep", "keep", {0}, frequency_khz=3500000)
    for mutations in ("missing", "negative", "reversed"):
      data = stable_run.environment_snapshot(changes, None, self.cpu)
      if mutations == "missing":
        data["cpu_settings"].pop()
      elif mutations == "negative":
        data["cpu_settings"][0]["original"] = "-1"
      else:
        data["cpu_settings"][0]["original"] = "4800000"
      with self.subTest(mutation=mutations), self.assertRaises(ValueError):
        stable_run.environment_from_snapshot(data, self.cpu)


if __name__ == "__main__":
  unittest.main()
