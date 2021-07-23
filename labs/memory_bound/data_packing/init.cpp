
#include "solution.h"
#include <random>

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(minRandom, maxRandom - 1);

  for (int i = 0; i < N; i++) {
    int random_int1 = distribution(generator);
    int random_int2 = distribution(generator);

    arr[i].i = random_int1;
    arr[i].s = static_cast<short>(random_int2);
    arr[i].l = static_cast<long long>(random_int1) * random_int2;
    arr[i].d = static_cast<double>(random_int1) / maxRandom;
    arr[i].b = random_int1 < random_int2;
  }
}
