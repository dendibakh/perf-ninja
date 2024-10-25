#include "solution.hpp"
#include <limits>
#include <iostream>

#define SOLUTION

#ifdef SOLUTION
uint16_t checksum(const Blob &blob) {
  uint32_t acc{0};
  for (auto value : blob) {
    acc += value;
  }
  constexpr uint32_t full_mask = ~(uint32_t{0});
  constexpr uint32_t low_mask = full_mask >> 16;

  // overflow 1
  uint32_t low = acc & low_mask;
  uint32_t high = acc >> 16;
  acc = high + low;

  // overflow 2
  low = acc & low_mask;
  high = acc >> 16;
  acc = high + low;

  return int16_t(acc);
}

#else
uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  for (auto value : blob) {
    acc += value;
    acc += acc < value; // add carry
  }
  return acc;
}

#endif
