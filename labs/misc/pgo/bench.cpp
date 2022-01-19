
#include "benchmark/benchmark.h"

extern "C" { int runLuaBenchmark (int argc, char **argv); }

static void bench1(benchmark::State &state) {
  const char* run_bench_tests[] = { "lua", "../run_bench_tests.lua", "bench", 0 };
  for (auto _ : state) {
    int status = runLuaBenchmark(3/*argc*/, (char**)run_bench_tests);
    benchmark::DoNotOptimize(status);
  }
}

// Register the function as a benchmark and measure time in milliseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();