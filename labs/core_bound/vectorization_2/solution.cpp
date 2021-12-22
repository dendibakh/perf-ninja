#include "solution.hpp"
#include <limits>

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  uint16_t res = acc;
  acc >>= std::numeric_limits<uint16_t>::digits;
  res += acc;
  return res;
}
