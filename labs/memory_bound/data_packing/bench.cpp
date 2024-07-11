// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include "benchmark/benchmark.h"
#include "solution.h"

static void bench1(benchmark::State &state) {
  std::array<S, N> arr;
  init(arr);

  for (auto _ : state) {
    solution(arr);
    benchmark::DoNotOptimize(arr);
  }
}

// Register the function as a benchmark
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
