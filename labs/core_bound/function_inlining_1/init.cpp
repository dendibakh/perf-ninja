
#include "solution.h"
#include <random>

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<uint32_t> distribution(
      0, 9000u);

  for (size_t i = 0; i < N; i++) {
    uint32_t random_int1 = distribution(generator);
    uint32_t random_int2 = distribution(generator);

    arr[i].key1 = random_int1;
    arr[i].key2 = random_int2;
  }
}
