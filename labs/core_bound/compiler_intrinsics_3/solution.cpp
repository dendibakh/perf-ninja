#include "solution.hpp"
#include <algorithm>
#include "immintrin.h"

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input)
{
  std::uint64_t x = 0, y = 0, z = 0;
  size_t sz = input.size(), i = 0;
  const auto *base_ptr = input.data();
  auto x_indices = _mm256_set_epi32(21, 18, 15, 12, 9, 6, 3, 0);
  auto y_indices = _mm256_set_epi32(22, 19, 16, 13, 10, 7, 4, 1);
  auto z_indices = _mm256_set_epi32(23, 20, 17, 14, 11, 8, 5, 2);
  auto x_sum = _mm256_setzero_si256(), y_sum = _mm256_setzero_si256(), z_sum = _mm256_setzero_si256();
  ;
  for (; i + 8 <= sz; i += 8)
  {
    const auto *ptr = base_ptr + i;
    auto x_values = _mm256_i32gather_epi32(reinterpret_cast<const int *>(ptr), x_indices, 4);
    auto y_values = _mm256_i32gather_epi32(reinterpret_cast<const int *>(ptr), y_indices, 4);
    auto z_values = _mm256_i32gather_epi32(reinterpret_cast<const int *>(ptr), z_indices, 4);
    auto x0 = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(x_values, 1));
    auto x1 = _mm256_cvtepu32_epi64(_mm256_castsi256_si128(x_values));
    auto y0 = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(y_values, 1));
    auto y1 = _mm256_cvtepu32_epi64(_mm256_castsi256_si128(y_values));
    auto z0 = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(z_values, 1));
    auto z1 = _mm256_cvtepu32_epi64(_mm256_castsi256_si128(z_values));
    x_sum = _mm256_add_epi64(x_sum, _mm256_add_epi64(x0, x1));
    y_sum = _mm256_add_epi64(y_sum, _mm256_add_epi64(y0, y1));
    z_sum = _mm256_add_epi64(z_sum, _mm256_add_epi64(z0, z1));
  }
  x += _mm256_extract_epi64(x_sum, 0) + _mm256_extract_epi64(x_sum, 1) + _mm256_extract_epi64(x_sum, 2) + _mm256_extract_epi64(x_sum, 3);
  y += _mm256_extract_epi64(y_sum, 0) + _mm256_extract_epi64(y_sum, 1) + _mm256_extract_epi64(y_sum, 2) + _mm256_extract_epi64(y_sum, 3);
  z += _mm256_extract_epi64(z_sum, 0) + _mm256_extract_epi64(z_sum, 1) + _mm256_extract_epi64(z_sum, 2) + _mm256_extract_epi64(z_sum, 3);
  for (; i < sz; ++i)
  {
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