
#include "solution.h"
#include <limits>
#include <random>

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<std::uint32_t> distribution(
      std::numeric_limits<std::uint32_t>::min(),
      std::numeric_limits<std::uint32_t>::max());

  for (std::size_t i = 0; i < N; i++) {
    arr[i].first = distribution(generator);
    arr[i].second = distribution(generator);
  }
}
