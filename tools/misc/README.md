# Building, benchmarking, and profiling labs

Scripts for working with perf-ninja labs locally on Linux. They build labs,
run benchmarks with temporary CPU settings, and record `perf` profiles.
Run the examples below from the repository root unless stated otherwise.

## Quick start

With the dependencies installed and Google Benchmark built:

```bash
source tools/misc/env.sh

build_lab.sh misc/warmup
cmake --build labs/misc/warmup/build --target validateLab

run_lab.sh misc/warmup --benchmark_min_time=5s --benchmark_repetitions=5
perf_lab.sh misc/warmup --benchmark_min_time=5s
perf report -i labs/misc/warmup/build/perf.data
```

Run the wrappers as your normal user. `stable_run.py` invokes `sudo` for a helper
that changes system settings; `lab` and `perf` run as your user. Configure access
to `perf` events separately: the helper's use of `sudo` does not grant those
permissions to `perf`.

## Requirements and setup

- Bash, Python 3.9+ with its standard library, `sudo`, `taskset`, and `setarch`.
- For building: CMake 3.25+, Ninja, and `clang` and `clang++` in `PATH`.
- Google Benchmark: CMake looks for a local build in `tools/benchmark/build`;
  an installed `benchmark` package or `benchmark_DIR` can also be used.
- For profiling: `perf` installed and permission to run `perf record` as your
  normal user.
- For `tools/check_speedup.py`: SciPy for the comparison report.
- For the default settings: the CPUFreq `performance` governor, Turbo/boost
  controls in sysfs, support for 3.5 GHz in every CPUFreq policy, and a platform
  exposing `performance` in `/sys/firmware/acpi/platform_profile_choices`.
  `powerprofilesctl` is used when its daemon is available; otherwise the helper
  uses the platform sysfs interface directly. Use `PLATFORM_PROFILE=keep` if
  platform profiles are unavailable or should remain unchanged.
  Use `CPU_FREQ_KHZ=keep` to skip explicit frequency limits or select a
  different supported frequency. Isolation requires cgroup v2 at `/sys/fs/cgroup`,
  writable by the root helper, with the `cpuset` controller and isolated partitions.
  Disabling SMT requires writable `cpuN/online` files for the sibling threads.

If Google Benchmark has not been built locally, run this once:

```bash
(cd tools && bash make_benchmark_library.sh)
```

This script downloads Google Benchmark and Google Test, then builds the library.
Run it from `tools`. General course setup instructions are in
[GetStarted.md](../../GetStarted.md) and [QuickstartLinux.md](../../QuickstartLinux.md).

## Scripts

| File | Purpose |
| --- | --- |
| [env.sh](env.sh) | Source this file to add this directory to `PATH` in the current Bash session. Sourcing it again does not duplicate the entry or change the working directory or shell settings. |
| [build_lab.sh](build_lab.sh) | Configures and builds a lab in `<lab>/build`: Release with debug information (`-g`), using Clang and Ninja by default. |
| [run_lab.sh](run_lab.sh) | Runs an existing `lab` binary through `stable_run.py`. Arguments are forwarded unchanged. |
| [perf_lab.sh](perf_lab.sh) | Runs an existing `lab` binary through `perf record` and `stable_run.py`, pinning the lab and the recording process to selected CPUs. |
| [stable_run.py](stable_run.py) | Temporarily configures the environment for an arbitrary command, saves the original state, and restores it after completion or a crash. |
| [platform_power.py](platform_power.py) | Internal helper for platform profiles and snapshots of writable Intel RAPL controls. |
| `test_stable_run.py`, `test_cpu_isolation.py`, `test_state_journal.py`, `test_frequency.py`, `test_platform_power.py`, `test_lab_wrappers.py`, `test_check_speedup.py` | Tests for wrapper/comparison defaults, CPU and platform settings, power limits, isolation, and recovery, including `SIGKILL`. |

Without sourcing `env.sh`, invoke the scripts by path, for example
`tools/misc/run_lab.sh misc/warmup` from the repository root.

## Selecting and building a lab

All three wrappers, `build_lab.sh`, `run_lab.sh`, and `perf_lab.sh`, accept
`category/lab`, a repository-relative `labs/category/lab` path, or an existing
absolute or relative path to a lab directory as their first argument.
If omitted, the current directory is used.

After sourcing `env.sh`, you can work directly from a lab directory:

```bash
(
  cd labs/memory_bound/loop_interchange_1
  build_lab.sh
  cmake --build build --target validateLab
  run_lab.sh --benchmark_min_time=5s
  perf_lab.sh --benchmark_min_time=5s
)
```

Remaining arguments to `build_lab.sh` are passed to CMake. `JOBS` controls build
parallelism and defaults to `8`:

```bash
JOBS=4 build_lab.sh misc/warmup -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Use `CC`, `CXX`, `CMAKE_GENERATOR`, or `-G` to select the compiler and generator.
For example, for a fresh build with GCC:

```bash
CC=gcc CXX=g++ build_lab.sh misc/warmup
```

For an existing build, account for the compiler and generator stored in the
CMake cache. `run_lab.sh` and `perf_lab.sh` do not rebuild the lab: after editing
the sources, rerun `build_lab.sh` and the `validateLab` check.

## Benchmark settings

`run_lab.sh`, `perf_lab.sh`, and `tools/check_speedup.py` read the following
environment variables:

| Variable | Default | Meaning |
| --- | --- | --- |
| `CPU` | `2` | Logical CPU for `lab`. |
| `GOVERNOR` | `performance` | Governor for all CPUFreq policies; `keep` preserves their current values. |
| `PLATFORM_PROFILE` | `performance` | Temporarily selects the platform's `performance` profile; `keep` skips profile and power-limit management. The original profile and writable RAPL settings are restored afterwards. |
| `TURBO` | `on` | Turbo/boost mode: `off`, `on`, or `keep`. |
| `CPU_FREQ_KHZ` | `3500000` | Set both CPUFreq limits to this positive frequency in kHz for the policies chosen by `CPU_FREQ_SCOPE`; `keep` skips explicit limits. |
| `CPU_FREQ_SCOPE` | `all` | `all` applies frequency limits to every CPUFreq policy; `selected` applies them only to the lab/recorder CPU policies. Ignored when `CPU_FREQ_KHZ=keep`. |
| `ISOLATE` | `on` | `on` reserves physical cores using a temporary isolated cpuset; `off` skips this step. |
| `SMT` | `off` | `off` temporarily takes unselected SMT siblings of the selected CPUs offline; `keep` preserves their state. |
| `ASLR` | `off` | `off` disables address randomization only for the command and its descendants; `keep` preserves the inherited mode. |

The defaults `2` and `3` are logical CPU numbers, so check your machine's topology:

```bash
lscpu -e=CPU,CORE,SOCKET,ONLINE
cat /sys/devices/system/cpu/cpu2/topology/thread_siblings_list
```

Choose different physical cores for `lab` and `perf`: different `SOCKET,CORE`
pairs. With `SMT=off`, you cannot select two threads of the same physical core.
If an SMT sibling cannot be taken offline, select the other thread of that core
or use `SMT=keep`. Isolation leaves at least one physical core for the system.
Hardware interrupts and kernel threads bound to individual CPUs may still run
on isolated CPUs.

Examples of overriding settings and saving results:

```bash
CPU=1 TURBO=keep CPU_FREQ_KHZ=keep run_lab.sh misc/warmup --benchmark_repetitions=9

run_lab.sh memory_bound/loop_interchange_1 \
  --benchmark_repetitions=30

TURBO=off CPU_FREQ_KHZ=keep run_lab.sh memory_bound/loop_interchange_1

CPU_FREQ_SCOPE=selected run_lab.sh memory_bound/loop_interchange_1 --benchmark_repetitions=30

PLATFORM_PROFILE=keep run_lab.sh misc/warmup

run_lab.sh misc/warmup --benchmark_min_time=5s \
  --benchmark_repetitions=7 --benchmark_out=baseline.json --benchmark_out_format=json
```

`run_lab.sh` keeps the usual Google Benchmark behavior: set the benchmark duration,
filter, and other options through arguments or `BENCHMARK_*` variables.
Both wrappers run the binary from `<lab>/build`, so a relative
`--benchmark_out=baseline.json` writes the result there.

By default, both wrappers enable Turbo and request 3.5 GHz for every CPUFreq
policy (`TURBO=on CPU_FREQ_KHZ=3500000 CPU_FREQ_SCOPE=all`) by setting
`scaling_min_freq` and `scaling_max_freq` to the same value.
The helper rejects out-of-range frequencies and
limits that the driver rounds or clamps instead of accepting exactly.
Turbo must allow the requested frequency. Hardware power/thermal limits and
requests from other threads can still affect the actual clock, so monitor it
under load. The original limits are saved and restored, including crash recovery.

Both wrappers also select the platform profile `performance`. This setting is
separate from the CPUFreq governor. Before changing it, the helper saves the
current platform profile, the daemon profile when available, and writable Intel
RAPL controls from the MSR and MMIO interfaces: power limits, applicable time
windows, and zone enable flags. It also preserves CPU frequency limits, governors,
EPP and EPB settings that the profile manager may change. No power-limit value is
hard-coded for the benchmark: the selected profile and firmware choose the limits.

Afterwards, the helper brings CPUs online, restores the original profile, restores
CPU settings, and finally restores the saved RAPL values with readback checks.
When necessary, it temporarily uses the `powersave` governor to let the profile
manager update EPP, then restores the saved governors and EPP values.
Returning to `balanced` alone may leave the higher power limit in place, so power
limits are restored explicitly. A failed restoration returns a nonzero status and
retains the snapshot for retry. Other active power/thermal managers can still
change these settings independently.

The default `CPU_FREQ_SCOPE=all` includes the system's housekeeping CPUs.
Set `CPU_FREQ_SCOPE=selected` to limit policies containing `CPU`; `perf_lab.sh`
also includes `PERF_CPU`. A shared policy affects its other CPUs as well.
CPU affinity, isolation, and the choice
of SMT siblings to take offline still use `CPU` and `PERF_CPU`. Limits are set
before siblings go offline and restored after they come back online. All policies
must support the requested frequency.

You can skip individual features if they are unavailable. For example:

```bash
ISOLATE=off SMT=keep ASLR=keep run_lab.sh misc/warmup
PLATFORM_PROFILE=keep GOVERNOR=keep TURBO=keep CPU_FREQ_KHZ=keep ISOLATE=off SMT=keep ASLR=keep run_lab.sh misc/warmup
```

The second example only pins the command to `CPU` for the new run. A snapshot
left by a previous crash is still restored first. Governor and Turbo settings,
and taking logical CPUs offline, affect the whole system, including other
processes.

## Profiling with perf

Additional variables for `perf_lab.sh`:

| Variable | Default | Meaning |
| --- | --- | --- |
| `PERF_CPU` | `3` | Logical CPU for the `perf record` process. |
| `EVENT` | `cycles:u` | Event expression to sample; accepts perf comma lists and groups. Overridden by CLI `-e` options. |
| `FREQ` | `199` | Target samples per second. Overridden by CLI `-F` / `--freq`. |
| `CALL_GRAPH` | `dwarf` | Stack recording method for `perf record --call-graph`. |
| `OUTPUT` | `<lab>/build/perf.data` | Profile output file; its parent directory must exist. |
| `BENCHMARK_MIN_TIME` | `5s` | Minimum benchmark duration; the `--benchmark_min_time` argument takes precedence. |

Use `-e` / `--event` (alias `--events`) to choose events and `-F` / `--freq`
(alias `--frequency`) to set a positive sampling frequency in Hz. Repeat `-e`
for several event expressions or quote a comma list or group as one argument.
Expressions are passed unchanged to `perf record`. CLI events replace `EVENT`;
CLI frequency replaces `FREQ`. Without overrides, the defaults are `cycles:u`
and 199 Hz.

Wrapper options can precede or follow the lab path, but must come before the
benchmark arguments. The first unrecognized option or `--` ends wrapper option
parsing; all remaining arguments are forwarded unchanged to `lab`. If the lab
path is omitted, the current directory is used. Long options also accept `=value`.

```bash
perf_lab.sh memory_bound/loop_interchange_1 \
  -e cycles:u -e cache-misses:u -F 99 -- --benchmark_min_time=10s

perf_lab.sh --events 'cycles:u,instructions:u' --freq 199 misc/warmup

CPU=2 PERF_CPU=3 FREQ=99 perf_lab.sh memory_bound/loop_interchange_1 \
  --benchmark_min_time=10s --benchmark_repetitions=3

perf report -i labs/memory_bound/loop_interchange_1/build/perf.data
```

A relative `OUTPUT` path is resolved against the caller's working directory,
unlike relative paths in the benchmark's own arguments:

```bash
OUTPUT=/tmp/warmup.data perf_lab.sh misc/warmup --benchmark_filter=bench1
perf report -i /tmp/warmup.data
```

You can explicitly set `CPU` and `PERF_CPU` to the same CPU if separate cores
are unavailable. Do not wrap `perf_lab.sh` or `run_lab.sh` in another
`stable_run.py` invocation: they already use it, and the lock rejects concurrent
sessions.

## Comparing baseline and SOLUTION

`tools/check_speedup.py` rebuilds and compares the current lab sources with and
without the `SOLUTION` macro. Keep both implementations under `#ifdef SOLUTION`
and `#else`. Each `-num_runs` iteration creates a baseline/solution build pair;
the script recreates `baselineBuildN` and `solutionBuildN` in its working directory.

Only the measurement target runs through `stable_run.py`, using the same defaults
and environment overrides as `run_lab.sh`. Configuration, compilation and
`validateLab` run outside the temporary measurement settings. The CMake
`benchmarkLab` target supplies the lab's input arguments, benchmark duration
(normally 2 s), and `result.json` output path in each variant's build directory.

From the repository root:

```bash
repo_dir=$PWD
mkdir -p /tmp/perf-ninja-speedup
cd /tmp/perf-ninja-speedup
python3 "$repo_dir/tools/check_speedup.py" \
  -lab_path "$repo_dir/labs/misc/warmup" -num_runs 3 -v

CPU=3 CPU_FREQ_KHZ=3000000 python3 "$repo_dir/tools/check_speedup.py" \
  -lab_path "$repo_dir/labs/misc/warmup" -num_runs 3
```

Run as your normal user; only the settings helper requests `sudo`. The original
profile, power limits and CPU settings are restored after each measurement.
Benchmark/recovery diagnostics remain visible without `-v`. A failed build,
validation, measurement or restoration stops the script with a nonzero status;
it does not continue to compare a failed measurement. Recovery uses the same
snapshot and `stable_run.py --restore` as the lab wrappers.

## Using stable_run.py for any command

Use command-line options when invoking this program directly. By default it
sets the governor to `performance` and disables Turbo. Isolation is disabled,
SMT, ASLR and the platform profile are unchanged, and the command is not pinned
to a CPU. Add `--platform-profile performance` to manage the platform profile and
restore its original profile and power limits afterwards.

```bash
stable_run.py --cpu 2 --isolate --smt off --aslr off -- \
  ./labs/misc/warmup/build/lab --benchmark_min_time=5s

stable_run.py --platform-profile performance --turbo on -- \
  ./labs/misc/warmup/build/lab --benchmark_min_time=5s
```

`--cpu N` pins the command to one CPU. `--cpus 2,3` or `--cpus 2-3` selects
CPUs for isolation and SMT control; if omitted, the value of `--cpu` is used.
`--frequency-khz 3500000` sets both frequency bounds for these CPU policies.
Add `--frequency-scope all` to set the bounds for every policy without expanding
the set of isolated CPUs or offlined SMT siblings.
When isolation is enabled or SMT is disabled, this set must include the command's
CPU. The command keeps your working directory, environment, and user identity.

## Cleanup and crash recovery

Before the first change, the root helper atomically saves the original state
to `/run/perf-ninja-stable-run.json` with permissions `0600`. The snapshot contains
the original governors, Turbo/boost settings, explicitly configured frequency
limits, the state of SMT siblings to be
taken offline, and the information needed to return processes from the temporary
cgroup and restore the controller process's CPU affinity. With platform profile
management enabled, it also includes the original platform/daemon profiles,
writable RAPL power limits and time windows, zone enable flags, and CPU settings
affected by profile changes. Older snapshots without platform settings remain
supported.
The `/run/perf-ninja-stable-run.lock` lock prevents concurrent invocations of
these scripts from changing settings at the same time.

When the command exits, including with a nonzero status, the helper releases
the cpuset, returns processes, brings the disabled CPUs online, and restores
the original platform profile, Turbo, frequency limits, governors, EPP/EPB, RAPL
settings, and the controller's CPU affinity. The snapshot is removed
only after restoration succeeds. ASLR is set for child processes and does not
require a separate system-wide rollback.

On Ctrl+C, `SIGTERM`, or `SIGHUP`, the wrapper forwards the signal to the command's
process group and waits for it to finish before restoring settings. If the command
ignores the signal, the wrapper keeps waiting. If only the controller process
dies, the surviving helper detects that its communication pipe has closed and
starts recovery.

If the helper itself is killed with `kill -9`, or restoration fails, the snapshot
remains. On the next invocation of `stable_run.py`, `run_lab.sh`, or `perf_lab.sh`,
the helper holds the same lock while it:

1. Validates the snapshot and restores the previous settings.
2. Checks CPU availability and saves a fresh snapshot of the restored state.
3. Applies the new benchmark settings and allows the command to start.

If the original cgroup no longer exists, remaining processes in the temporary
cgroup are moved to the root cgroup. The script reports this in the terminal.

If recovery fails, the new command is not started and the old snapshot is kept
for another attempt. The error is printed in the terminal. To recover without
starting a new benchmark, run this as your normal user:

```bash
stable_run.py --restore
```

The program requests `sudo` itself. Repeating this command after successful
recovery reports that no saved state remains and exits successfully. If another
helper is still running and holding the lock, recovery is rejected: finish the
current benchmark first. On failure, resolve the reported cause and retry
`--restore`; do not delete the snapshot instead of restoring settings.

The file in `/run` survives process termination but is cleared on reboot.
Snapshots from a different boot are rejected. This mechanism does not restore
previous settings after a reboot.

## Help and tests

```bash
build_lab.sh --help
run_lab.sh --help
perf_lab.sh --help
stable_run.py --help

python3 -m unittest discover -s tools/misc -p 'test_*.py' -v
```

The tests use temporary files instead of the system's sysfs/cgroup files and do
not require `sudo`. They cover rollback after partial setup, signals, forcibly
terminating the helper, and manual and automatic recovery from a saved snapshot.
