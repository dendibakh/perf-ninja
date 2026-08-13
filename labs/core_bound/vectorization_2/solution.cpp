#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  return (acc & ((1 << 16) - 1)) + (acc >> 16) + 1;
}
