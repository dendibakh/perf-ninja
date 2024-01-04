#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  uint16_t temp = 0;
  for (auto value : blob) {
    acc += value;
    temp += (acc < value); // add carry
  }
  return acc+temp;
}
