#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc{0};
  for (auto value : blob)
    acc += value;

  auto high = acc >> 16;
  auto low = acc & 0xFFFFu;
  acc = low + high;

  high = acc >> 16;
  low = acc & 0xFFFFu;
  acc = low + high;
  return static_cast<uint16_t>(acc);
}
