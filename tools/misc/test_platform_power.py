"""Exercise platform/RAPL rollback and recovery without touching host settings."""
from contextlib import redirect_stderr, redirect_stdout
import copy
import errno
import io
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import platform_power
import stable_run
from test_stable_run import make_topology


class PlatformPowerTests(unittest.TestCase):
  def setUp(self):
    temporary = tempfile.TemporaryDirectory(prefix="stable-run-platform-")
    self.addCleanup(temporary.cleanup)
    self.root = Path(temporary.name)
    self.cpu = self.root / "cpu"
    self.cpu.mkdir()
    make_topology(self.cpu)
    for index,governor in enumerate(("powersave","performance")):
      policy = self.cpu / f"cpufreq/policy{index}"
      policy.mkdir(parents=True)
      for name,value in {"scaling_governor":governor,
          "scaling_available_governors":"powersave performance", "related_cpus":str(index),
          "scaling_min_freq":"400000", "scaling_max_freq":"4700000",
          "cpuinfo_min_freq":"400000", "cpuinfo_max_freq":"4700000",
          "energy_performance_preference":"balance_performance" if index==0 else "performance"}.items():
        (policy / name).write_text(value+"\n")
    self.epb = self.cpu / "cpu0/power/energy_perf_bias"
    self.epb.parent.mkdir()
    self.epb.write_text("6\n")
    turbo = self.cpu / "intel_pstate/no_turbo"
    turbo.parent.mkdir()
    turbo.write_text("0\n")
    self.platform = self.root / "platform"
    self.platform.mkdir()
    self.profile = self.platform / "platform_profile"
    self.profile.write_text("balanced\n")
    (self.platform / "platform_profile_choices").write_text("low-power balanced performance\n")
    self.power = self.root / "powercap"
    self.mmio = self.power / "intel-rapl-mmio/intel-rapl-mmio:0/constraint_0_power_limit_uw"
    for kind,limit in (("intel-rapl",64000000),("intel-rapl-mmio",10000000)):
      zone = self.power / kind / (kind+":0")
      zone.mkdir(parents=True)
      for name,value in {"enabled":"1","constraint_0_power_limit_uw":str(limit),
          "constraint_0_time_window_us":"27983872","constraint_1_power_limit_uw":"64000000",
          "constraint_1_time_window_us":"2440","energy_uj":"12345"}.items():
        (zone / name).write_text(value+"\n")
    self.lock = self.root / "settings.lock"
    self.journal = stable_run.StateJournal(self.lock.with_suffix(".json"))
    self.which = mock.patch.object(platform_power.shutil,"which",return_value=None)
    self.which.start()
    self.addCleanup(self.which.stop)
    self.original = self.values()

  def values(self):
    return {str(p.relative_to(self.root)):p.read_text().strip()
            for base in (self.cpu,self.platform,self.power) for p in base.rglob("*") if p.is_file()}

  def firmware_write(self,path,value):
    if path==self.profile:
      self.assertTrue(self.journal.exists(),"profile changed before snapshot publication")
      path.write_text(value+"\n")
      # Returning to balanced deliberately does NOT reset the MMIO limit.
      self.mmio.write_text("20000000\n")
      for policy in self.cpu.glob("cpufreq/policy*"):
        (policy / "scaling_governor").write_text("powersave\n")
        (policy / "energy_performance_preference").write_text("performance\n")
        (policy / "scaling_min_freq").write_text("800000\n")
      self.epb.write_text("0\n")
    else:
      path.write_text(value+"\n")

  def serve(self,stdin=None,stdout=None,**options):
    with mock.patch.object(platform_power.PlatformSettings,"write",side_effect=self.firmware_write):
      with mock.patch.object(sys,"stdin",stdin or io.StringIO("")):
        with redirect_stdout(stdout or io.StringIO()),redirect_stderr(io.StringIO()):
          return stable_run.serve_settings("performance","on",self.cpu,self.lock,
            platform_profile="performance",platform_root=self.platform,power_root=self.power,**options)

  def restore(self):
    with mock.patch.object(platform_power.PlatformSettings,"write",side_effect=self.firmware_write):
      with redirect_stderr(io.StringIO()):
        return stable_run.restore_saved_state(self.cpu,self.lock,
          platform_root=self.platform,power_root=self.power)

  def test_helper_restores_profile_power_and_cpu_side_effects(self):
    test = self
    class Ready(io.StringIO):
      def write(self,text):
        test.assertEqual(test.profile.read_text().strip(),"performance")
        test.assertEqual(test.mmio.read_text().strip(),"20000000")
        for policy in test.cpu.glob("cpufreq/policy*"):
          test.assertEqual((policy / "scaling_governor").read_text().strip(),"performance")
        return super().write(text)
    output = Ready()
    self.assertEqual(self.serve(stdout=output,frequency_khz=3500000,frequency_scope="all"),0)
    self.assertEqual(output.getvalue(),stable_run.READY)
    self.assertEqual(self.values(),self.original)
    self.assertFalse(self.journal.exists())

  def test_partial_profile_setup_failure_restores_and_never_announces_ready(self):
    write = self.firmware_write
    def fail(path,value):
      write(path,value)
      if path==self.profile and value=="performance":
        raise OSError("partial profile switch")
    output = io.StringIO()
    with mock.patch.object(self,"firmware_write",side_effect=fail):
      self.assertEqual(self.serve(stdout=output),1)
    self.assertEqual(output.getvalue(),"")
    self.assertEqual(self.values(),self.original)
    self.assertFalse(self.journal.exists())

  def test_failed_power_restore_retains_snapshot_and_retry_recovers(self):
    write = self.firmware_write
    def fail(path,value):
      if path==self.mmio:
        raise OSError("power limit locked")
      write(path,value)
    with mock.patch.object(self,"firmware_write",side_effect=fail):
      self.assertEqual(self.serve(),1)
    self.assertTrue(self.journal.exists())
    self.assertEqual(self.profile.read_text().strip(),"balanced")
    self.assertEqual(self.restore(),0)
    self.assertEqual(self.values(),self.original)
    self.assertFalse(self.journal.exists())

  def test_keep_never_reads_or_changes_platform_or_power(self):
    with mock.patch.object(stable_run,"PlatformSettings") as platform:
      with mock.patch.object(sys,"stdin",io.StringIO("")):
        with redirect_stdout(io.StringIO()),redirect_stderr(io.StringIO()):
          self.assertEqual(stable_run.serve_settings("keep","keep",self.cpu,self.lock),0)
      platform.assert_not_called()
    self.assertEqual(self.values(),self.original)

  def test_cpu_online_precedes_profile_restore_and_power_limits_are_last(self):
    write = self.firmware_write
    online = self.cpu / 'cpu6/online'
    def ordered(path,value):
      if path==self.profile and value=='balanced':
        self.assertEqual(online.read_text().strip(),'1')
      if path==self.mmio and value=='10000000':
        self.assertEqual(self.profile.read_text().strip(),'balanced')
        self.assertEqual(self.epb.read_text().strip(),'6')
        self.assertEqual((self.cpu/'cpufreq/policy1/scaling_governor').read_text().strip(),'performance')
      write(path,value)
    with mock.patch.object(self,'firmware_write',side_effect=ordered):
      self.assertEqual(self.serve(cpus={2},smt='off'),0)
    self.assertEqual(self.values(),self.original)

  def test_ignored_power_restore_write_is_detected_and_retained(self):
    write = self.firmware_write
    def ignored(path,value):
      if path!=self.mmio:
        write(path,value)
    with mock.patch.object(self,'firmware_write',side_effect=ignored):
      self.assertEqual(self.serve(),1)
    self.assertTrue(self.journal.exists())
    self.assertEqual(self.restore(),0)
    self.assertEqual(self.values(),self.original)

  def test_no_profile_support_or_performance_fails_without_snapshot(self):
    for choices in ("balanced low-power",""):
      with self.subTest(choices=choices):
        (self.platform / "platform_profile_choices").write_text(choices)
        self.assertEqual(self.serve(),1)
        self.assertFalse(self.journal.exists())
        self.assertEqual(self.profile.read_text().strip(),"balanced")

  def test_power_metadata_and_energy_counters_are_never_saved(self):
    readonly = self.mmio.with_name("constraint_2_power_limit_uw")
    readonly.write_text("121000000\n")
    readonly.chmod(0o444)
    p = platform_power.PlatformSettings(self.platform,self.power)
    data = p.plan()
    self.assertFalse(any(e['path'].endswith(('energy_uj',readonly.name)) for e in data['power_settings']))

  def test_missing_peak_time_window_is_skipped(self):
    path = self.mmio.with_name("constraint_2_time_window_us")
    path.write_text("unused\n")
    real = Path.read_text
    def read(p,*args,**kwargs):
      if p==path:
        raise OSError(errno.ENODATA,"no time window")
      return real(p,*args,**kwargs)
    with mock.patch.object(Path,"read_text",read):
      data = platform_power.PlatformSettings(self.platform,self.power).plan()
    self.assertNotIn(path.name,[Path(e['path']).name for e in data['power_settings']])

  def test_malformed_snapshot_is_rejected_before_any_write(self):
    p = platform_power.PlatformSettings(self.platform,self.power)
    changes = stable_run.CpuSettings(self.cpu).profile_snapshot([])
    p.plan()
    data = stable_run.environment_snapshot(changes,None,self.cpu,p)
    for key,value in (("path","../../victim"),("path","intel-rapl/intel-rapl:0/energy_uj"),
                      ("original","-1"),("original",True)):
      invalid = copy.deepcopy(data)
      invalid['platform_settings']['power_settings'][0][key] = value
      self.journal.save(invalid)
      before = self.values()
      self.assertEqual(self.restore(),1)
      self.assertTrue(self.journal.exists())
      self.assertEqual(self.values(),before)
      self.journal.clear()

  def test_daemon_profile_and_actual_platform_profile_are_restored(self):
    p = platform_power.PlatformSettings(self.platform,self.power)
    daemon = ['balanced']
    def control(command,action,*args):
      if action=='get':
        return daemon[0]
      daemon[0] = args[0]
      self.firmware_write(self.profile,'performance' if args[0]=='performance' else 'balanced')
      return ''
    with mock.patch.object(platform_power.shutil,'which',return_value='/fake/powerprofilesctl'):
      with mock.patch.object(platform_power.PlatformSettings,'control',side_effect=control):
        with redirect_stderr(io.StringIO()):
          p.plan()
          self.journal.save(stable_run.environment_snapshot([],None,self.cpu,p))
          p.apply()
          self.assertEqual(daemon[0],'performance')
          self.assertEqual(p.restore_profile(),[])
          self.assertEqual(p.restore_power(),[])
    self.assertEqual(daemon[0],'balanced')
    self.assertEqual(self.profile.read_text().strip(),'balanced')
    self.assertEqual(self.mmio.read_text().strip(),'10000000')

  def test_daemon_epp_restore_requires_powersave_after_hotplug(self):
    daemon = ['balanced']
    def control(command,action,*args):
      if action=='get':
        return daemon[0]
      if args[0]=='balanced':
        self.assertEqual((self.cpu/'cpu6/online').read_text().strip(),'1')
        for p in self.cpu.glob('cpufreq/policy*/scaling_governor'):
          if p.read_text().strip()=='performance':
            raise OSError(errno.EBUSY,'EPP is locked by the performance governor')
      daemon[0] = args[0]
      self.firmware_write(self.profile,'performance' if args[0]=='performance' else 'balanced')
      return ''
    with mock.patch.object(platform_power.shutil,'which',return_value='/fake/powerprofilesctl'):
      with mock.patch.object(platform_power.PlatformSettings,'control',side_effect=control):
        self.assertEqual(self.serve(cpus={2},smt='off'),0)
    self.assertEqual(self.values(),self.original)
    self.assertEqual(daemon[0],'balanced')

  def test_sigkill_after_profile_write_and_repeated_recovery(self):
    program = f'''
import os, signal, sys
from pathlib import Path
sys.path.insert(0,{str(Path(stable_run.__file__).parent)!r})
import stable_run, platform_power
platform_power.shutil.which = lambda name: None
profile = Path({str(self.profile)!r})
mmio = Path({str(self.mmio)!r})
def write(path,value):
    assert Path({str(self.journal.path)!r}).exists()
    path.write_text(value+'\\n')
    if path==profile:
        mmio.write_text('20000000\\n')
        os.kill(os.getpid(),signal.SIGKILL)
platform_power.PlatformSettings.write = staticmethod(write)
stable_run.serve_settings('keep','keep',Path({str(self.cpu)!r}),Path({str(self.lock)!r}),
    platform_profile='performance',platform_root=Path({str(self.platform)!r}),power_root=Path({str(self.power)!r}))
'''
    result = subprocess.run([sys.executable,'-c',program],input='',text=True,capture_output=True,timeout=5)
    self.assertEqual(result.returncode,-signal.SIGKILL,result.stderr)
    self.assertTrue(self.journal.exists())
    self.assertEqual(self.restore(),0)
    self.assertEqual(self.restore(),0)
    self.assertEqual(self.values(),self.original)

  def test_auto_recovery_precedes_new_profile_snapshot(self):
    p = platform_power.PlatformSettings(self.platform,self.power)
    p.plan()
    changes = stable_run.CpuSettings(self.cpu).profile_snapshot([])
    self.journal.save(stable_run.environment_snapshot(changes,None,self.cpu,p))
    self.firmware_write(self.profile,'performance')
    self.assertEqual(self.serve(),0)
    self.assertEqual(self.values(),self.original)
    self.assertFalse(self.journal.exists())


if __name__=='__main__':
  unittest.main()
