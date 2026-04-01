#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob)
    acc += value;
  acc = (acc >> 16) + (acc & 0xFFFF);
  uint16_t ans = (acc >> 16) + (acc & 0xFFFF);
  return ans;
}
