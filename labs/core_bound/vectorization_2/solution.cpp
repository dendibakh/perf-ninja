#include "solution.hpp"

using acc_t = std::array<uint16_t, 16>;

uint16_t checksum(const Blob &blob) {
  acc_t acc = {0};
  for (std::size_t i = 0; i < N; i += 16) {
    for (std::size_t j = 0; j < 16; j++) {
      acc[j] += blob[i + j];
      acc[j] += acc[j] < blob[i + j];  // add carry
    }
  }
  for (std::size_t i = 1; i < 16; i++) {
    acc[0] += acc[i];
    acc[0] += acc[0] < acc[i];  // add carry
  }
  return acc[0];
}
