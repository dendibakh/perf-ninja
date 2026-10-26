"""Snapshot and restore platform profiles and RAPL settings for stable_run."""

import errno
from pathlib import Path
import re
import shutil
import stat
import subprocess
import sys
import time


PLATFORM_ROOT = Path("/sys/firmware/acpi")
POWER_ROOT = Path("/sys/devices/virtual/powercap")
ZONE = r"intel-rapl(?:-mmio)?(?::[0-9]+)+"
POWER_PATH = re.compile(
  rf"intel-rapl(?:-mmio)?/(?:{ZONE}/)+(?:enabled|constraint_[0-9]+_(?:power_limit_uw|time_window_us))")


class PlatformSettings:
  def __init__(self, platform_root=PLATFORM_ROOT, power_root=POWER_ROOT):
    self.root = platform_root
    self.power_root = power_root
    self.profile = platform_root / "platform_profile"
    self.active = False
    self.data = None

  @staticmethod
  def control(command, *args):
    try:
      result = subprocess.run([command, *args], capture_output=True, text=True, timeout=15)
    except subprocess.TimeoutExpired as error:
      raise RuntimeError("powerprofilesctl timed out") from error
    if result.returncode:
      raise RuntimeError("powerprofilesctl: " + (result.stderr.strip() or str(result.returncode)))
    return result.stdout.strip()

  @staticmethod
  def write(path, value):
    path.write_text(value + "\n")

  def plan(self):
    if not self.profile.exists():
      raise RuntimeError("Platform profiles are unavailable; use --platform-profile keep to skip")
    choices = (self.root / "platform_profile_choices").read_text().split()
    original = self.profile.read_text().strip()
    if "performance" not in choices or original not in choices:
      raise RuntimeError("The platform must support performance and restoring its current profile; "
                         "use --platform-profile keep to skip")
    daemon = None
    command = shutil.which("powerprofilesctl")
    if command is not None:
      try:
        daemon = self.control(command, "get")
      except RuntimeError as error:
        print(f"{error}; using the platform sysfs interface", file=sys.stderr)
      if daemon is not None and daemon not in ("performance", "balanced", "power-saver"):
        raise RuntimeError("Unrecognized power-profiles-daemon profile")
    power = []
    for path in sorted(self.power_root.rglob("*")):
      relative = path.relative_to(self.power_root).as_posix()
      if not POWER_PATH.fullmatch(relative) or not path.is_file():
        continue
      if not path.stat().st_mode & (stat.S_IWUSR | stat.S_IWGRP | stat.S_IWOTH):
        continue
      try:
        original_value = path.read_text().strip()
      except OSError as error:
        # For example, peak-power constraints have no time window.
        if error.errno in (errno.ENODATA, errno.EOPNOTSUPP):
          continue
        raise
      power.append({"path":relative, "original":original_value})
    self.data = {"profile":original, "daemon_profile":daemon, "power_settings":power}
    self.validate(self.data)
    return self.data

  def validate(self, data):
    if (not isinstance(data, dict) or set(data) != {"profile", "daemon_profile", "power_settings"} or
        not isinstance(data["profile"], str) or
        not re.fullmatch(r"[a-z][a-z0-9-]*", data["profile"]) or
        data["daemon_profile"] not in (None, "performance", "balanced", "power-saver") or
        not isinstance(data["power_settings"], list)):
      raise ValueError("Invalid platform settings snapshot")
    if data["profile"] not in (self.root / "platform_profile_choices").read_text().split():
      raise ValueError("The saved platform profile is unavailable")
    seen = set()
    for entry in data["power_settings"]:
      if not isinstance(entry, dict) or set(entry) != {"path", "original"}:
        raise ValueError("Invalid saved power setting")
      relative, value = entry["path"], entry["original"]
      if (not isinstance(relative, str) or not POWER_PATH.fullmatch(relative) or relative in seen or
          not isinstance(value, str) or not re.fullmatch(r"0|[1-9][0-9]{0,19}", value) or
          int(value) > 2**64-1 or (relative.endswith("/enabled") and value not in ("0", "1")) or
          not (self.power_root / relative).resolve().is_relative_to(self.power_root.resolve())):
        raise ValueError("Invalid or duplicate power setting path/value")
      seen.add(relative)

  @classmethod
  def from_snapshot(cls, data, platform_root=PLATFORM_ROOT, power_root=POWER_ROOT):
    instance = cls(platform_root, power_root)
    instance.validate(data)
    instance.data = data
    instance.active = True
    return instance

  def snapshot(self):
    return self.data

  def set_profile(self, profile, daemon):
    if daemon is not None:
      command = shutil.which("powerprofilesctl")
      if command is None:
        raise RuntimeError("powerprofilesctl is required to restore the saved daemon profile")
      self.control(command, "set", daemon)
      if self.control(command, "get") != daemon:
        raise RuntimeError("power-profiles-daemon did not accept the profile")
    if self.profile.read_text().strip() != profile:
      self.write(self.profile, profile)
    deadline = time.monotonic()+2
    while self.profile.read_text().strip() != profile:
      if time.monotonic() >= deadline:
        raise RuntimeError(f"The platform did not accept profile {profile}")
      time.sleep(.02)

  def apply(self):
    self.active = True  # A failed daemon call may already have changed settings.
    self.set_profile("performance", "performance" if self.data["daemon_profile"] is not None else None)
    print(f"Platform profile: {self.data['profile']} -> performance", file=sys.stderr)

  def restore_profile(self):
    if not self.active:
      return []
    try:
      self.set_profile(self.data["profile"], self.data["daemon_profile"])
    except (OSError, RuntimeError) as error:
      return [f"Restoring platform profile: {error}"]
    return []

  def restore_power(self):
    if not self.active:
      return []
    errors = []
    # Restore the profile first: it can overwrite power limits asynchronously.
    # Restore limits/windows before enable flags and never touch energy counters.
    entries = sorted(self.data["power_settings"], key=lambda e:e["path"].endswith("/enabled"))
    for entry in entries:
      path, value = self.power_root / entry["path"], entry["original"]
      try:
        if path.read_text().strip() != value:
          self.write(path, value)
      except OSError as error:
        errors.append(f"{path}: {error}")
    time.sleep(.2)
    for entry in entries:
      path = self.power_root / entry["path"]
      try:
        if path.read_text().strip() != entry["original"]:
          errors.append(f"{path}: original power setting was not restored")
      except OSError as error:
        errors.append(f"{path}: {error}")
    try:
      if self.profile.read_text().strip() != self.data["profile"]:
        errors.append("The original platform profile did not remain active")
    except OSError as error:
      errors.append(f"Restoring platform profile: {error}")
    return errors
