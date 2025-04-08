#include "benchmark/benchmark.h"
#include "solution.hpp"

static void BM_compute_alignment(benchmark::State& state) {
  auto [sequences1, sequences2] = init();

  for (auto _ : state) {
    auto res = compute_alignment(sequences1, sequences2);
    benchmark::DoNotOptimize(res);
  }
}

// Register the functions as a benchmark
BENCHMARK(BM_compute_alignment)->Iterations(10);

// Run the benchmark
BENCHMARK_MAIN();