
#include "solution.h"
#include <random>

S create_entry(const uint8_t first_value, const uint8_t second_value) {
  S entry;

  // We can pack the data in 4 bytes
  // the first * second value is always less than 2^14
  // The first and second value are always less than 2^7 (128)
  // and boolean is 1 bit
  // 14 + 7 + 7 + 1 = 29 bits
  unsigned int packed = 0;
  // Pack the first value in the first 7 bits
  packed |= first_value;
  // Pack the second value in the 8th to 14th bits
  packed |= second_value << 7;
  // Pack the first * second value in the 15th to 28th bits
  packed |= (first_value * second_value) << 14;
  // Pack the boolean in the 29th bit
  packed |= (first_value < second_value) << 29;

  entry.maskedData = packed;
  entry.d = static_cast<float>(first_value) / maxRandom;
  return entry;
}

void init(std::array<S, N> &arr) {
  std::default_random_engine generator;
  std::uniform_int_distribution<uint8_t> distribution(minRandom, maxRandom - 1);

  for (int i = 0; i < N; i++) {
    const uint8_t random_int1 = distribution(generator);
    const uint8_t random_int2 = distribution(generator);

    arr[i] = create_entry(random_int1, random_int2);
  }
}
