#include "solution.hpp"
#include <algorithm>

#if defined(SOLUTION) && defined(__AVX2__)

#include <immintrin.h>

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {

  __m256i acc = _mm256_setzero_si256();

  constexpr std::size_t step = 1;
  const std::size_t size = input.size();
  std::size_t i = 0;
  const Position<std::uint32_t>* const data = input.data();

  for (; i < size; i += step)
  {
    const Position<std::uint32_t>* const dataToLoad = data + i;
    const std::int64_t data64Mem[4] = {static_cast<std::int64_t>(dataToLoad->x), static_cast<std::int64_t>(dataToLoad->y), static_cast<std::int64_t>(dataToLoad->z), 0};
    const __m256i data64 = _mm256_loadu_si256((__m256i *)data64Mem);
    acc = _mm256_add_epi64 (acc, data64);
  }
  
  const std::size_t divisor = std::max<std::uint64_t>(1, input.size());
  std::int64_t data64[4];
  _mm256_storeu_si256((__m256i *)(data64), acc);


  return {
          static_cast<std::uint32_t>(data64[0] / divisor),
          static_cast<std::uint32_t>(data64[1] / divisor),
          static_cast<std::uint32_t>(data64[2] / divisor),
  };
}

#else

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  for (auto pos: input) {
    x += pos.x;
    y += pos.y;
    z += pos.z;
  }

  return {
          static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
  };
}

#endif