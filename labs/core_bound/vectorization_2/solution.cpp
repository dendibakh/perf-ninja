#include "solution.hpp"

#define SOLUTION

#ifdef SOLUTION
uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }

  // sum up two halves of uint32
  auto high = acc >> 16;
  auto low = acc & 0xFFFFu;
  acc = low + high;

  // account for potential overflow
  high = acc >> 16;
  low = acc & 0xFFFFu;
  acc = low + high;

  return static_cast<uint16_t>(acc);
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