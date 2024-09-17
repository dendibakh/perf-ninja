
#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench1(benchmark::State &state) {
  std::vector<int> values;
  values.reserve(NUM_VALUES);
  init(values);

  // Run the benchmark
  for (auto _ : state) {
    auto output = histogram(values);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
