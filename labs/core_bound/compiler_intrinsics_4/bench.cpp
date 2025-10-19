#include "data_paths.h"
#include "picture.h"
#include "solution.h"

#include "benchmark/benchmark.h"

#include <iostream>
#include <fstream>

namespace {

constexpr auto kImageWidth = 1280;
constexpr auto kImageHeight = 720;

bool image_saved = false;

void bench1(benchmark::State& state) {
  std::vector<short> data;
  for (auto _ : state) {
    data = mandelbrot(kImageWidth, kImageHeight);
  }
  if (!image_saved) {
    image_saved = true;
    std::cout << "Saving image to " << benchmark_image_path << std::endl;
    std::ofstream(benchmark_image_path, std::ios::binary) << generate_ppm_image(data, kImageWidth, kImageHeight);
  }
}

}  // namespace

BENCHMARK(bench1)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
