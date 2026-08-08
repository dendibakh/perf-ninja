#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  uint32_t res = (acc & UINT16_MAX) + (acc >> 16);
  return (res & UINT16_MAX) + (res >> 16);
}
