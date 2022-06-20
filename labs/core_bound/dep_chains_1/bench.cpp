
#include "benchmark/benchmark.h"
#include "solution.hpp"
#include <memory>

static void bench1(benchmark::State &state) {
  // Init benchmark data
  auto arena1 = ArenaListAllocator{};
  auto l1 = getRandomList(arena1);
  auto arena2 = ArenaListAllocator{};
  auto l2 = getRandomList(arena2);

  // Run the benchmark
  for (auto _ : state) {
    auto output = solution(l1, l2);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
