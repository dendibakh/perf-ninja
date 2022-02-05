#include "solution.hpp"
#include <limits>
#include <random>

void init(std::vector<int> &values) {
  std::default_random_engine generator;
  // generate random integer in the closed interval [0,99]
  std::uniform_int_distribution<int> distribution(0, 99);
  for (int i = 0; i < NUM_VALUES; i++) {
    values.push_back(distribution(generator));
  }
}