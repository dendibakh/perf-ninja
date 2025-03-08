
#include "benchmark/benchmark.h"
#include "solution.hpp"
#include <cstdint>
#include <random>
#include <vector>

static std::string inputsDirName;

static void bench1(benchmark::State &state) {

  std::vector<Position<std::uint32_t>> input;
  std::default_random_engine eng{};
  std::uniform_int_distribution<std::uint32_t> distr;
  for (std::size_t i = 0; i < (64 << 8); ++i) {

    input.push_back(Position<std::uint32_t>{distr(eng), distr(eng), distr(eng)});
  }
  // Run the benchmark
  for (auto _: state) {
    auto output = solution(input);
    benchmark::DoNotOptimize(input);
    benchmark::DoNotOptimize(output);
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond);

int main(int argc, char **argv) {
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}
