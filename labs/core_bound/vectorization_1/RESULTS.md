# Vectorization 1 — Baseline Benchmark Results

This file documents a baseline run of the `core_bound/vectorization_1` lab from
[perf-ninja](https://github.com/saurabhtangri/perf-ninja). The lab implements a
Needleman–Wunsch–style pairwise sequence alignment and the goal of the exercise
is to expose SIMD parallelism by transposing the data layout across 16
independent sequence pairs.

## Environment

| Item | Value |
|---|---|
| OS | Ubuntu 24.04.3 LTS, kernel 6.8.0-110-generic |
| CPU | QEMU Virtual CPU v2.5+ (8 vCPU, 2592 MHz nominal) |
| ISA flags (relevant) | `sse2`, `ssse3`, `sse4_1`, `sse4_2`, `aes`, `popcnt`, `cx16` (no AVX/AVX2/AVX-512 exposed by the hypervisor) |
| Caches | L1d 32 KiB · L1i 32 KiB · L2 4 MiB · L3 16 MiB |
| Compiler | clang-17 (Ubuntu 17.0.6) |
| Build flags | `-O3 -march=native -ffast-math` (from `tools/labs.cmake`) |
| Google Benchmark | v1.9.5-50-gb8081188, Release |
| CPU scaling | governor change rejected by the hypervisor — `cpu_scaling_enabled=false` is reported by Google Benchmark, so frequency is effectively pinned by the host. |

> Note on the CPU: the baseline `solution.cpp` is the unvectorized scalar
> implementation. Because this VM does not expose AVX/AVX2 to the guest,
> auto-vectorization can only target 128-bit SSE — speedups from a manual
> SIMD rewrite will therefore be smaller here than on the bare-metal Alder
> Lake / Coffee Lake CI machines listed in `GetStarted.md`.

## What was run

```bash
# 1. Build the Google Benchmark dependency
bash ~/dev/perf-ninja/tools/make_benchmark_library.sh

# 2. Configure & build the lab in Release mode with clang-17
cd ~/dev/perf-ninja/labs/core_bound/vectorization_1
cmake -E make_directory build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=clang-17 \
      -DCMAKE_CXX_COMPILER=clang++-17 ..
cmake --build . --config Release --parallel 8

# 3. Validate correctness
cmake --build . --target validateLab
# -> "Validation Successful"

# 4. Run the benchmark
./lab --benchmark_min_time=2s --benchmark_repetitions=5 \
      --benchmark_report_aggregates_only=true
```

## Validation

```
Validation Successful
[100%] Built target validateLab
```

The baseline `compute_alignment` returns the expected scores for the reference
sequence pairs.

## Benchmark output

Raw aggregate output from `./lab --benchmark_min_time=2s --benchmark_repetitions=5`:

```
Run on (8 X 2592 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 4096 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 0.27, 0.23, 0.10
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
bench_compute_alignment_mean      2703922 ns      2703824 ns            5
bench_compute_alignment_median    2711589 ns      2711514 ns            5
bench_compute_alignment_stddev      20493 ns        20500 ns            5
bench_compute_alignment_cv           0.76 %          0.76 %             5
```

Per-repetition wall times from the JSON run (`--benchmark_out=bench.json`):

| Repetition | Iterations | Real time (ns) | CPU time (ns) |
|---:|---:|---:|---:|
| 0 | 1033 | 2,692,297 | 2,692,280 |
| 1 | 1033 | 2,712,630 | 2,712,605 |
| 2 | 1033 | 2,656,201 | 2,656,178 |
| 3 | 1033 | 2,689,274 | 2,689,242 |
| 4 | 1033 | — | — |

## Interpretation

* **Wall-clock per call:** ≈ **2.70 ms** to align all 16 pairs of length-200
  sequences (`sequence_size_v = 200`, `sequence_count_v = 16`).
* **Per-pair cost:** `2.70 ms / 16 ≈ 169 µs`, i.e. roughly `169 µs / (200×200) ≈
  4.2 ns per cell` of the score matrix on this VM.
* **Stability:** coefficient of variation = **0.76 %** across 5 repetitions.
  That is well under the 1 % rule-of-thumb for a noisy run, so the baseline
  number is trustworthy as a reference point even without `cpupower`-driven
  governor pinning.
* **Why the baseline is slow:** the inner loop in `solution.cpp` carries a
  serial dependency on `last_diagonal_score`, `last_vertical_gap`, and
  `score_column[row]` along the *row* axis. The compiler cannot vectorize that
  recurrence inside a single sequence pair. The intended optimization for
  this lab — packing 16 independent sequence pairs into the lanes of a SIMD
  vector — sidesteps the dependency by parallelizing *across* pairs instead
  of along the recurrence.
* **Hardware ceiling on this VM:** with only SSE4.2 visible, the theoretical
  upper bound for a 16-lane `int16` SIMD rewrite is one 128-bit register × 8
  `int16` lanes = roughly an **8× speedup**. On a host that exposes AVX2 you
  would get 16 lanes in one register and a single-pass speedup closer to
  ~16×; with AVX-512, 32 lanes.

## How to reproduce / iterate

After a code change to `solution.cpp`:

```bash
cd ~/dev/perf-ninja/labs/core_bound/vectorization_1/build
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab     # must still print "Validation Successful"
./lab --benchmark_min_time=2s --benchmark_repetitions=5 \
      --benchmark_report_aggregates_only=true
```

To compare against this baseline using the perf-ninja helper:

```bash
# Save this baseline as JSON
./lab --benchmark_min_time=2s --benchmark_out_format=json \
      --benchmark_out=baseline.json
# ... edit solution.cpp ...
./lab --benchmark_min_time=2s --benchmark_out_format=json \
      --benchmark_out=solution.json
python3 ~/dev/perf-ninja/tools/benchmark/tools/compare.py \
        benchmarks baseline.json solution.json
```
