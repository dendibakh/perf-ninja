#include "solution.h"
#include <cmath>
#include <limits>
#include <random>

constexpr int N = 40000;

void init(InputVector &data) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0, 255);

  data.reserve(N);
  for (int i = 0; i < N; i++) {
    uint8_t value = static_cast<uint8_t>(distribution(generator));
    data.emplace_back(value);
  }
}

void zero(OutputVector &data, std::size_t size) {
  data.clear();
  data.resize(size);
}
