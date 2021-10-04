
#include "benchmark/benchmark.h"
#include "solution.h"
#include <limits>

static void bench1(benchmark::State &state) {
  std::array<S, N> arr;
  init(arr);

  std::array<S, N> temp;
  constexpr auto lower = (std::numeric_limits<std::uint32_t>::max() / 4) + 1;
  constexpr auto upper =
      (std::numeric_limits<std::uint32_t>::max() / 2) + lower;

  for (auto _ : state) {
    select(temp, arr, lower, upper);
    benchmark::DoNotOptimize(temp);
  }
}

// Register the function as a benchmark
BENCHMARK(bench1)->Iterations(10000)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
