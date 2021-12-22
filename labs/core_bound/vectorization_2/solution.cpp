#include "solution.hpp"
#include <limits>

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }

  while(auto overflow = (acc >> std::numeric_limits<uint16_t>::digits)){
    acc &= std::numeric_limits<uint16_t>::max();
    acc += overflow;
  }
  
  return acc;
}
