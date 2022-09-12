#include "AllocateDoublesArray.hpp"
#include "GenerateMesh.hpp"
#include "solution.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>

static const auto seed = std::random_device{}();
static constexpr unsigned n_nodes_x = 100, n_nodes_y = 200,
                          n_nodes = n_nodes_x * n_nodes_y;

template <typename Allocator> auto solve(Allocator &&alloc) {
  const auto x_alloc = alloc(n_nodes);
  const auto y_alloc = alloc(n_nodes);
  const auto topology =
      generateMesh(n_nodes_x, n_nodes_y, x_alloc.get(), y_alloc.get(), seed);
  const double *x = x_alloc.get();
  const double *y = y_alloc.get();

  // Generate random left-hand side
  const auto lhs_alloc = alloc(2 * n_nodes);
  double *lhs = lhs_alloc.get();
  std::mt19937 prng{seed};
  auto dist = std::uniform_real_distribution<double>{0., 42.};
  std::generate_n(lhs, 2 * n_nodes, [&] { return dist(prng); });

  // Right-hand side
  auto rhs_alloc = alloc(2 * n_nodes);
  double *rhs = rhs_alloc.get();

  // Eval operator
  solution(topology, n_nodes, x, y, lhs, rhs);

  return rhs_alloc;
}

int main() {
  try {
    const auto sol_user =
        solve([](size_t size) { return allocateDoublesArray(size); });
    const auto sol_valid =
        solve([](size_t size) { return std::make_unique<double[]>(size); });
    const auto L2_error = std::sqrt(std::transform_reduce(
        sol_user.get(), sol_user.get() + 2 * n_nodes, sol_valid.get(), 0.,
        std::plus<>{}, [](double a, double b) { return (a - b) * (a - b); }));

    if (L2_error > 1e-9) {
      std::cerr
          << "Validation Failed. The error between the result of the "
             "evaluation "
             "of the original and user-supplied operators exceeded 1e-9\n";
      return EXIT_FAILURE;
    }

    std::cout << "Validation Successful\n";
    return EXIT_SUCCESS;

  } catch (const std::bad_alloc &) {
    std::cerr << "Validation failed. std::bad_alloc was thrown\n";
    return EXIT_FAILURE;
  } catch (const std::exception &e) {
    std::cerr << "Validation failed. The following exception was thrown:\n"
              << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Validation failed. Unknown exception was thrown\n";
    return EXIT_FAILURE;
  }
}
