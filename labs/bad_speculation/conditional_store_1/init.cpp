
#include "solution.h"
#include <limits>
#include <random>

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<std::int32_t> distribution(0, std::numeric_limits<std::int32_t>::max());

  for (std::size_t i = 0; i < N; i++) {
    arr[i].first = distribution(generator);
    arr[i].second = distribution(generator);
  }
}
