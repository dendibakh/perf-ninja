
#include "benchmark/benchmark.h"
#include "solution.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

static std::string inputsDirName;

static void bench1(benchmark::State &state) {
  const std::vector<std::string> inputs = {
      "LoopVectorize.cpp",       // a large C++ file from the LLVM compiler.
      "MarkTwain-TomSawyer.txt", // a typical text file with long lines.
      /*"counter-example.txt" // input where sequential solution is faster*/
  };

  std::vector<std::string> inputContents;

  for (auto &input : inputs) {
    std::filesystem::path inputsDirPath = inputsDirName;
    inputsDirPath.append(input);
    std::ifstream inFile{inputsDirPath.string()};
    inputContents.emplace_back(std::istreambuf_iterator<char>(inFile),
                               std::istreambuf_iterator<char>());
  }

  // Run the benchmark
  for (auto _ : state) {
    for (auto &inputContent : inputContents) {
      auto output = solution(inputContent);
      benchmark::DoNotOptimize(output);
    }
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond);

int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 1;
  if (argc < 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: lab path/to/inputs [gbench args]" << std::endl;
    return 1;
  }
  inputsDirName = argv[1];

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc - mandatoryArgumentsCount,
                                               argv + mandatoryArgumentsCount))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}
