
#include "solution.h"
#include <random>

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(minRandom, maxRandom - 1);

  for (int i = 0; i < N; i++) {
    int random_int1 = distribution(generator);
    int random_int2 = distribution(generator);

    // casting to char is fine, since we are allowed to assume that maxRandom
    // never exceeds 100
    arr[i].i = static_cast<char>(random_int1);
    arr[i].s = static_cast<char>(random_int2);
  }
}
