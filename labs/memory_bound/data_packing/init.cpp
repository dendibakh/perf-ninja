#include "solution.h"
#include <random>

S create_entry(int first_value, int second_value) {
  S entry;

  entry.i = static_cast<unsigned char>(first_value);
  entry.s = static_cast<unsigned char>(second_value);
  entry.b = first_value < second_value;
  entry.l = static_cast<unsigned short>(first_value * second_value);
  entry.d = static_cast<float>(first_value) / maxRandom;

  return entry;
}

void init(std::vector<S> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(minRandom, maxRandom - 1);

  for (int i = 0; i < N; i++) {
    int random_int1 = distribution(generator);
    int random_int2 = distribution(generator);

    arr[i] = create_entry(random_int1, random_int2);
  }
}
