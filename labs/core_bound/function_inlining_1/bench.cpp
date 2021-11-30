
#include "benchmark/benchmark.h"
#include "solution.h"

static void bench1(benchmark::State &state) {
  std::array<S, N> arr;
  init(arr);

  for (auto _ : state) {
    auto copy = arr;
    solution(copy);
    benchmark::DoNotOptimize(copy);
  }
}

// Register the function as a benchmark
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
