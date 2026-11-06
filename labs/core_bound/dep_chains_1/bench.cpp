
#include "benchmark/benchmark.h"
#include "solution.hpp"
#include <memory>

template <size_t group_size>
static void bench1(benchmark::State &state) {
  // Init benchmark data
  auto arena1 = ArenaListAllocator{};
  auto l1 = getRandomList(arena1);
  auto arena2 = ArenaListAllocator{};
  auto l2 = getRandomList(arena2);

  // Run the benchmark
  for (auto _ : state) {
    // auto output = solution<4>(l1, l2);
    auto output = solution<group_size>(l1, l2);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK_TEMPLATE(bench1, 2)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bench1, 4)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bench1, 8)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bench1, 16)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bench1, 32)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bench1, 64)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
