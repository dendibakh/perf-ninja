
#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench1(benchmark::State &state) {
  std::vector<int> values;
  values.reserve(NUM_VALUES);
#ifdef SOLUTION
  std::vector<std::size_t> table;
  init(values, table);
#else
  init(values);
#endif

  // Run the benchmark
#ifdef SOLUTION
  for (auto _ : state) {
    auto output = histogram(values, table);
    benchmark::DoNotOptimize(output);
  }
#else
  for (auto _ : state) {
    auto output = histogram(values);
    benchmark::DoNotOptimize(output);
  }
#endif
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
