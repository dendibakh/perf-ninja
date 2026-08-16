// #pragma GCC target("avx512f")
#include "solution.hpp"
#include <algorithm>
#include <immintrin.h>

__attribute__((target("avx512f")))
Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;
  __m512i mm = _mm512_setzero_si512();

  int sz = input.size();
  int i = 0;
  for (; i + 2 < sz; i += 2) {
    __m512i fst = _mm512_loadu_si512((__m512i*)&input[i]);
    mm = _mm512_add_epi64(mm, fst);
  }

  alignas(64) uint64_t buf[8];
  _mm512_storeu_si512((__m512i*)&buf, mm);

  x = buf[0] + buf[4];
  y = buf[1] + buf[5];
  z = buf[2] + buf[6];

  for (; i < sz; ++i) {
    x += input[i].x;
    y += input[i].y;
    z += input[i].z;
  }

  return {
          static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
  };
}