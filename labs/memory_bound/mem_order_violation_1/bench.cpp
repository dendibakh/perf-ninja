
#include "benchmark/benchmark.h"
#include "solution.h"
#include "DataPaths.h"

constexpr const char *file_names[] = {bird, coins, pepper, pixabay};

static void bench1(benchmark::State &state) {
  const char *input = file_names[state.range(0)];
  GrayscaleImage image;
  if (!image.load(input, kMaxImageDimension)) {
    state.SkipWithError("An IO problem");
    return;
  }

  std::string output = state.name() + "-binary.pgm";
  // Delete an output file
  std::remove(output.data());

  // Only benchmark the histogram part
  std::array<uint32_t, 256> hist;
  for (auto _ : state) {
    hist = computeHistogram(image);
    benchmark::DoNotOptimize(hist);
  }
  
  // Proceed with the rest of the algorithm
  auto totalPixels = image.height * image.width;
  int threshold = calcOtsuThreshold(hist, totalPixels);
  // Apply Otsu's thresholding
  for (int i = 0; i < totalPixels; ++i)
    image.data[i] = (image.data[i] >= threshold) ? 255 : 0;
  // save the output
  image.save(output); 
}

// Register the bench1 function as a benchmark
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond)->Arg(0)->Name("bird");
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond)->Arg(1)->Name("coins");
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond)->Arg(2)->Name("pepper");
BENCHMARK(bench1)->Unit(benchmark::kMillisecond)->Arg(3)->Name("pixabay");

BENCHMARK_MAIN();