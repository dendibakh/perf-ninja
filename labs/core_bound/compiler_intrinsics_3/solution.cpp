#include "solution.hpp"
#include <algorithm>

#if defined(SOLUTION) && defined(__AVX2__)

#include <immintrin.h>

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input)
{

  __m256i acc_xyzx = _mm256_setzero_si256();
  __m256i acc_yzxy = _mm256_setzero_si256();
  __m256i acc_zxyz = _mm256_setzero_si256();

  constexpr std::size_t step = 4;
  const std::size_t size = input.size();
  std::size_t i = 0;
  static_assert(sizeof(Position<std::uint32_t>) == 3 *sizeof(std::uint32_t));
  const std::uint32_t *const data = reinterpret_cast<const std::uint32_t*>(input.data());

  for (; i < size % step; ++i)
  {
    const std::uint32_t *const dataToLoad = data + i * 3 + 0 * 4;
    // X Y Z Dummy
    const std::int64_t data64Mem[4] = {static_cast<std::int64_t>(dataToLoad[0]), static_cast<std::int64_t>(dataToLoad[1]), static_cast<std::int64_t>(dataToLoad[2]), 0};
    const __m256i data64 = _mm256_loadu_si256((__m256i *)data64Mem);
    acc_xyzx = _mm256_add_epi64(acc_xyzx, data64);
  }

  for (; i < size; i += step)
  {
    {
      const std::uint32_t *const dataToLoad = data + i * 3 + 0 * 4;
      // X Y Z X
      // Test _mm256_cvtepu32_epi64 
      const std::int64_t data64Mem[4] = {static_cast<std::int64_t>(dataToLoad[0]), static_cast<std::int64_t>(dataToLoad[1]), static_cast<std::int64_t>(dataToLoad[2]), static_cast<std::int64_t>(dataToLoad[3])};
      const __m256i data64 = _mm256_loadu_si256((__m256i *)data64Mem);
      acc_xyzx = _mm256_add_epi64(acc_xyzx, data64);
    }

    {
      const std::uint32_t *const dataToLoad = data + i * 3 + 1 * 4;
      // Y Z X Y
      const std::int64_t data64Mem[4] = {static_cast<std::int64_t>(dataToLoad[0]), static_cast<std::int64_t>(dataToLoad[1]), static_cast<std::int64_t>(dataToLoad[2]), static_cast<std::int64_t>(dataToLoad[3])};
      const __m256i data64 = _mm256_loadu_si256((__m256i *)data64Mem);
      acc_yzxy = _mm256_add_epi64(acc_yzxy, data64);
    }

    {
      const std::uint32_t *const dataToLoad = data + i * 3 + 2 * 4;
      // Z X Y Z
      const std::int64_t data64Mem[4] = {static_cast<std::int64_t>(dataToLoad[0]), static_cast<std::int64_t>(dataToLoad[1]), static_cast<std::int64_t>(dataToLoad[2]), static_cast<std::int64_t>(dataToLoad[3])};
      const __m256i data64 = _mm256_loadu_si256((__m256i *)data64Mem);
      acc_zxyz = _mm256_add_epi64(acc_zxyz, data64);
    }
  }

  std::int64_t acc_mem[4 * step];
  _mm256_storeu_si256((__m256i *)(acc_mem + 0 * 4), acc_xyzx);
  _mm256_storeu_si256((__m256i *)(acc_mem + 1 * 4), acc_yzxy);
  _mm256_storeu_si256((__m256i *)(acc_mem + 2 * 4), acc_zxyz);

  for (size_t i = 1; i < 4; ++i)
  {
    acc_mem[0] += acc_mem[i * 3 + 0];
    acc_mem[1] += acc_mem[i * 3 + 1];
    acc_mem[2] += acc_mem[i * 3 + 2];
  }
  



  const std::size_t divisor = std::max<std::uint64_t>(1, input.size());

  return {
      static_cast<std::uint32_t>(acc_mem[0] / divisor),
      static_cast<std::uint32_t>(acc_mem[1] / divisor),
      static_cast<std::uint32_t>(acc_mem[2] / divisor),
  };
}

#else

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input)
{
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  for (auto pos : input)
  {
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