#!/usr/bin/env bash
# Build, validate, and benchmark every perf-ninja lab. Tolerant of failures
# (a broken lab is recorded, not aborted on). Outputs per-lab artifacts in
# each lab's build/ directory and a top-level summary CSV.

set -u
ROOT="$(cd "$(dirname "$0")" && pwd)"
SUMMARY="$ROOT/run_all_labs.summary.csv"
echo "lab,build,validate,bench_mean_ns,bench_cv_pct,note" >"$SUMMARY"

CXX=clang++-17
CC=clang-17
JOBS="$(nproc)"
MIN_TIME=1s
REPS=3

cd "$ROOT"
mapfile -t LABS < <(find labs -name CMakeLists.txt -not -path '*/build/*' | sort)
echo "Found ${#LABS[@]} labs."

for lab_cmake in "${LABS[@]}"; do
  lab=$(dirname "$lab_cmake")
  name="${lab#labs/}"
  echo "===== $name ====="
  pushd "$lab" >/dev/null

  rm -rf build
  mkdir build && cd build

  build_status=fail
  validate_status=skip
  bench_mean=
  bench_cv=
  note=

  if cmake -DCMAKE_BUILD_TYPE=Release \
           -DCMAKE_C_COMPILER="$CC" \
           -DCMAKE_CXX_COMPILER="$CXX" .. >cmake.log 2>&1 \
     && cmake --build . --config Release --parallel "$JOBS" >>cmake.log 2>&1; then
    build_status=ok
  else
    note="build_error"
    tail -3 cmake.log >cmake.tail
  fi

  if [[ "$build_status" == ok && -x ./lab ]]; then
    if cmake --build . --target validateLab >validate.log 2>&1; then
      if grep -q "Validation Successful" validate.log; then
        validate_status=ok
      else
        validate_status=fail
        note="${note:+$note;}validate_unexpected"
      fi
    else
      validate_status=fail
      note="${note:+$note;}validate_error"
    fi

    if timeout 120 ./lab --benchmark_min_time="$MIN_TIME" \
            --benchmark_repetitions="$REPS" \
            --benchmark_report_aggregates_only=true \
            --benchmark_format=json \
            --benchmark_out=bench.json >bench.txt 2>&1; then
      # Extract mean wall time and CV from JSON (single-benchmark labs only).
      bench_mean=$(python3 -c '
import json,sys
d=json.load(open("bench.json"))
for b in d["benchmarks"]:
  if b.get("aggregate_name")=="mean":
    print(int(b["real_time"])); sys.exit(0)
' 2>/dev/null)
      bench_cv=$(python3 -c '
import json,sys
d=json.load(open("bench.json"))
mean=cv=None
for b in d["benchmarks"]:
  if b.get("aggregate_name")=="mean": mean=b["real_time"]
  if b.get("aggregate_name")=="stddev": stddev=b["real_time"];
if "mean" in dir() and mean: pass
mean=None;stddev=None
for b in d["benchmarks"]:
  if b.get("aggregate_name")=="mean": mean=b["real_time"]
  if b.get("aggregate_name")=="stddev": stddev=b["real_time"]
if mean and stddev is not None and mean>0:
  print(f"{stddev/mean*100:.2f}")
' 2>/dev/null)
    else
      note="${note:+$note;}bench_timeout_or_error"
    fi
  fi

  echo "$name,$build_status,$validate_status,${bench_mean:-},${bench_cv:-},${note:-}" >>"$SUMMARY"
  popd >/dev/null
done

echo
echo "===== summary ====="
cat "$SUMMARY"
