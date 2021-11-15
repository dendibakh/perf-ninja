#include "solution.hpp"
#include <limits>
#include <random>

void init(Blob &blob) {
  std::default_random_engine generator;
  std::uniform_int_distribution<uint16_t> distribution(
      0, std::numeric_limits<uint16_t>::max());

  for (std::size_t i = 0; i < N; i++) {
    blob[i] = distribution(generator);
  }
}
