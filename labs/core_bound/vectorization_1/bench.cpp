#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench_compute_alignment(benchmark::State &state) {
  auto [sequences1, sequences2] = init();

  for (auto _ : state) {
    auto res = compute_alignment(sequences1, sequences2);
    benchmark::DoNotOptimize(res);
  }
}

// Register the functions as a benchmark
BENCHMARK(bench_compute_alignment);

// Run the benchmark
BENCHMARK_MAIN();
