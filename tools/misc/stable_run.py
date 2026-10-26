#!/usr/bin/env python3
"""Temporarily stabilize the Linux environment around a benchmark command."""

import argparse
import errno
import fcntl
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import signal
import stat
import subprocess
import sys
import tempfile
import time

from platform_power import PLATFORM_ROOT, POWER_ROOT, PlatformSettings

CPU_ROOT = Path("/sys/devices/system/cpu")
CGROUP_ROOT = Path("/sys/fs/cgroup")
LOCK_PATH = Path("/run/perf-ninja-stable-run.lock")
STATE_PATH = LOCK_PATH.with_suffix(".json")
READY = "CPU settings ready\n"


def boot_id():
  return Path("/proc/sys/kernel/random/boot_id").read_text().strip()


class StateJournal:
  """Publish a complete, private snapshot before any system changes."""

  def __init__(self, path):
    self.path = path
    self.owned = False

  def exists(self):
    return os.path.lexists(self.path)

  def sync_directory(self):
    fd = os.open(self.path.parent, os.O_RDONLY | os.O_DIRECTORY)
    try:
      os.fsync(fd)
    finally:
      os.close(fd)

  def save(self, data):
    if self.exists():
      raise RuntimeError(f"Unrestored CPU state at {self.path}; run stable_run.py --restore first")
    fd, temporary = tempfile.mkstemp(prefix=f".{self.path.name}.", dir=self.path.parent)
    try:
      with os.fdopen(fd, "w") as output:
        json.dump(data, output, indent=2)
        output.write("\n")
        output.flush()
        os.fsync(output.fileno())
      # link() publishes atomically and refuses to replace an existing file.
      os.link(temporary, self.path)
      self.owned = True
      self.sync_directory()
    finally:
      os.unlink(temporary)

  def load(self):
    fd = os.open(self.path, os.O_RDONLY | os.O_NOFOLLOW)
    with os.fdopen(fd, "r") as source:
      info = os.fstat(source.fileno())
      if not stat.S_ISREG(info.st_mode) or info.st_uid != os.geteuid() or info.st_mode & 0o077:
        raise RuntimeError("The CPU state file must be a private regular file owned by the restoring user (root)")
      raw = source.read(1024 * 1024 + 1)
      if len(raw) > 1024 * 1024:
        raise RuntimeError("The CPU state file is too large")
      data = json.loads(raw)
    if (not isinstance(data, dict) or type(data.get("version")) is not int or data["version"] != 1 or
        not {"boot_id", "helper_pid", "cpu_settings", "isolation"} <= data.keys() or
        type(data.get("helper_pid")) is not int or data["helper_pid"] <= 0):
      raise RuntimeError("Unsupported CPU state file format")
    if data.get("boot_id") != boot_id():
      raise RuntimeError("The CPU state file belongs to another boot; refusing to apply it")
    return data

  def clear(self):
    self.path.unlink()
    self.owned = False
    self.sync_directory()


def parse_cpu_list(value):
  """Parse the CPU list format used by sysfs and cpusets."""
  cpus = set()
  for item in value.strip().split(","):
    if not item:
      if not value.strip():
        return cpus
      raise ValueError(f"Invalid CPU list: {value!r}")
    bounds = item.split("-")
    if len(bounds) not in (1, 2) or not all(part.isdecimal() for part in bounds):
      raise ValueError(f"Invalid CPU list: {value!r}")
    first, last = int(bounds[0]), int(bounds[-1])
    if first > last:
      raise ValueError(f"Invalid CPU range: {item!r}")
    cpus.update(range(first, last + 1))
  return cpus


def cpu_list(cpus):
  return ",".join(str(cpu) for cpu in sorted(cpus))


def physical_cores(cpus, cpu_root=CPU_ROOT):
  cores = set()
  for cpu in cpus:
    siblings = parse_cpu_list(
      (cpu_root / f"cpu{cpu}/topology/thread_siblings_list").read_text())
    if cpu not in siblings:
      raise RuntimeError(f"Invalid SMT topology for CPU {cpu}")
    cores.update(siblings)
  return cores


class CpuSettings:
  def __init__(self, cpu_root=CPU_ROOT):
    self.cpu_root = cpu_root
    self.saved = []

  def profile_snapshot(self, changes):
    """Also preserve CPU controls that power-profiles-daemon may change."""
    snapshot = list(changes)
    seen = {path for path, _, _ in snapshot}
    paths = []
    for name in ("scaling_governor", "scaling_min_freq", "scaling_max_freq",
                 "energy_performance_preference"):
      paths.extend(self.cpu_root.glob("cpufreq/policy*/" + name))
    paths.extend(self.cpu_root.glob("cpu[0-9]*/power/energy_perf_bias"))
    paths.extend(p for p in (self.cpu_root / "intel_pstate/no_turbo",
                             self.cpu_root / "cpufreq/boost") if p.exists())
    for path in sorted(paths):
      if path not in seen:
        original = path.read_text().strip()
        snapshot.append((path, original, original))
        seen.add(path)
    return snapshot

  def remember(self, path, original):
    if not any(p == path for p, _ in self.saved):
      self.saved.append((path, original))

  def allow_profile_epp_changes(self):
    """intel_pstate rejects nonzero EPP while a policy uses performance."""
    errors = []
    for path, _ in self.saved:
      if path.name != "scaling_governor" or not (path.parent / "energy_performance_preference").exists():
        continue
      try:
        if path.read_text().strip() == "performance":
          if "powersave" not in (path.parent / "scaling_available_governors").read_text().split():
            raise RuntimeError("powersave governor is required to let the profile manager change EPP")
          self.write(path, "powersave")
      except (OSError, RuntimeError) as error:
        errors.append(f"Preparing {path} for profile switching: {error}")
    return errors

  def plan(self, governor, turbo, cpus=(), smt="keep", frequency_khz=None,
           frequency_scope="selected"):
    if frequency_scope not in ("selected", "all"):
      raise ValueError("Frequency scope must be selected or all")
    if frequency_scope == "all" and frequency_khz is None:
      raise ValueError("All-policy frequency control requires a frequency in kHz")
    changes = []
    if governor != "keep":
      paths = sorted(self.cpu_root.glob("cpufreq/policy*/scaling_governor"))
      if not paths:
        raise RuntimeError("No CPUFreq policies found; use --governor keep to skip")
      for path in paths:
        available = (path.parent / "scaling_available_governors").read_text().split()
        if governor not in available:
          raise RuntimeError(f"Governor {governor!r} is unavailable for {path.parent.name}")
        changes.append((path, path.read_text().strip(), governor))

    if turbo != "keep":
      no_turbo = self.cpu_root / "intel_pstate/no_turbo"
      boost = self.cpu_root / "cpufreq/boost"
      if no_turbo.exists():
        path, target = no_turbo, "1" if turbo == "off" else "0"
      elif boost.exists():
        path, target = boost, "0" if turbo == "off" else "1"
      else:
        raise RuntimeError("No supported Turbo/boost control found; use --turbo keep to skip")
      changes.append((path, path.read_text().strip(), target))

    if frequency_khz is not None:
      if (type(frequency_khz) is not int or frequency_khz <= 0 or
          (frequency_scope == "selected" and not cpus)):
        raise ValueError("A positive frequency in kHz and selected CPUs (or all-policy scope) are required")
      remaining = set(cpus) if frequency_scope == "selected" else set()
      policies = sorted(self.cpu_root.glob("cpufreq/policy*"))
      if not policies:
        raise RuntimeError("No CPUFreq policies found for frequency control")
      for policy in policies:
        related = parse_cpu_list(",".join((policy / "related_cpus").read_text().split()))
        if frequency_scope == "selected" and not related.intersection(cpus):
          continue
        remaining.difference_update(related)
        lower = int((policy / "cpuinfo_min_freq").read_text())
        upper = int((policy / "cpuinfo_max_freq").read_text())
        no_turbo = self.cpu_root / "intel_pstate/no_turbo"
        if no_turbo.exists() and (turbo == "off" or
            (turbo == "keep" and no_turbo.read_text().strip() == "1")):
          base = policy / "base_frequency"
          if base.exists():
            upper = min(upper, int(base.read_text()))
        if not lower <= frequency_khz <= upper:
          raise RuntimeError(f"Frequency {frequency_khz} kHz is outside {lower}..{upper} "
                             f"for {policy.name} with Turbo {turbo}")
        available = policy / "scaling_available_frequencies"
        if available.exists() and frequency_khz not in map(int, available.read_text().split()):
          raise RuntimeError(f"Frequency {frequency_khz} kHz is unavailable for {policy.name}")
        for name in ("scaling_min_freq", "scaling_max_freq"):
          path = policy / name
          changes.append((path, path.read_text().strip(), str(frequency_khz)))
      if remaining:
        raise RuntimeError(f"No CPUFreq policy found for CPUs {cpu_list(remaining)}")

    if smt == "off":
      if not cpus:
        raise RuntimeError("SMT control requires selected CPUs")
      cpus = set(cpus)
      for cpu in cpus:
        siblings = physical_cores({cpu}, self.cpu_root)
        if len(siblings & cpus) != 1:
          raise RuntimeError("Selected CPUs share a physical core; choose one "
                             "thread per core or use --smt keep")
      for sibling in sorted(physical_cores(cpus, self.cpu_root) - cpus):
        path = self.cpu_root / f"cpu{sibling}/online"
        if not path.exists():
          raise RuntimeError(f"SMT sibling CPU {sibling} cannot be offlined; "
                             f"select CPU {sibling} instead or use --smt keep")
        original = path.read_text().strip()
        if original == "1":
          # Restore online CPUs before restoring their CPUFreq policies.
          changes.append((path, original, "0"))
    return changes

  @staticmethod
  def write(path, value):
    path.write_text(value + "\n")

  def set_frequency_range(self, policy, minimum, maximum):
    # Widen the range before narrowing it; this also works after partial setup
    # or recovery from SIGKILL, regardless of which bound was last written.
    low, high = policy / "scaling_min_freq", policy / "scaling_max_freq"
    bounds = [(high, maximum), (low, minimum)]
    if int(maximum) < int(low.read_text()):
      bounds.reverse()
    for path, value in bounds:
      if path.read_text().strip() != value:
        self.write(path, value)
      # CPUFreq updates its effective policy from a workqueue after the sysfs
      # write returns. Wait for each bound before relying on it for the next
      # write, and before declaring setup or restoration complete.
      deadline = time.monotonic() + 1
      while path.read_text().strip() != value:
        if time.monotonic() >= deadline:
          raise RuntimeError(f"{path}: driver did not accept {value} kHz; "
                             f"observed {path.read_text().strip()} kHz")
        time.sleep(0.01)
    if low.read_text().strip() != minimum or high.read_text().strip() != maximum:
      raise RuntimeError(f"{policy}: driver did not accept frequency limits "
                         f"{minimum}..{maximum} kHz")

  def apply(self, changes):
    frequency_targets = {path: target for path, _, target in changes
                         if path.name in ("scaling_min_freq", "scaling_max_freq")}
    for path, original, target in changes:
      if path.name == "scaling_max_freq":
        continue  # Both limits are handled together at scaling_min_freq.
      if path.name == "scaling_min_freq":
        high = path.parent / "scaling_max_freq"
        originals = {p: old for p, old, _ in changes if p in (path, high)}
        for p, old in originals.items():
          self.remember(p, old)
        self.set_frequency_range(path.parent, target, frequency_targets[high])
        print(f"{path.parent}: frequency limits -> {target} kHz", file=sys.stderr)
        continue
      # Save before writing so even a partially failed write is rolled back.
      self.remember(path, original)
      if path.read_text().strip() != target:
        self.write(path, target)
      print(f"{path}: {original} -> {target}", file=sys.stderr)

  def restore(self, before_controls=None):
    errors = []
    # Bring CPUs online, restore the optional platform profile, then restore
    # Turbo, frequency bounds, governors and EPP/EPB. Replaying individual
    # frequency writes backwards is unsafe after an interrupted setup.
    frequency = {path: value for path, value in self.saved
                 if path.name in ("scaling_min_freq", "scaling_max_freq")}
    regular = [(path, original) for path, original in reversed(self.saved) if path not in frequency]
    early = [(p, v) for p, v in regular if p.name in ("online", "no_turbo", "boost")]
    late = [(p, v) for p, v in regular if p.name not in ("online", "no_turbo", "boost")]
    # EPP/EPB writes must follow governor restoration (performance can lock EPP).
    late.sort(key=lambda item: item[0].name in ("energy_performance_preference", "energy_perf_bias"))
    for path, original in early:
      if path.name != "online":
        continue
      try:
        if path.read_text().strip() != original:
          self.write(path, original)
      except OSError as error:
        errors.append(f"{path}: {error}")
    if before_controls is not None:
      errors.extend(before_controls())
    for path, original in early:
      if path.name == "online":
        continue
      try:
        if path.read_text().strip() != original:
          self.write(path, original)
      except OSError as error:
        errors.append(f"{path}: {error}")
    for policy in sorted({p.parent for p in frequency}):
      try:
        self.set_frequency_range(policy, frequency[policy / "scaling_min_freq"],
                                 frequency[policy / "scaling_max_freq"])
      except (OSError, RuntimeError) as error:
        errors.append(f"{policy}: {error}")
    for path, original in late:
      try:
        if path.read_text().strip() != original:
          self.write(path, original)
      except OSError as error:
        errors.append(f"{path}: {error}")
    return errors


class CpusetIsolation:
  """Reserve whole cores in a temporary cgroup v2 isolated partition."""

  def __init__(self, cpus, controller_pid, cpu_root=CPU_ROOT,
               cgroup_root=CGROUP_ROOT, proc_root=Path("/proc")):
    self.cpus = set(cpus)
    self.controller_pid = controller_pid
    self.cpu_root = cpu_root
    self.root = cgroup_root
    self.proc_root = proc_root
    self.group = self.root / f"perf-ninja-{controller_pid}"
    self.created = False
    self.enabled_cpuset = False
    self.moved = False

  def plan(self):
    if not (self.root / "cgroup.controllers").exists():
      raise RuntimeError("Isolation requires a cgroup v2 mount at /sys/fs/cgroup")
    if "cpuset" not in (self.root / "cgroup.controllers").read_text().split():
      raise RuntimeError("The cgroup v2 cpuset controller is unavailable")
    if not (self.root / "cpuset.cpus.isolated").exists():
      raise RuntimeError("The kernel does not support isolated cpuset partitions")
    if not os.access(self.root, os.W_OK):
      raise RuntimeError("The cgroup v2 mount is not writable")
    self.reserved = physical_cores(self.cpus, self.cpu_root)
    online = parse_cpu_list((self.cpu_root / "online").read_text())
    effective = parse_cpu_list((self.root / "cpuset.cpus.effective").read_text())
    if not self.cpus <= online or not (self.reserved & online) <= effective:
      raise RuntimeError("Requested cores are offline or reserved by another cpuset partition")
    if not online - self.reserved:
      raise RuntimeError("Isolation must leave at least one physical core for the system")
    if self.group.exists():
      raise RuntimeError(f"Cgroup already exists: {self.group}")
    self.memory_nodes = (self.root / "cpuset.mems.effective").read_text().strip()
    entry = next((line[3:] for line in
                  (self.proc_root / str(self.controller_pid) / "cgroup").read_text().splitlines()
                  if line.startswith("0::")), None)
    if entry is None:
      raise RuntimeError("The controller has no cgroup v2 membership")
    self.original_group = (self.root / entry.lstrip("/")).resolve()
    if not self.original_group.is_relative_to(self.root.resolve()):
      raise RuntimeError("The original cgroup is outside the visible cgroup mount")
    if not (self.original_group / "cgroup.procs").exists():
      raise RuntimeError("The original cgroup is not accessible")
    self.original_affinity = os.sched_getaffinity(self.controller_pid)
    self.original_start = self.process_start()
    self.cpuset_enabled_before = "cpuset" in (self.root / "cgroup.subtree_control").read_text().split()

  def snapshot(self):
    return {
      "cpus": sorted(self.cpus), "controller_pid": self.controller_pid,
      "controller_start": self.original_start,
      "original_affinity": sorted(self.original_affinity),
      "original_group": self.original_group.relative_to(self.root.resolve()).as_posix(),
      "cpuset_enabled_before": self.cpuset_enabled_before,
    }

  def process_start(self):
    # A command name can contain spaces and parentheses; starttime is field 22.
    return (self.proc_root / str(self.controller_pid) / "stat").read_text().rsplit(")", 1)[1].split()[19]

  @staticmethod
  def write(path, value):
    path.write_text(value + "\n")

  def make_group(self):
    self.group.mkdir()

  def remove_group(self):
    self.group.rmdir()

  def apply(self):
    if "cpuset" not in (self.root / "cgroup.subtree_control").read_text().split():
      self.enabled_cpuset = True
      self.write(self.root / "cgroup.subtree_control", "+cpuset")
    self.make_group()
    self.created = True
    self.write(self.group / "cpuset.mems", self.memory_nodes)
    self.write(self.group / "cpuset.cpus", cpu_list(self.reserved))
    self.write(self.group / "cpuset.cpus.partition", "isolated")
    state = (self.group / "cpuset.cpus.partition").read_text().strip()
    effective = parse_cpu_list((self.group / "cpuset.cpus.effective").read_text())
    if state != "isolated" or not self.cpus <= effective:
      raise RuntimeError(f"CPU isolation failed: {state}, effective CPUs {cpu_list(effective)}")
    if self.process_start() != self.original_start:
      raise RuntimeError("The controller exited during CPU isolation setup")
    # Children inherit this cgroup without a privileged command launcher.
    # The sudo helper stays outside it on the housekeeping CPUs.
    self.moved = True
    self.write(self.group / "cgroup.procs", str(self.controller_pid))
    print(f"Isolated physical cores: {cpu_list(self.reserved)}", file=sys.stderr)

  def restore(self):
    errors = []
    if self.created:
      try:
        # Release the CPUs even if moving a remaining process back fails.
        self.write(self.group / "cpuset.cpus.partition", "member")
      except OSError as error:
        errors.append(f"{self.group}: {error}")
      if self.moved:
        try:
          destination = self.original_group
          if not (destination / "cgroup.procs").exists():
            destination = self.root
            print("Original cgroup disappeared; returning remaining processes to the root cgroup",
                  file=sys.stderr)
          # Normally only the sleeping controller remains. Also return any
          # daemonized descendants instead of leaving CPUs reserved for them.
          for _ in range(5):
            pids = (self.group / "cgroup.procs").read_text().split()
            if not pids:
              break
            for pid in pids:
              try:
                self.write(destination / "cgroup.procs", pid)
              except OSError as error:
                if error.errno != errno.ESRCH:
                  raise
        except OSError as error:
          errors.append(f"Returning processes to {self.original_group}: {error}")
      try:
        self.remove_group()
        self.created = False
      except OSError as error:
        errors.append(f"Removing {self.group}: {error}")
    if self.enabled_cpuset:
      try:
        # Do not discard cpuset settings added by another manager during a run.
        for child in self.root.iterdir():
          if not child.is_dir():
            continue
          for name in ("cgroup.subtree_control", "cpuset.cpus", "cpuset.mems",
                       "cpuset.cpus.exclusive", "cpuset.cpus.partition"):
            path = child / name
            if path.exists():
              value = path.read_text().strip()
              if ((name == "cgroup.subtree_control" and "cpuset" in value.split()) or
                  (name == "cpuset.cpus.partition" and value != "member") or
                  (name not in ("cgroup.subtree_control", "cpuset.cpus.partition") and value)):
                raise RuntimeError(f"Concurrent cpuset changes in {child}; leaving cpuset enabled")
        self.write(self.root / "cgroup.subtree_control", "-cpuset")
        self.enabled_cpuset = False
      except (OSError, RuntimeError) as error:
        errors.append(str(error))
    return errors

  def restore_affinity(self):
    if self.moved:
      try:
        if self.process_start() == self.original_start:
          os.sched_setaffinity(self.controller_pid, self.original_affinity)
      except (FileNotFoundError, ProcessLookupError):
        pass
      except OSError as error:
        return [f"Restoring controller affinity: {error}"]
    return []


def environment_snapshot(changes, isolation, cpu_root, platform=None):
  data = {
    "version": 1, "boot_id": boot_id(), "helper_pid": os.getpid(),
    "cpu_settings": [{"path": path.relative_to(cpu_root).as_posix(),
                      "original": original, "target": target}
                     for path, original, target in changes],
    "isolation": isolation.snapshot() if isolation is not None else None,
  }
  if platform is not None:
    data["platform_settings"] = platform.snapshot()
  return data


def environment_from_snapshot(data, cpu_root=CPU_ROOT, cgroup_root=CGROUP_ROOT,
                              proc_root=Path("/proc"), platform_root=PLATFORM_ROOT,
                              power_root=POWER_ROOT):
  """Validate every saved value before restoring anything as root."""
  entries = data.get("cpu_settings")
  if not isinstance(entries, list):
    raise ValueError("Invalid CPU settings snapshot")
  settings = CpuSettings(cpu_root)
  seen = set()
  for entry in entries:
    if not isinstance(entry, dict):
      raise ValueError("Invalid CPU setting entry")
    relative, original, target = (entry.get(key) for key in ("path", "original", "target"))
    if (not isinstance(relative, str) or not re.fullmatch(
        r"cpufreq/policy[0-9]+/(?:scaling_(?:governor|min_freq|max_freq)|energy_performance_preference)|intel_pstate/no_turbo|cpufreq/boost|cpu[0-9]+/(?:online|power/energy_perf_bias)",
        relative) or relative in seen):
      raise ValueError("Invalid or duplicate CPU setting path")
    seen.add(relative)
    values = (original, target)
    if relative.endswith(("/scaling_governor", "/energy_performance_preference")):
      valid = all(isinstance(value, str) and re.fullmatch(r"[a-zA-Z0-9_-]+", value) for value in values)
    elif relative.endswith("/energy_perf_bias"):
      valid = all(isinstance(value, str) and re.fullmatch(r"[0-9]|1[0-5]", value) for value in values)
    elif relative.endswith(("/scaling_min_freq", "/scaling_max_freq")):
      valid = all(isinstance(value, str) and re.fullmatch(r"[1-9][0-9]*", value) for value in values)
    else:
      valid = all(value in ("0", "1") for value in values)
    path = cpu_root / relative
    if not valid or not path.resolve().is_relative_to(cpu_root.resolve()):
      raise ValueError("Invalid CPU setting value or path")
    settings.saved.append((path, original))

  frequency = {p: v for p, v in settings.saved if p.name in ("scaling_min_freq", "scaling_max_freq")}
  for policy in {p.parent for p in frequency}:
    low, high = policy / "scaling_min_freq", policy / "scaling_max_freq"
    if low not in frequency or high not in frequency or int(frequency[low]) > int(frequency[high]):
      raise ValueError("Incomplete or invalid saved frequency range")

  saved = data.get("isolation")
  isolation = None
  if saved is not None:
    def valid_cpus(cpus):
      return isinstance(cpus, list) and bool(cpus) and all(type(cpu) is int and cpu >= 0 for cpu in cpus)

    if (not isinstance(saved, dict) or type(saved.get("controller_pid")) is not int or
        saved["controller_pid"] <= 0 or not valid_cpus(saved.get("cpus")) or
        not valid_cpus(saved.get("original_affinity")) or
        not isinstance(saved.get("controller_start"), str) or
        not saved["controller_start"].isdecimal() or
        type(saved.get("cpuset_enabled_before")) is not bool):
      raise ValueError("Invalid CPU isolation snapshot")
    relative = saved.get("original_group")
    if (not isinstance(relative, str) or not relative or Path(relative).is_absolute() or
        ".." in Path(relative).parts):
      raise ValueError("Invalid original cgroup path")
    isolation = CpusetIsolation(saved["cpus"], saved["controller_pid"], cpu_root, cgroup_root, proc_root)
    isolation.original_group = (cgroup_root / relative).resolve()
    if not isolation.original_group.is_relative_to(cgroup_root.resolve()):
      raise ValueError("Original cgroup path escapes the cgroup mount")
    if isolation.group.is_symlink() or (isolation.group.exists() and not isolation.group.is_dir()):
      raise ValueError("Invalid temporary cgroup path")
    isolation.original_affinity = set(saved["original_affinity"])
    isolation.original_start = saved["controller_start"]
    isolation.created = isolation.group.exists()
    isolation.moved = True  # Migration or cleanup may have been interrupted.
    isolation.enabled_cpuset = (not saved["cpuset_enabled_before"] and
      "cpuset" in (cgroup_root / "cgroup.subtree_control").read_text().split())
  platform = None
  if "platform_settings" in data:
    platform = PlatformSettings.from_snapshot(data["platform_settings"], platform_root, power_root)
  return settings, isolation, platform


def restore_environment(settings, isolation, platform=None):
  errors = isolation.restore() if isolation is not None else []
  def restore_profile():
    if not platform.active:
      return []
    failures = settings.allow_profile_epp_changes() if platform.data["daemon_profile"] is not None else []
    failures.extend(platform.restore_profile())
    return failures
  errors.extend(settings.restore(restore_profile) if platform is not None else settings.restore())
  if platform is not None:
    errors.extend(platform.restore_power())
  if isolation is not None:
    errors.extend(isolation.restore_affinity())
  return errors


def restore_journal(journal, cpu_root, cgroup_root, proc_root,
                    platform_root=PLATFORM_ROOT, power_root=POWER_ROOT):
  """Recover while the caller holds the settings lock."""
  settings, isolation, platform = environment_from_snapshot(
    journal.load(), cpu_root, cgroup_root, proc_root, platform_root, power_root)
  errors = restore_environment(settings, isolation, platform)
  if errors:
    raise RuntimeError("Error restoring CPU settings:\n" + "\n".join(errors))
  journal.clear()
  print("CPU settings restored; saved state removed", file=sys.stderr)


def restore_saved_state(cpu_root=CPU_ROOT, lock_path=LOCK_PATH, *, state_path=None,
                        cgroup_root=CGROUP_ROOT, proc_root=Path("/proc"),
                        platform_root=PLATFORM_ROOT, power_root=POWER_ROOT):
  journal = StateJournal(state_path if state_path is not None else lock_path.with_suffix(".json"))
  try:
    with lock_path.open("a") as lock:
      try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
      except BlockingIOError:
        raise RuntimeError("A stable_run.py settings helper is active; recovery was not started")
      if not journal.exists():
        print("No saved CPU state to restore", file=sys.stderr)
        return 0
      restore_journal(journal, cpu_root, cgroup_root, proc_root, platform_root, power_root)
      return 0
  except (OSError, RuntimeError, ValueError) as error:
    print(f"Error: {error}", file=sys.stderr)
    if journal.exists():
      print(f"Saved state retained at {journal.path}; retry stable_run.py --restore", file=sys.stderr)
    return 1


def serve_settings(governor, turbo, cpu_root=CPU_ROOT, lock_path=LOCK_PATH, *,
                   cpus=(), smt="keep", isolate=False, controller_pid=None,
                   cgroup_root=CGROUP_ROOT, proc_root=Path("/proc"), state_path=None,
                   frequency_khz=None, frequency_scope="selected", platform_profile="keep",
                   platform_root=PLATFORM_ROOT, power_root=POWER_ROOT):
  """Hold privileges and the lock until the controller closes its pipe."""
  settings = CpuSettings(cpu_root)
  isolation = None
  platform = None
  journal = StateJournal(state_path if state_path is not None else lock_path.with_suffix(".json"))
  result = 0
  try:
    with lock_path.open("a") as lock:
      try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
      except BlockingIOError:
        print("Error: another stable_run.py session is active", file=sys.stderr)
        return 1
      try:
        if journal.exists():
          print(f"Recovering saved CPU state from {journal.path} before the new run", file=sys.stderr)
          restore_journal(journal, cpu_root, cgroup_root, proc_root, platform_root, power_root)
        # A stale partition or offline SMT thread may have made these CPUs
        # unavailable until recovery. Validate the controller, not the helper.
        if controller_pid is not None:
          unavailable = set(cpus) - os.sched_getaffinity(controller_pid)
          if unavailable:
            raise RuntimeError(f"CPUs {cpu_list(unavailable)} are outside the current CPU affinity")
        # Read every original value and validate everything before changing it.
        changes = settings.plan(governor, turbo, cpus, smt, frequency_khz, frequency_scope)
        snapshot_changes = changes
        if platform_profile not in ("keep", "performance"):
          raise ValueError("Platform profile must be performance or keep")
        if platform_profile != "keep":
          platform = PlatformSettings(platform_root, power_root)
          platform.plan()
          snapshot_changes = settings.profile_snapshot(changes)
        if isolate:
          isolation = CpusetIsolation(cpus, controller_pid, cpu_root, cgroup_root, proc_root)
          isolation.plan()
        journal.save(environment_snapshot(snapshot_changes, isolation, cpu_root, platform))
        print(f"Saved initial CPU state: {journal.path}", file=sys.stderr)
        if platform is not None:
          settings.saved = [(path, original) for path, original, _ in snapshot_changes]
          if platform.data["daemon_profile"] is not None:
            errors = settings.allow_profile_epp_changes()
            if errors:
              raise RuntimeError("\n".join(errors))
          platform.apply()
        settings.apply(changes)
        if isolation is not None:
          isolation.apply()
        sys.stdout.write(READY)
        sys.stdout.flush()
        # EOF also arrives if the controller crashes. The benchmark does not
        # inherit this pipe, so it cannot accidentally postpone restoration.
        sys.stdin.read(1)
      except (OSError, RuntimeError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        if journal.exists() and not journal.owned:
          print(f"Saved state retained at {journal.path}; retry the run or stable_run.py --restore",
                file=sys.stderr)
        result = 1
      finally:
        errors = restore_environment(settings, isolation, platform)
        if journal.owned and not errors:
          try:
            journal.clear()
          except OSError as error:
            errors.append(f"Removing saved state {journal.path}: {error}")
        if errors:
          print("Error restoring CPU settings:\n" + "\n".join(errors), file=sys.stderr)
          if journal.exists():
            print(f"Saved state retained at {journal.path}; retry stable_run.py --restore", file=sys.stderr)
          result = 1
        elif settings.saved:
          print("CPU settings restored", file=sys.stderr)
          if platform is not None:
            print("Platform profile and RAPL settings restored", file=sys.stderr)
  except OSError as error:
    print(f"Error: {error}", file=sys.stderr)
    result = 1
  return result


def helper_command(governor, turbo, *, cpus=(), smt="keep", isolate=False,
                   controller_pid=None, frequency_khz=None, frequency_scope="selected",
                   platform_profile="keep"):
  return ["sudo", "--", sys.executable, str(Path(__file__).resolve()),
          "--_settings", governor, turbo, cpu_list(cpus), smt,
          str(controller_pid), "1" if isolate else "0",
          str(frequency_khz) if frequency_khz is not None else "keep", frequency_scope,
          platform_profile]


def main(argv=None):
  parser = argparse.ArgumentParser(
    description="Temporarily set CPU power settings, optionally isolate physical "
                "cores, disable their SMT siblings and disable command ASLR.",
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog="""Run as your normal user. Only the settings helper uses sudo; the command
keeps your user, environment and working directory. Governor and Turbo settings
apply system-wide; --cpu only pins the command. --cpus selects the cores for
--smt, --isolate and --frequency-khz (defaults to --cpu). --frequency-scope all
applies frequency limits to every CPUFreq policy without changing the selected
cores for SMT control and isolation. Isolation uses a temporary cgroup v2
isolated partition and leaves at least one physical core for the system.
Per-CPU kernel threads and hardware interrupts can still run on isolated CPUs.
--platform-profile performance saves the original platform/daemon profiles and
writable Intel RAPL power limits, time windows and enable flags before selecting
performance. It restores the profile before CPU controls, and RAPL settings last.
The direct invocation default is --platform-profile keep; the lab wrappers use
performance by default. powerprofilesctl is used when its daemon is available,
otherwise the helper uses the platform sysfs interface.
ASLR is changed only for the command and its descendants; privileged executables
may reset that flag. Concurrent wrapper runs are rejected. Restoration covers
command errors, Ctrl+C, SIGTERM and SIGHUP.
Original state is saved atomically in /run/perf-ninja-stable-run.json before
changes. A new run automatically restores any leftover snapshot under the same
lock before checking CPU availability and saving a fresh snapshot. If recovery
fails, the new command is not started and the old snapshot is retained.
stable_run.py --restore performs recovery without running a command.
/run is cleared on reboot; snapshots are checked against the current boot.

Examples (from the repository root):
  tools/misc/stable_run.py --cpu 2 -- ./labs/misc/warmup/build/lab
  tools/misc/stable_run.py --cpu 2 --isolate --smt off --aslr off -- ./labs/misc/warmup/build/lab
  tools/misc/stable_run.py --cpu 2 --turbo on --frequency-khz 3500000 --frequency-scope all -- ./labs/misc/warmup/build/lab
  tools/misc/stable_run.py --cpu 2 -- perf stat -e cycles:u ./labs/misc/warmup/build/lab
  tools/misc/stable_run.py --turbo keep -- ./labs/misc/warmup/build/lab
  tools/misc/stable_run.py --platform-profile performance --turbo on -- ./labs/misc/warmup/build/lab
  tools/misc/stable_run.py --restore
""",
  )
  parser.add_argument("--governor", default="performance",
                      help="CPUFreq governor for all policies, or keep (default: performance)")
  parser.add_argument("--turbo", choices=("off", "on", "keep"), default="off",
                      help="Turbo/boost mode (default: off)")
  parser.add_argument("--platform-profile", choices=("performance", "keep"), default="keep",
                      help="temporarily select performance, restoring the original profile and RAPL limits (default: keep)")
  parser.add_argument("--frequency-khz", type=int,
                      help="set equal min/max frequency limits in kHz; "
                           "hardware power/thermal limits still apply (default: keep)")
  parser.add_argument("--frequency-scope", choices=("selected", "all"), default="selected",
                      help="apply frequency limits to selected CPUs' policies or every policy (default: selected)")
  parser.add_argument("--cpu", type=int,
                      help="optionally pin the command to one logical CPU with taskset")
  parser.add_argument("--cpus", type=parse_cpu_list,
                      help="CPU list for SMT/isolation/frequency limits, e.g. 2-3 (defaults to --cpu)")
  parser.add_argument("--isolate", action="store_true",
                      help="reserve the selected physical cores using cgroup v2")
  parser.add_argument("--smt", choices=("off", "keep"), default="keep",
                      help="offline unselected SMT siblings of selected CPUs (default: keep)")
  parser.add_argument("--aslr", choices=("off", "keep"), default="keep",
                      help="disable ASLR for the command with setarch (default: keep)")
  parser.add_argument("--restore", action="store_true",
                      help="restore the saved state after SIGKILL or a failed cleanup")
  parser.add_argument("command", nargs=argparse.REMAINDER,
                      help="command and arguments, following --")
  args = parser.parse_args(argv)
  command = args.command
  if command and command[0] == "--":
    command = command[1:]
  if args.restore:
    if command:
      parser.error("--restore does not accept a command")
    if os.geteuid() == 0:
      return restore_saved_state()
    try:
      return subprocess.call(["sudo", "--", sys.executable,
                              str(Path(__file__).resolve()), "--_restore"])
    except OSError as error:
      print(f"Error starting recovery: {error}", file=sys.stderr)
      return 1
  if not command:
    parser.error("a command is required, for example: -- ./build/lab")
  if os.geteuid() == 0:
    parser.error("run as your normal user; the script invokes sudo for CPU settings")
  if shutil.which("sudo") is None:
    parser.error("sudo is not installed")
  cpus = args.cpus if args.cpus is not None else ({args.cpu} if args.cpu is not None else set())
  if args.frequency_khz is not None and args.frequency_khz <= 0:
    parser.error("--frequency-khz must be a positive integer")
  if args.frequency_scope == "all" and args.frequency_khz is None:
    parser.error("--frequency-scope all requires --frequency-khz")
  if args.isolate or args.smt == "off" or (args.frequency_khz is not None and args.frequency_scope == "selected"):
    if not cpus:
      parser.error("isolation, SMT control and frequency limits require --cpu or a nonempty --cpus list")
    if args.cpu is not None and args.cpu not in cpus:
      parser.error("--cpus must include --cpu")
  if args.aslr == "off":
    setarch = shutil.which("setarch")
    if setarch is None:
      parser.error("setarch is not installed")
    # Fail before any system changes if personality(ADDR_NO_RANDOMIZE) is denied.
    try:
      probe = subprocess.run([setarch, "--addr-no-randomize", sys.executable, "-c", "pass"],
                             capture_output=True, text=True)
    except OSError as error:
      parser.error(f"ASLR could not be disabled: {error}")
    if probe.returncode:
      parser.error("ASLR could not be disabled; use --aslr keep to skip: " + probe.stderr.strip())
    command = [setarch, "--addr-no-randomize", *command]
  if args.cpu is not None:
    if shutil.which("taskset") is None:
      parser.error("taskset is not installed")
    command = ["taskset", "-c", str(args.cpu), *command]

  child = None
  received_signal = None

  def forward_signal(signum, _frame):
    nonlocal received_signal
    received_signal = signum
    if child is not None and child.poll() is None:
      try:
        os.killpg(child.pid, signum)
      except ProcessLookupError:
        pass

  old_handlers = {}
  helper = None
  result = 1
  try:
    for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
      old_handlers[signum] = signal.signal(signum, forward_signal)
    helper = subprocess.Popen(helper_command(args.governor, args.turbo,
                              cpus=cpus, smt=args.smt, isolate=args.isolate,
                              frequency_khz=args.frequency_khz,
                              frequency_scope=args.frequency_scope,
                              platform_profile=args.platform_profile,
                              controller_pid=os.getpid()),
                              stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                              text=True)
    if helper.stdout.readline() != READY:
      print("Error: CPU settings could not be prepared; command was not started",
            file=sys.stderr)
    elif received_signal is None:
      print("Running: " + shlex.join(command), file=sys.stderr)
      # A separate session lets signals reach the entire benchmark process tree
      # without killing the privileged settings helper before it can restore.
      child = subprocess.Popen(command, start_new_session=True)
      if received_signal is not None:
        forward_signal(received_signal, None)
      result = child.wait()
      if result < 0:
        result = 128 - result
  except FileNotFoundError as error:
    print(f"Error: {error}", file=sys.stderr)
    result = 127
  except OSError as error:
    print(f"Error: {error}", file=sys.stderr)
    result = 126
  finally:
    if helper is not None:
      try:
        helper.stdin.close()
      except BrokenPipeError:
        pass
      helper.stdout.close()
      if helper.wait() != 0 and result == 0:
        result = 1
    for signum, handler in old_handlers.items():
      signal.signal(signum, handler)
  if received_signal is not None:
    return 128 + received_signal
  return result


if __name__ == "__main__":
  if sys.argv[1:] == ["--_restore"]:
    if os.geteuid() != 0:
      sys.exit("Error: CPU state recovery must run as root")
    for signum in (signal.SIGINT, signal.SIGQUIT, signal.SIGTERM, signal.SIGHUP):
      signal.signal(signum, signal.SIG_IGN)
    sys.exit(restore_saved_state())
  if len(sys.argv) in (10, 11) and sys.argv[1] == "--_settings":
    if os.geteuid() != 0:
      sys.exit("Error: the CPU settings helper must run as root")
    # The controller forwards terminal signals to the command and then closes
    # the pipe. Keep the helper alive until that command has stopped.
    for signum in (signal.SIGINT, signal.SIGQUIT, signal.SIGTERM, signal.SIGHUP):
      signal.signal(signum, signal.SIG_IGN)
    sys.exit(serve_settings(sys.argv[2], sys.argv[3], cpus=parse_cpu_list(sys.argv[4]),
                            smt=sys.argv[5], controller_pid=int(sys.argv[6]),
                            isolate=sys.argv[7] == "1",
                            frequency_khz=None if sys.argv[8] == "keep" else int(sys.argv[8]),
                            frequency_scope=sys.argv[9],
                            platform_profile=sys.argv[10] if len(sys.argv) == 11 else "keep"))
  sys.exit(main())
