#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: build_lab.sh [category/lab | lab-directory] [CMake options...]

Configure and build a lab in Release mode with debug information (-g),
using Clang and Ninja.
Build output goes in the lab's build subdirectory.
The script can be called from any working directory.
If no lab is specified, use the current directory.

Examples:
  tools/misc/build_lab.sh memory_bound/loop_interchange_1
  tools/misc/build_lab.sh labs/misc/warmup -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  CC=gcc CXX=g++ tools/misc/build_lab.sh misc/warmup
  JOBS=4 tools/misc/build_lab.sh misc/warmup
  (cd labs/misc/warmup && ../../../tools/misc/build_lab.sh)

Set CC/CXX and CMAKE_GENERATOR (or pass -G) to override the defaults.
Existing builds keep their cached compiler and generator.
JOBS controls build parallelism (default: 8).
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

jobs=${JOBS:-8}
[[ $jobs =~ ^[1-9][0-9]*$ ]] || fail "JOBS must be a positive integer"
command -v cmake >/dev/null 2>&1 || fail "CMake is not installed"

build_dir="$lab_dir/build"
CC=${CC:-clang} CXX=${CXX:-clang++} CMAKE_GENERATOR=${CMAKE_GENERATOR:-Ninja} \
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-g -DCMAKE_CXX_FLAGS=-g \
    "$@" -S "$lab_dir" -B "$build_dir"
cmake --build "$build_dir" --config Release --parallel "$jobs"

printf '\nBuilt lab: %s\nBuild directory: %s\n' "${lab_dir#"$repo_dir/labs/"}" "$build_dir"
