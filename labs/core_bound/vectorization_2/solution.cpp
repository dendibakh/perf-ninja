#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  while ((acc >> 16) > 0) {
    uint32_t lower = acc & ((1 << 16) - 1);
    uint32_t upper = (acc >> 16);
    acc = lower + upper;
  }
  return acc;
}
