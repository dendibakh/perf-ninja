
#include "benchmark/benchmark.h"
#include "solution.h"
#include <cstdio>
#include <iostream>

// Variables for passed arguments
static std::string input, output;

static void bench1(benchmark::State &state) {
  // Delete an output file for the case of possible crash
  std::remove(output.data());

  Grayscale image;
  if (image.load(input, kMaxImageDimension)) {
    decltype(Grayscale::data) result(new uint8_t[image.size]);
    decltype(Grayscale::data) temp(new uint8_t[image.size]);
    if (result && temp) {
      for (auto _ : state) {
        blur(result.get(), image.data.get(), image.width, image.height,
             temp.get());
        benchmark::DoNotOptimize(image);
      }

      std::swap(image.data, result);
      image.save(output);
      return;
    }
  }

  state.SkipWithError("An IO problem");
}

// Register the function as a benchmark
BENCHMARK(bench1);

// Run the benchmark
int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 2;
  if (argc < 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: input.pgm output.pgm [--name=value...]" << std::endl;
    return 1;
  }
  input = argv[1];
  output = argv[2];

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc - mandatoryArgumentsCount,
                                               argv + mandatoryArgumentsCount))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}
