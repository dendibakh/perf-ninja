#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: perf_lab.sh [perf options] [category/lab | lab-directory] [--] [benchmark options...]

Profile an already built lab with perf record through stable_run.py.
CPU governors, Turbo mode, SMT siblings and CPU isolation are restored afterwards.
The platform profile and RAPL power limits are also saved and restored.
The script can be called from any working directory.
If no lab is specified, use the current directory.
Run the lab from its build directory, as the CMake benchmark target does.

Perf options (before benchmark arguments, before or after the lab path):
  -e, --event EVENTS        Event expression; repeat for multiple events
      --events EVENTS      Alias for --event; comma lists/groups pass unchanged
  -F, --freq HZ            Target samples per second (positive integer)
      --frequency HZ       Alias for --freq
  --                       Pass all remaining arguments to lab unchanged

Options override EVENT and FREQ. Without -e, EVENT defaults to cycles:u;
without -F, FREQ defaults to 199. Long options also accept --option=value.

Defaults:
  CPU=2                     Logical CPU for the lab
  PERF_CPU=3                Logical CPU for perf record
  GOVERNOR=performance      CPUFreq governor for all policies, or keep
  PLATFORM_PROFILE=performance  Temporary platform profile: performance, keep
  TURBO=on                  Turbo/boost mode: off, on, keep
  CPU_FREQ_KHZ=3500000      Equal min/max frequency limits in kHz, or keep
  CPU_FREQ_SCOPE=all        Frequency policies: all, or selected (CPU and PERF_CPU)
  ISOLATE=on                Reserve the physical cores with cgroup v2: on, off
  SMT=off                   Offline their unused SMT siblings: off, keep
  ASLR=off                  Address randomization for the command: off, keep
  FREQ=199                  Target samples per second
  EVENT=cycles:u            Sample user-space CPU cycles
  CALL_GRAPH=dwarf          User-space stack unwinding method
  OUTPUT=<lab>/build/perf.data
  BENCHMARK_MIN_TIME=5s      Minimum benchmark time; CLI options take precedence

Relative OUTPUT paths are resolved against the caller's working directory.
All remaining arguments are passed to the benchmark executable.
Build the lab first with build_lab.sh; perf access must already be configured.
Run without an outer sudo: stable_run.py requests sudo only for CPU settings;
perf and the lab run as your user.
Choose CPU and PERF_CPU on different physical cores to reduce interference.
They may be set to the same CPU if needed.
Isolation requires a writable cgroup v2 mount with isolated partitions and
leaves at least one physical core for the system. Hardware interrupts and
per-CPU kernel threads can still run on the isolated CPUs.
After SIGKILL or a failed cleanup, the next run first recovers the original
settings saved in /run/perf-ninja-stable-run.json. For recovery without a new
measurement, run stable_run.py --restore.

Examples:
  tools/misc/perf_lab.sh memory_bound/loop_interchange_1
  tools/misc/perf_lab.sh labs/misc/warmup --benchmark_min_time=10s
  tools/misc/perf_lab.sh misc/warmup -e cycles:u -e instructions:u -F 99
  tools/misc/perf_lab.sh --events 'cycles:u,cache-misses:u' --freq 199 misc/warmup
  CPU=2 PERF_CPU=3 FREQ=99 tools/misc/perf_lab.sh misc/warmup
  TURBO=keep CPU_FREQ_KHZ=keep tools/misc/perf_lab.sh misc/warmup
  TURBO=off CPU_FREQ_KHZ=keep tools/misc/perf_lab.sh misc/warmup
  CPU_FREQ_SCOPE=selected tools/misc/perf_lab.sh memory_bound/loop_interchange_1
  PLATFORM_PROFILE=keep tools/misc/perf_lab.sh misc/warmup
  ISOLATE=off SMT=keep ASLR=keep tools/misc/perf_lab.sh misc/warmup
  OUTPUT=/tmp/warmup.data tools/misc/perf_lab.sh misc/warmup
  (cd labs/misc/warmup && ../../../tools/misc/perf_lab.sh)
EOF
}

fail() {
  printf 'Error: %s\n' "$*" >&2
  exit 1
}

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
repo_dir=$(cd -- "$script_dir/../.." && pwd -P)
lab_input=.
lab_selected=false
freq=${FREQ:-199}
events=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      usage
      exit 0
      ;;
    -e|--event|--events)
      [[ $# -gt 1 && -n ${2:-} && ${2:-} != -* ]] || fail "Missing event expression for $1"
      events+=("$2")
      shift 2
      ;;
    --event=*|--events=*)
      events+=("${1#*=}")
      shift
      ;;
    -e?*)
      events+=("${1#-e}")
      shift
      ;;
    -F|--freq|--frequency)
      [[ $# -gt 1 && -n ${2:-} && ${2:-} != -* ]] || fail "Missing sample frequency for $1"
      freq=$2
      shift 2
      ;;
    --freq=*|--frequency=*)
      freq=${1#*=}
      shift
      ;;
    -F?*)
      freq=${1#-F}
      shift
      ;;
    --)
      shift
      break
      ;;
    -*)
      break  # The first benchmark argument ends wrapper option parsing.
      ;;
    *)
      if [[ $lab_selected == true ]]; then
        break
      fi
      lab_input=$1
      lab_selected=true
      shift
      ;;
  esac
done
[[ $freq =~ ^[1-9][0-9]*$ ]] || fail "FREQ / --freq must be a positive integer"
if [[ ${#events[@]} -eq 0 ]]; then
  events=("${EVENT:-cycles:u}")
fi
perf_events=()
for event in "${events[@]}"; do
  [[ -n $event && $event != -* ]] || fail "Event expression must not be empty or start with '-'"
  perf_events+=(-e "$event")
done

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
perf_cpu=${PERF_CPU:-3}
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
[[ $perf_cpu =~ ^[0-9]+$ ]] || fail "PERF_CPU must be a non-negative integer"
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
command -v perf >/dev/null 2>&1 || fail "perf is not installed"
[[ -x "$script_dir/stable_run.py" ]] || fail "stable_run.py is not executable"
command -v python3 >/dev/null 2>&1 || fail "Python 3 is not installed"
command -v taskset >/dev/null 2>&1 || fail "taskset is not installed"

# stable_run checks both CPUs after recovering any stale cpuset/SMT settings.

output=${OUTPUT:-"$build_dir/perf.data"}
if [[ $output != /* ]]; then
  output="$PWD/$output"
fi
[[ -d "$(dirname -- "$output")" ]] || fail "Output directory does not exist: $output"

printf 'Lab: %s\nCPUs: lab=%s perf=%s\nPerf data: %s\nView with: perf report -i %q\n' \
  "${lab_dir#"$repo_dir/labs/"}" "$cpu" "$perf_cpu" "$output" "$output"

export BENCHMARK_MIN_TIME=${BENCHMARK_MIN_TIME:-5s}
stable_options=(--cpu "$perf_cpu" --cpus "$cpu,$perf_cpu"
  --governor "${GOVERNOR:-performance}" --turbo "$turbo" --smt "$smt" --aslr "$aslr"
  --platform-profile "$platform_profile")
if [[ $cpu_freq_khz != keep ]]; then
  stable_options+=(--frequency-khz "$cpu_freq_khz" --frequency-scope "$cpu_freq_scope")
fi
if [[ $isolate == on ]]; then
  stable_options+=(--isolate)
fi
lab_command=(taskset -c "$cpu")
if [[ $aslr == off ]]; then
  command -v setarch >/dev/null 2>&1 || fail "setarch is not installed"
  # perf's file capabilities can reset an inherited ADDR_NO_RANDOMIZE flag.
  # Set it again in the workload, after the privileged perf executable.
  lab_command+=(setarch --addr-no-randomize)
fi
lab_command+=("$lab_binary" "$@")
cd -- "$build_dir"
exec "$script_dir/stable_run.py" "${stable_options[@]}" \
  -- perf record \
  "${perf_events[@]}" -F "$freq" \
  --call-graph "${CALL_GRAPH:-dwarf}" -o "$output" \
  -- "${lab_command[@]}"
