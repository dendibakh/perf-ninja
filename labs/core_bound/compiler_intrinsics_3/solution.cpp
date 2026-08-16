#include "solution.hpp"
#include <algorithm>
#include <immintrin.h>

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;
  __m256i mm = _mm256_setzero_si256();

  int sz = input.size();
  for (int i = 0; i < sz; i++) {
    __m256i fst = _mm256_loadu_si256((__m256i*)&input[i]);
    mm = _mm256_add_epi64(mm, fst);
  }

  alignas(64) uint64_t buf[4];
  _mm256_storeu_si256((__m256i*)&buf, mm);

  return {
          static_cast<std::uint32_t>(buf[0] / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(buf[1] / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(buf[2] / std::max<std::uint64_t>(1, input.size())),
  };
}