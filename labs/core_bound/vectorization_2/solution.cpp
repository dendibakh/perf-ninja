#include "solution.hpp"

#define SOLUTION
#ifdef SOLUTION
uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }

  constexpr uint32_t mask = (1 << 16) - 1;

  // N = 64 * 1024 = 2^16
  // Carry should fit in the upper 16 bits
  acc = (acc & mask) + (acc >> 16);
  // 2^16 carries might overflow one last time after added
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
