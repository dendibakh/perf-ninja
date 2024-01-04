
#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench1(benchmark::State &state) {
  // Init benchmark data
  auto particles = initParticles();

  // Run the benchmark
  for (auto _ : state) {
    randomParticleMotion<XorShift32>(particles, STEPS);
    benchmark::DoNotOptimize(particles);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
