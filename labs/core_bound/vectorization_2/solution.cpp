#include "solution.hpp"

#define SOLUTION
#ifdef SOLUTION
uint16_t checksum(const Blob &blob) {
  uint64_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }

  constexpr uint64_t mask = (1 << 16) - 1;

  acc = (acc & mask) + (acc >> 16);
  acc = (acc & mask) + (acc >> 16);
  return (uint16_t)acc;
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
