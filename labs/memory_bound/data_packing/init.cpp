
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
  std::uniform_int_distribution<unsigned short> distribution(minRandom, maxRandom - 1);

  for (int i = 0; i < N; i++) {
    unsigned short random_int1 = distribution(generator);
    unsigned short random_int2 = distribution(generator);

    // arr[i] = create_entry(random_int1, random_int2);
    arr[i].i = random_int1;
    arr[i].s = random_int2;
    arr[i].l = (random_int1 * random_int2);
    arr[i].d = static_cast<float>(random_int1) / maxRandom;
    arr[i].b = random_int1 < random_int2;
  }
}
