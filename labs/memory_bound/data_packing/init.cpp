
#include "solution.h"
#include <random>
#include <iostream>

S create_entry(int first_value, int second_value) {
  S entry;

  // std::cout << "sizeof S: " << sizeof(S) << "\n";

  entry.i = static_cast<char>(first_value);
  entry.s = static_cast<short>(second_value);
  entry.l = static_cast<int>(first_value * second_value);
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
