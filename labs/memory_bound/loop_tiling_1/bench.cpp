
#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench1(benchmark::State &state) {
  constexpr int N = 2000;

  MatrixOfDoubles in;
  MatrixOfDoubles out;
  in.resize(N, std::vector<double>(N, 0.0));
  out.resize(N, std::vector<double>(N, 0.0));

  // Init benchmark data
  initMatrix(in);

  // Run the benchmark
  for (auto _ : state) {
    auto output = solution(in, out);
    benchmark::DoNotOptimize(output);
  }
}

static void bench2(benchmark::State &state) {
  constexpr int N = 2000;

  const int TILE_SIZE = static_cast<int>(state.range(0));

  MatrixOfDoubles in;
  MatrixOfDoubles out;
  in.resize(N, std::vector<double>(N, 0.0));
  out.resize(N, std::vector<double>(N, 0.0));

  // Init benchmark data
  initMatrix(in);

  // Run the benchmark
  for (auto _ : state) {
    auto output = solution_fix(in, out, TILE_SIZE);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMillisecond);
// BENCHMARK(bench2)
//     ->Arg(10)
//     ->Arg(15) <-- BEST
//     ->Arg(20)
//     ->Arg(50)
//     ->Arg(100)
//     ->Arg(200)
//     ->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
