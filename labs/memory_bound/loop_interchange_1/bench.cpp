
#include "benchmark/benchmark.h"
#include "solution.h"
#include <memory>

static void bench1(benchmark::State &state) {
  std::unique_ptr<Matrix> a(new Matrix());
  init(*a);

  std::unique_ptr<Matrix> b(new Matrix());
  zero(*b);

  for (auto _ : state) {
    *b = power(*a, 2021);
    benchmark::DoNotOptimize(b);
  }
}

// Register the function as a benchmark
BENCHMARK(bench1)->Iterations(10);

// Run the benchmark
BENCHMARK_MAIN();
