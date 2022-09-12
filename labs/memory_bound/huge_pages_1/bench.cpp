#include "AllocateDoublesArray.hpp"
#include "GenerateMesh.hpp"
#include "benchmark/benchmark.h"
#include "solution.hpp"

static void bench1(benchmark::State &state) {
  try {
    // Mesh
    constexpr unsigned n_nodes_x = 800, n_nodes_y = 20000,
                       n_nodes = n_nodes_x * n_nodes_y;
    constexpr unsigned seed = 0xaf173e8au;
    const auto x_alloc = allocateDoublesArray(n_nodes);
    const auto y_alloc = allocateDoublesArray(n_nodes);
    const auto topology =
        generateMesh(n_nodes_x, n_nodes_y, x_alloc.get(), y_alloc.get(), seed);
    const double *x = x_alloc.get();
    const double *y = y_alloc.get();

    // Generate random left-hand side
    const auto lhs_alloc = allocateDoublesArray(2 * n_nodes);
    double *lhs = lhs_alloc.get();
    std::mt19937 prng{std::random_device{}()};
    auto dist = std::uniform_real_distribution<double>{0., 42.};
    std::generate_n(lhs, 2 * n_nodes, [&] { return dist(prng); });

    // Right-hand side
    const auto rhs_alloc = allocateDoublesArray(2 * n_nodes);
    double *rhs = rhs_alloc.get();

    // Run the benchmark
    for (auto _ : state) {
      solution(topology, n_nodes, x, y, lhs, rhs);
      benchmark::DoNotOptimize(rhs);
      benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * topology.size() * 4 *
                            sizeof(double));

    // Cleanup via RAII
  } catch (const std::bad_alloc &) {
    std::cerr << "std::bad_alloc was thrown\n";
    throw;
  } catch (const std::exception &e) {
    std::cerr << "The following exception was thrown:\n" << e.what() << '\n';
    throw;
  } catch (...) {
    std::cerr << "Unknown exception was thrown\n";
    throw;
  }
}

// Register the function as a benchmark and measure time in microseconds
BENCHMARK(bench1)->Name("Apply matrix-free operator")->Unit(benchmark::kSecond);

// Run the benchmark
BENCHMARK_MAIN();
