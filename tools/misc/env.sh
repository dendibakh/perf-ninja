#!/usr/bin/env bash
# Source from any directory: source /path/to/perf-ninja/tools/misc/env.sh
# Exposes build_lab.sh, run_lab.sh, perf_lab.sh and stable_run.py in this shell.

if [[ ${BASH_SOURCE[0]} == "$0" ]]; then
  printf 'Source this script: source %q\n' "${BASH_SOURCE[0]}" >&2
  exit 1
fi

_perf_ninja_setup_path() {
  local script_path=${BASH_SOURCE[0]}
  local script_dir=.
  if [[ $script_path == */* ]]; then
    script_dir=${script_path%/*}
  fi
  script_dir=$(cd -- "$script_dir" && pwd -P) || return 1

  case ":${PATH-}:" in
    *":$script_dir:"*) ;;
    *) PATH="$script_dir${PATH:+:$PATH}" ;;
  esac
  export PATH
}

if _perf_ninja_setup_path; then
  unset -f _perf_ninja_setup_path
else
  unset -f _perf_ninja_setup_path
  return 1
fi
