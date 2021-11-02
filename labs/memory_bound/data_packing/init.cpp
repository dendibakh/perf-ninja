
#include "solution.h"
#include <random>

S create_entry(int first_value, int second_value) {
  S entry;
  // first_value and second_value range : [0;99]

  entry.i = static_cast<unsigned char>(first_value);                 // 128 = 2^7 -> 7 bits for i
  entry.s = static_cast<unsigned char>(second_value);                // same for s
  entry.l = static_cast<unsigned short>(entry.i * entry.s);          // 99 * 99 < 10000 -> 2^14 -> 14 bits for l
  entry.d = static_cast<float>(first_value) / maxRandom;
  entry.b = first_value < second_value;                              // 1 bit

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
