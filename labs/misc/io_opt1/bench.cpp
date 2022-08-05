
#include "benchmark/benchmark.h"
#include "solution.hpp"

#include <iostream>

constexpr const char *file_names[] = {small_data, medium_data, large_data};

static void bench1(benchmark::State &state) {
  const char *file = file_names[state.range(0)];

  // Run the benchmark
  for (auto _ : state) {
    auto output = solution(file);
    benchmark::DoNotOptimize(output);
  }
}

BENCHMARK(bench1)->Unit(benchmark::kMicrosecond)->Arg(0)->Name("Small file");
BENCHMARK(bench1)->Unit(benchmark::kMillisecond)->Arg(1)->Name("Medium file");
BENCHMARK(bench1)->Unit(benchmark::kSecond)->Arg(2)->Name("Large file");

BENCHMARK_MAIN();
