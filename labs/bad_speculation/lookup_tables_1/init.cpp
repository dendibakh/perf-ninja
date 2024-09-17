#include "solution.hpp"
#include <limits>
#include <random>

void init(std::vector<int> &values) {
  std::random_device r;
  std::default_random_engine generator(r());
  // generate random integer in the closed interval [0,150]
  // the chance of selecting the default bucket is ~33%
  std::uniform_int_distribution<int> distribution(0, 150);
  for (int i = 0; i < NUM_VALUES; i++) {
    values.push_back(distribution(generator));
  }
}