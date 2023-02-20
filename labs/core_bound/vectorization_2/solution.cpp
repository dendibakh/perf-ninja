#include "solution.hpp"

#define SOLUTION

#ifdef SOLUTION
uint16_t checksum(const Blob &blob) {
  constexpr unsigned bs{64};
  std::array<uint16_t, bs> acc{};
  for (unsigned k{}; k < N; k += bs) {
    auto *p = &blob[k];
    for (unsigned m{}; m < bs; ++m) {
      auto value = p[m];
      acc[m] += value;
      acc[m] += acc[m] < value;
    }
  }
  uint16_t acc1{};
  for (auto value : acc) {
    acc1 += value;
    acc1 += acc1 < value; // add carry
  }
  return acc1;
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
