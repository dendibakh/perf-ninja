#include "benchmark/benchmark.h"
#include "solution.h"

static void bench1(benchmark::State& state) {
    InstanceArray arr;
    generateObjects(arr);

    for (auto _: state) {
        std::size_t data = 0;
        invoke(arr, data);
        benchmark::DoNotOptimize(data);
    }
}

// Register the function as a benchmark
BENCHMARK(bench1)->Unit(benchmark::kMicrosecond); // ->Iterations(10000)

// Run the benchmark
BENCHMARK_MAIN();
