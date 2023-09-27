
#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench1(benchmark::State &state) {
  // Init benchmark data
  std::vector<Life::Grid> grids;
  for (int i = 0; i < NumberOfGrids; i++)
    grids.emplace_back(initRandom());

  // Run the benchmark
  for (auto _ : state) {
    auto output = solution(grids);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
