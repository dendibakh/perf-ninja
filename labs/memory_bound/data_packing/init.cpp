// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include "solution.h"
#include <random>

S create_entry(int first_value, int second_value) {
  S entry;

  entry.i = first_value;
  entry.s = static_cast<short>(second_value);
  entry.l = static_cast<long long>(first_value * second_value);
  entry.d = static_cast<double>(first_value) / maxRandom;
  entry.b = first_value < second_value;

  return entry;
}

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(minRandom, maxRandom - 1);

  for (int i = 0; i < N; i++) {
    int random_int1 = distribution(generator);
    int random_int2 = distribution(generator);

    arr[i] = create_entry(random_int1, random_int2);
  }
}
