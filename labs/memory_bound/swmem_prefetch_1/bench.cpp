
#include "benchmark/benchmark.h"
#include "solution.hpp"
#include <memory>

static void bench1(benchmark::State &state) {
  // Init benchmark data
  auto hash_map = std::make_unique<hash_map_t>(HASH_MAP_SIZE);
  std::vector<int> lookups;
  lookups.reserve(NUMBER_OF_LOOKUPS);
  init(hash_map.get(), lookups);

  // Run the benchmark
  for (auto _ : state) {
    auto output = solution(hash_map.get(), lookups);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
