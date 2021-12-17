
#include "benchmark/benchmark.h"
#include "ao.h"

int ao_bench();

Sphere spheres[3];
Plane  plane;

static void bench1(benchmark::State &state) {
  // Run the benchmark
  for (auto _ : state) {
    auto output = ao_bench();
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in milliseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
