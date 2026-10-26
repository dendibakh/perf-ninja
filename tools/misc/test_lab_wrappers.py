"""Check the wrapper defaults and argument forwarding with a recording helper."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


class LabWrapperTests(unittest.TestCase):
  def setUp(self):
    temporary = tempfile.TemporaryDirectory(prefix="lab-wrapper-profile-")
    self.addCleanup(temporary.cleanup)
    self.root = Path(temporary.name)
    scripts = self.root/'tools/misc'
    scripts.mkdir(parents=True)
    source = Path(__file__).parent
    for name in ('run_lab.sh','perf_lab.sh'):
      shutil.copy2(source/name,scripts/name)
    self.scripts = scripts
    self.lab = self.root/'labs/misc/warmup'
    self.build = self.lab/'build'
    self.build.mkdir(parents=True)
    for name in ('CMakeLists.txt','bench.cpp'):
      (self.lab/name).touch()
    (self.build/'lab').write_text('#!/bin/sh\nexit 0\n')
    (self.build/'lab').chmod(0o755)
    helper = scripts/'stable_run.py'
    helper.write_text(f'#!{sys.executable}\nimport json, os, sys\nfrom pathlib import Path\n'
      'Path(os.environ["WRAPPER_CAPTURE"]).write_text(json.dumps({"argv":sys.argv[1:],"cwd":os.getcwd()}))\n')
    helper.chmod(0o755)
    self.capture = self.root/'capture.json'
    bin_dir = self.root/'bin'
    bin_dir.mkdir()
    for name in ('perf','taskset','setarch'):
      p = bin_dir/name
      p.write_text('#!/bin/sh\nexit 0\n')
      p.chmod(0o755)
    self.env = {k:v for k,v in os.environ.items() if k not in
      ('CPU','PERF_CPU','GOVERNOR','PLATFORM_PROFILE','TURBO','CPU_FREQ_KHZ','CPU_FREQ_SCOPE',
       'ISOLATE','SMT','ASLR','OUTPUT','FREQ','CALL_GRAPH','EVENT')}
    self.env.update(PATH=str(bin_dir)+os.pathsep+os.environ.get('PATH',os.defpath),
                    WRAPPER_CAPTURE=str(self.capture))

  def run_wrapper(self,name,args=None,cwd=None,**env):
    self.capture.unlink(missing_ok=True)
    if args is None:
      args = ['misc/warmup','--benchmark_filter=argument with spaces']
    result = subprocess.run(['bash',str(self.scripts/name),*args],cwd=cwd or self.root,env=self.env|env,
      text=True,capture_output=True,timeout=5)
    return result,json.loads(self.capture.read_text()) if self.capture.exists() else None

  def test_default_platform_profile_and_frequency_are_forwarded(self):
    for name in ('run_lab.sh','perf_lab.sh'):
      with self.subTest(wrapper=name):
        result,captured = self.run_wrapper(name)
        self.assertEqual(result.returncode,0,result.stderr)
        args = captured['argv']
        self.assertEqual(args[args.index('--platform-profile')+1],'performance')
        self.assertEqual(args[args.index('--frequency-khz')+1],'3500000')
        self.assertEqual(args[args.index('--frequency-scope')+1],'all')
        self.assertEqual(args[-1],'--benchmark_filter=argument with spaces')
        self.assertEqual(captured['cwd'],str(self.build))

  def test_keep_profile_and_skip_frequency_are_forwarded(self):
    for name in ('run_lab.sh','perf_lab.sh'):
      with self.subTest(wrapper=name):
        result,captured = self.run_wrapper(name,PLATFORM_PROFILE='keep',CPU_FREQ_KHZ='keep')
        self.assertEqual(result.returncode,0,result.stderr)
        args = captured['argv']
        self.assertEqual(args[args.index('--platform-profile')+1],'keep')
        self.assertNotIn('--frequency-khz',args)
        self.assertNotIn('--frequency-scope',args)

  def test_invalid_profile_is_rejected_before_starting_the_helper(self):
    for name in ('run_lab.sh','perf_lab.sh'):
      with self.subTest(wrapper=name):
        result,captured = self.run_wrapper(name,PLATFORM_PROFILE='invalid')
        self.assertNotEqual(result.returncode,0)
        self.assertIn('PLATFORM_PROFILE',result.stderr)
        self.assertIsNone(captured)

  def perf_arguments(self,captured):
    args = captured['argv']
    start = args.index('record')+1
    end = args.index('--',start)
    return args[start:end]

  def test_perf_event_and_frequency_environment_remains_supported(self):
    result,captured = self.run_wrapper('perf_lab.sh',EVENT='cycles:u,cache-misses:u',FREQ='99')
    self.assertEqual(result.returncode,0,result.stderr)
    args = self.perf_arguments(captured)
    self.assertEqual(args[args.index('-e')+1],'cycles:u,cache-misses:u')
    self.assertEqual(args[args.index('-F')+1],'99')

  def test_perf_repeated_events_and_cli_frequency_override_environment(self):
    result,captured = self.run_wrapper('perf_lab.sh',args=[
      '-e','cycles:u','-F99','misc/warmup','-einstructions:u','--freq=127',
      '--benchmark_filter=argument with spaces'],EVENT='cache-misses:u',FREQ='199')
    self.assertEqual(result.returncode,0,result.stderr)
    args = self.perf_arguments(captured)
    self.assertEqual([args[i+1] for i,a in enumerate(args) if a=='-e'],['cycles:u','instructions:u'])
    self.assertEqual(args[args.index('-F')+1],'127')
    self.assertEqual(captured['argv'][-1],'--benchmark_filter=argument with spaces')

  def test_perf_long_aliases_preserve_groups_and_raw_event_expressions(self):
    expression='cpu/event=0x3c,umask=0x00/:u'
    result,captured = self.run_wrapper('perf_lab.sh',args=[
      '--events={cycles:u,instructions:u}','misc/warmup','--frequency','77',
      '--event',expression,'--freq','101'])
    self.assertEqual(result.returncode,0,result.stderr)
    args = self.perf_arguments(captured)
    self.assertEqual([args[i+1] for i,a in enumerate(args) if a=='-e'],
                     ['{cycles:u,instructions:u}',expression])
    self.assertEqual(args[args.index('-F')+1],'101')

  def test_perf_separator_preserves_benchmark_options_that_match_wrapper_options(self):
    forwarded=['--event','benchmark-event','-F','321']
    result,captured = self.run_wrapper('perf_lab.sh',args=[
      'misc/warmup','-e','instructions:u','-F','99','--',*forwarded])
    self.assertEqual(result.returncode,0,result.stderr)
    args = self.perf_arguments(captured)
    self.assertEqual(args[args.index('-e')+1],'instructions:u')
    self.assertEqual(args[args.index('-F')+1],'99')
    self.assertEqual(captured['argv'][-len(forwarded):],forwarded)

  def test_perf_without_lab_uses_current_directory_and_stops_at_benchmark_args(self):
    forwarded=['--benchmark_filter','value with spaces','-e','benchmark-event']
    result,captured = self.run_wrapper('perf_lab.sh',args=['--event=cycles:u','--frequency=99',*forwarded],cwd=self.lab)
    self.assertEqual(result.returncode,0,result.stderr)
    self.assertEqual(captured['argv'][-len(forwarded):],forwarded)
    self.assertEqual(captured['cwd'],str(self.build))

  def test_perf_invalid_or_missing_values_do_not_start_the_helper(self):
    for options in (['-e'],['--events='],['--event','--freq','99'],['-F'],
                    ['--freq=0'],['--frequency=abc'],['-F-1']):
      with self.subTest(options=options):
        result,captured = self.run_wrapper('perf_lab.sh',args=['misc/warmup',*options])
        self.assertNotEqual(result.returncode,0)
        self.assertIn('Error:',result.stderr)
        self.assertIsNone(captured)


if __name__=='__main__':
  unittest.main()
