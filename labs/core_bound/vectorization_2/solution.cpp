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
  constexpr uint32_t div = UINT16_MAX + 1;
  while (acc > UINT16_MAX) {
    acc = acc % div + acc / div;
  }
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
