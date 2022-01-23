#include "benchmark/benchmark.h"
#include "solution.hpp"

#include <numeric>
#include <thread>

static void bench1(benchmark::State &state) {
  const auto size = 1024 * 1024;

  std::vector<uint32_t> data;
  data.reserve(size);

  for (int i = 0; i < size; i++) {
    data.push_back(i);
  }

  // Use thread count from 1 to <number of HW threads>
  size_t max_threads = std::thread::hardware_concurrency();
  std::vector<int> threads(max_threads);
  std::iota(threads.begin(), threads.end(), 1);

  for (auto _ : state) {
    for (auto thread_count : threads) {
      auto result = solution(data, thread_count);
      benchmark::DoNotOptimize(result);
    }
  }
}

// Register the function as a benchmark
BENCHMARK(bench1)->UseRealTime()->Unit(
    benchmark::kMillisecond); // ->Iterations(100)

// Run the benchmark
BENCHMARK_MAIN();
