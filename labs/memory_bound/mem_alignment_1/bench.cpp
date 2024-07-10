
#include "benchmark/benchmark.h"
#include "solution.h"
#include <memory>

template <class ...Args>
static void bench1(benchmark::State &state, Args&&... args) {
  auto args_tuple = std::make_tuple(std::move(args)...);
  size_t N = std::get<0>(args_tuple);
  const int K = n_columns(N);

  Matrix a1; a1.resize(N * K);  initRandom(a1, N, K);
  Matrix b1; b1.resize(N * K);  initRandom(b1, N, K);
  Matrix c1; c1.resize(N * K);  initZero  (c1, N, K);

  for (auto _ : state) {
    if (N < 100) {
      // for small matrices we can use a very simple GEMM implementation
      interchanged_matmul(a1.data(), b1.data(), c1.data(), N, K);
    } else {
      // for large matrices we use a blocked version
      blocked_matmul(a1.data(), b1.data(), c1.data(), N, K);
    }
    benchmark::DoNotOptimize(c1);
  }
}

// To make CI testing fast, we disabled most of the test cases below.
// However, we encourage you to benchmark many different matrix sizes.

// On macOS, it seems that the OS always properly aligns 
// matrices with sizes of powers of two, e.g. 64, 128, etc.
// That's why we don't see speedups on macOS for such matrices.

BENCHMARK_CAPTURE(bench1, _63,  63)->Unit(benchmark::kMicrosecond);
#ifndef ON_MACOS
BENCHMARK_CAPTURE(bench1, _64,  64)->Unit(benchmark::kMicrosecond);
#endif
BENCHMARK_CAPTURE(bench1, _65,  65)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _71,  71)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _72,  72)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _73,  73)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _79,  79)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _80,  80)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _81,  81)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _127, 127)->Unit(benchmark::kMicrosecond);
#ifndef ON_MACOS
BENCHMARK_CAPTURE(bench1, _128, 128)->Unit(benchmark::kMicrosecond);
#endif
//BENCHMARK_CAPTURE(bench1, _129, 129)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _255, 255)->Unit(benchmark::kMicrosecond);
#ifndef ON_MACOS
BENCHMARK_CAPTURE(bench1, _256, 256)->Unit(benchmark::kMicrosecond);
#endif
//BENCHMARK_CAPTURE(bench1, _257, 257)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(bench1, _511, 511)->Unit(benchmark::kMicrosecond);
#ifndef ON_MACOS
BENCHMARK_CAPTURE(bench1, _512, 512)->Unit(benchmark::kMicrosecond);
#endif
BENCHMARK_CAPTURE(bench1, _513, 513)->Unit(benchmark::kMicrosecond);
//BENCHMARK_CAPTURE(bench1, _1023, 1023)->Unit(benchmark::kMicrosecond);
#ifndef ON_MACOS
BENCHMARK_CAPTURE(bench1, _1024, 1024)->Unit(benchmark::kMicrosecond);
#endif
//BENCHMARK_CAPTURE(bench1, _1025, 1025)->Unit(benchmark::kMicrosecond);

// Run the benchmark
BENCHMARK_MAIN();
