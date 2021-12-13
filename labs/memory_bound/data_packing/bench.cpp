
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
