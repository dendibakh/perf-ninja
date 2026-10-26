#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: run_lab.sh [category/lab | lab-directory] [benchmark options...]

Run an already built lab through stable_run.py, which temporarily sets the
CPU governors, Turbo mode, SMT siblings and CPU isolation, and restores them
afterwards. ASLR is disabled only for the command and its descendants.
The platform profile and RAPL power limits are also saved and restored.
The script can be called from any working directory.
If no lab is specified, use the current directory.
Run the lab from its build directory, as the CMake benchmark target does.

Defaults (override with environment variables):
  CPU=2                     Logical CPU to run on
  GOVERNOR=performance      CPUFreq governor for all policies, or keep
  PLATFORM_PROFILE=performance  Temporary platform profile: performance, keep
  TURBO=on                  Turbo/boost mode: off, on, keep
  CPU_FREQ_KHZ=3500000      Equal min/max frequency limits in kHz, or keep
  CPU_FREQ_SCOPE=all        Frequency policies: all, or selected (CPU)
  ISOLATE=on                Reserve the physical core with cgroup v2: on, off
  SMT=off                   Offline its unused SMT siblings: off, keep
  ASLR=off                  Address randomization for the command: off, keep

All remaining arguments are passed unchanged to the benchmark executable.
Benchmark options and BENCHMARK_* environment variables keep their usual meaning.
Relative paths in benchmark options are resolved against the build directory.
Build the lab first with build_lab.sh. Run without an outer sudo:
stable_run.py requests sudo only for CPU settings; the lab runs as your user.
Isolation requires a writable cgroup v2 mount with isolated partitions and
leaves at least one physical core for the system. Hardware interrupts and
per-CPU kernel threads can still run on the isolated CPU.
After SIGKILL or a failed cleanup, the next run first recovers the original
settings saved in /run/perf-ninja-stable-run.json. For recovery without a new
measurement, run stable_run.py --restore.

Examples:
  tools/misc/run_lab.sh memory_bound/loop_interchange_1
  tools/misc/run_lab.sh misc/warmup --benchmark_repetitions=9
  CPU=3 TURBO=keep CPU_FREQ_KHZ=keep tools/misc/run_lab.sh labs/misc/warmup
  TURBO=off CPU_FREQ_KHZ=keep tools/misc/run_lab.sh memory_bound/loop_interchange_1
  CPU_FREQ_SCOPE=selected tools/misc/run_lab.sh memory_bound/loop_interchange_1
  PLATFORM_PROFILE=keep tools/misc/run_lab.sh misc/warmup
  ISOLATE=off SMT=keep ASLR=keep tools/misc/run_lab.sh misc/warmup
  tools/misc/run_lab.sh misc/warmup --benchmark_out=baseline.json
  (cd labs/misc/warmup && ../../../tools/misc/run_lab.sh)
EOF
}

fail() {
  printf 'Error: %s\n' "$*" >&2
  exit 1
}

if [[ ${1:-} == --help || ${1:-} == -h ]]; then
  usage
  exit 0
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
repo_dir=$(cd -- "$script_dir/../.." && pwd -P)
lab_input=.
if [[ $# -gt 0 && ${1:-} != -* ]]; then
  lab_input=$1
  shift
fi
if [[ ${1:-} == -- ]]; then
  shift
fi

if [[ -d "$lab_input" ]]; then
  lab_dir=$lab_input
elif [[ -d "$repo_dir/labs/$lab_input" ]]; then
  lab_dir="$repo_dir/labs/$lab_input"
elif [[ -d "$repo_dir/$lab_input" ]]; then
  lab_dir="$repo_dir/$lab_input"
else
  fail "Lab directory not found: $lab_input"
fi
lab_dir=$(cd -- "$lab_dir" && pwd -P)

case "$lab_dir" in
  "$repo_dir"/labs/*/*) ;;
  *) fail "Choose a lab inside $repo_dir/labs" ;;
esac
[[ -f "$lab_dir/CMakeLists.txt" && -f "$lab_dir/bench.cpp" ]] ||
  fail "Not a lab directory: $lab_dir"

build_dir="$lab_dir/build"
lab_binary="$build_dir/lab"
if [[ ! -x "$lab_binary" && -x "$build_dir/Release/lab" ]]; then
  lab_binary="$build_dir/Release/lab"
fi
[[ -x "$lab_binary" ]] ||
  fail "Lab executable not found. Build it first: $script_dir/build_lab.sh $lab_dir"

cpu=${CPU:-2}
platform_profile=${PLATFORM_PROFILE:-performance}
turbo=${TURBO:-on}
cpu_freq_khz=${CPU_FREQ_KHZ:-3500000}
cpu_freq_scope=${CPU_FREQ_SCOPE:-all}
isolate=${ISOLATE:-on}
smt=${SMT:-off}
aslr=${ASLR:-off}
[[ $cpu =~ ^[0-9]+$ ]] || fail "CPU must be a non-negative integer"
case "$platform_profile" in
  performance|keep) ;;
  *) fail "PLATFORM_PROFILE must be performance or keep" ;;
esac
[[ $cpu_freq_khz == keep || $cpu_freq_khz =~ ^[1-9][0-9]*$ ]] ||
  fail "CPU_FREQ_KHZ must be a positive integer in kHz, or keep"
case "$cpu_freq_scope" in
  selected|all) ;;
  *) fail "CPU_FREQ_SCOPE must be selected or all" ;;
esac
case "$turbo" in
  off|on|keep) ;;
  *) fail "TURBO must be off, on, or keep" ;;
esac
case "$isolate" in
  on|off) ;;
  *) fail "ISOLATE must be on or off" ;;
esac
case "$smt" in
  off|keep) ;;
  *) fail "SMT must be off or keep" ;;
esac
case "$aslr" in
  off|keep) ;;
  *) fail "ASLR must be off or keep" ;;
esac
[[ -x "$script_dir/stable_run.py" ]] || fail "stable_run.py is not executable"
command -v python3 >/dev/null 2>&1 || fail "Python 3 is not installed"

printf 'Lab: %s\n' "${lab_dir#"$repo_dir/labs/"}" >&2
stable_options=(--cpu "$cpu" --governor "${GOVERNOR:-performance}" --turbo "$turbo"
  --smt "$smt" --aslr "$aslr" --platform-profile "$platform_profile")
if [[ $cpu_freq_khz != keep ]]; then
  stable_options+=(--frequency-khz "$cpu_freq_khz" --frequency-scope "$cpu_freq_scope")
fi
if [[ $isolate == on ]]; then
  stable_options+=(--isolate)
fi
cd -- "$build_dir"
exec "$script_dir/stable_run.py" "${stable_options[@]}" \
  -- "$lab_binary" "$@"
