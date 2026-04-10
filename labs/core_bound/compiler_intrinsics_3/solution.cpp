#include "solution.hpp"
#include <algorithm>
#include "immintrin.h"

// First approach : gather instructions.
Position<std::uint32_t> solution1(std::vector<Position<std::uint32_t>> const &input)
{
  std::uint64_t x = 0, y = 0, z = 0;
  size_t sz = input.size(), i = 0;
  const auto *base_ptr = input.data();
  auto x_indices = _mm256_set_epi32(21, 18, 15, 12, 9, 6, 3, 0);
  auto y_indices = _mm256_set_epi32(22, 19, 16, 13, 10, 7, 4, 1);
  auto z_indices = _mm256_set_epi32(23, 20, 17, 14, 11, 8, 5, 2);
  auto x_sum = _mm256_setzero_si256(), y_sum = _mm256_setzero_si256(), z_sum = _mm256_setzero_si256();
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

// Second approach : loads + permutes + blends (manual shuffle).
Position<std::uint32_t> solution2(std::vector<Position<std::uint32_t>> const &input)
{
  std::uint64_t x = 0, y = 0, z = 0;
  size_t sz = input.size(), i = 0;
  const auto *base_ptr = input.data();
  auto x_perm0_idx = _mm256_set_epi32(0, 0, 0, 0, 0, 6, 3, 0);
  auto x_perm1_idx = _mm256_set_epi32(0, 0, 7, 4, 1, 0, 0, 0);
  auto x_perm2_idx = _mm256_set_epi32(5, 2, 0, 0, 0, 0, 0, 0);
  auto y_perm0_idx = _mm256_set_epi32(0, 0, 0, 0, 0, 7, 4, 1);
  auto y_perm1_idx = _mm256_set_epi32(0, 0, 0, 5, 2, 0, 0, 0);
  auto y_perm2_idx = _mm256_set_epi32(6, 3, 0, 0, 0, 0, 0, 0);
  auto z_perm0_idx = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 5, 2);
  auto z_perm1_idx = _mm256_set_epi32(0, 0, 0, 6, 3, 0, 0, 0);
  auto z_perm2_idx = _mm256_set_epi32(7, 4, 1, 0, 0, 0, 0, 0);
  auto x_sum = _mm256_setzero_si256(), y_sum = _mm256_setzero_si256(), z_sum = _mm256_setzero_si256();
  for (; i + 8 <= sz; i += 8)
  {
    const auto *ptr = reinterpret_cast<const __m256i *>(base_ptr + i);
    auto pos0 = _mm256_loadu_si256(ptr);
    auto pos1 = _mm256_loadu_si256(ptr + 1);
    auto pos2 = _mm256_loadu_si256(ptr + 2);
    auto x_perm0 = _mm256_permutevar8x32_epi32(pos0, x_perm0_idx);
    auto x_perm1 = _mm256_permutevar8x32_epi32(pos1, x_perm1_idx);
    auto x_perm2 = _mm256_permutevar8x32_epi32(pos2, x_perm2_idx);
    auto x_values = _mm256_blend_epi32(x_perm0, x_perm1, 0b00111000);
    x_values = _mm256_blend_epi32(x_values, x_perm2, 0b11000000);
    auto y_perm0 = _mm256_permutevar8x32_epi32(pos0, y_perm0_idx);
    auto y_perm1 = _mm256_permutevar8x32_epi32(pos1, y_perm1_idx);
    auto y_perm2 = _mm256_permutevar8x32_epi32(pos2, y_perm2_idx);
    auto y_values = _mm256_blend_epi32(y_perm0, y_perm1, 0b00011000);
    y_values = _mm256_blend_epi32(y_values, y_perm2, 0b11100000);
    auto z_perm0 = _mm256_permutevar8x32_epi32(pos0, z_perm0_idx);
    auto z_perm1 = _mm256_permutevar8x32_epi32(pos1, z_perm1_idx);
    auto z_perm2 = _mm256_permutevar8x32_epi32(pos2, z_perm2_idx);
    auto z_values = _mm256_blend_epi32(z_perm0, z_perm1, 0b00011100);
    z_values = _mm256_blend_epi32(z_values, z_perm2, 0b11100000);
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

// Third approach : golden branch solution.
Position<std::uint32_t> solution3(std::vector<Position<std::uint32_t>> const &input)
{
  std::uint64_t x = 0, y = 0, z = 0;
  size_t sz = input.size(), i = 0;
  const auto *input_ptr = reinterpret_cast<const __m128i *>(input.data());
  auto acc_XYZX = _mm256_setzero_si256(), acc_YZXY = _mm256_setzero_si256(), acc_ZXYZ = _mm256_setzero_si256();
  constexpr int UNROLL = 4;
  for (; i + UNROLL - 1 < input.size(); i += UNROLL)
  {
    auto XMM_XYZX = _mm_loadu_si128(input_ptr + 0); // load 128 bits
    auto XMM_YZXY = _mm_loadu_si128(input_ptr + 1);
    auto XMM_ZXYZ = _mm_loadu_si128(input_ptr + 2);
    auto YMM_XYZX = _mm256_cvtepu32_epi64(XMM_XYZX); // 32 bit -> 64 bit (vpmovsxdq)
    auto YMM_YZXY = _mm256_cvtepu32_epi64(XMM_YZXY);
    auto YMM_ZXYZ = _mm256_cvtepu32_epi64(XMM_ZXYZ);
    acc_XYZX = _mm256_add_epi64(acc_XYZX, YMM_XYZX); // accumulation
    acc_YZXY = _mm256_add_epi64(acc_YZXY, YMM_YZXY);
    acc_ZXYZ = _mm256_add_epi64(acc_ZXYZ, YMM_ZXYZ);
    input_ptr += 3;
  }

  // reduce acc_XYZX
  x += _mm256_extract_epi64(acc_XYZX, 0);
  y += _mm256_extract_epi64(acc_XYZX, 1);
  z += _mm256_extract_epi64(acc_XYZX, 2);
  x += _mm256_extract_epi64(acc_XYZX, 3);

  // reduce acc_YZXY
  y += _mm256_extract_epi64(acc_YZXY, 0);
  z += _mm256_extract_epi64(acc_YZXY, 1);
  x += _mm256_extract_epi64(acc_YZXY, 2);
  y += _mm256_extract_epi64(acc_YZXY, 3);

  // reduce acc_ZXYZ
  z += _mm256_extract_epi64(acc_ZXYZ, 0);
  x += _mm256_extract_epi64(acc_ZXYZ, 1);
  y += _mm256_extract_epi64(acc_ZXYZ, 2);
  z += _mm256_extract_epi64(acc_ZXYZ, 3);

  for (; i < input.size(); ++i)
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

// FASTEST, builds on the third approach.
// Fourth approach : No 64-bit extensions, just 32-bit additions + overflow counting.
Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input)
{
  std::uint64_t x = 0, y = 0, z = 0;
  size_t sz = input.size(), i = 0;
  const auto *base_ptr = input.data();

  // 32-bit accumulators
  auto acc0 = _mm256_setzero_si256(), acc1 = _mm256_setzero_si256(), acc2 = _mm256_setzero_si256();

  // Overflow counters (high 32-bits)
  auto ovf0 = _mm256_setzero_si256(), ovf1 = _mm256_setzero_si256(), ovf2 = _mm256_setzero_si256();

  // Mask to flip the sign bit for faking unsigned comparison
  auto sign_mask = _mm256_set1_epi32(0x80000000);

  // Process 8 structs per iteration
  for (; i + 8 <= sz; i += 8)
  {
    const auto *ptr = reinterpret_cast<const __m256i *>(base_ptr + i);

    auto val0 = _mm256_loadu_si256(ptr);
    auto val1 = _mm256_loadu_si256(ptr + 1);
    auto val2 = _mm256_loadu_si256(ptr + 2);

    acc0 = _mm256_add_epi32(acc0, val0);
    acc1 = _mm256_add_epi32(acc1, val1);
    acc2 = _mm256_add_epi32(acc2, val2);

    // Unsigned Comparison: (val > sum) implies overflow
    auto cmp0 = _mm256_cmpgt_epi32(_mm256_xor_si256(val0, sign_mask), _mm256_xor_si256(acc0, sign_mask));
    auto cmp1 = _mm256_cmpgt_epi32(_mm256_xor_si256(val1, sign_mask), _mm256_xor_si256(acc1, sign_mask));
    auto cmp2 = _mm256_cmpgt_epi32(_mm256_xor_si256(val2, sign_mask), _mm256_xor_si256(acc2, sign_mask));

    // Accumulate overflows. (cmp is 0xFFFFFFFF (-1) if true, 0 if false. Sub (-1) means Add 1)
    ovf0 = _mm256_sub_epi32(ovf0, cmp0);
    ovf1 = _mm256_sub_epi32(ovf1, cmp1);
    ovf2 = _mm256_sub_epi32(ovf2, cmp2);
  }

  // Store everything to local arrays so we can easily combine them into 64-bit ints
  // This runs exactly once, so it has virtually 0 performance impact.
  uint32_t a0[8], o0[8], a1[8], o1[8], a2[8], o2[8];
  _mm256_storeu_si256((__m256i *)a0, acc0);
  _mm256_storeu_si256((__m256i *)o0, ovf0);
  _mm256_storeu_si256((__m256i *)a1, acc1);
  _mm256_storeu_si256((__m256i *)o1, ovf1);
  _mm256_storeu_si256((__m256i *)a2, acc2);
  _mm256_storeu_si256((__m256i *)o2, ovf2);

  auto merge = [](uint32_t a, uint32_t o)
  { return (static_cast<uint64_t>(o) << 32) | a; };

  // acc0 layout: X, Y, Z, X, Y, Z, X, Y
  x += merge(a0[0], o0[0]) + merge(a0[3], o0[3]) + merge(a0[6], o0[6]);
  y += merge(a0[1], o0[1]) + merge(a0[4], o0[4]) + merge(a0[7], o0[7]);
  z += merge(a0[2], o0[2]) + merge(a0[5], o0[5]);

  // acc1 layout: Z, X, Y, Z, X, Y, Z, X
  z += merge(a1[0], o1[0]) + merge(a1[3], o1[3]) + merge(a1[6], o1[6]);
  x += merge(a1[1], o1[1]) + merge(a1[4], o1[4]) + merge(a1[7], o1[7]);
  y += merge(a1[2], o1[2]) + merge(a1[5], o1[5]);

  // acc2 layout: Y, Z, X, Y, Z, X, Y, Z
  y += merge(a2[0], o2[0]) + merge(a2[3], o2[3]) + merge(a2[6], o2[6]);
  z += merge(a2[1], o2[1]) + merge(a2[4], o2[4]) + merge(a2[7], o2[7]);
  x += merge(a2[2], o2[2]) + merge(a2[5], o2[5]);

  for (; i < sz; ++i)
  {
    x += input[i].x;
    y += input[i].y;
    z += input[i].z;
  }

  return {
      static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, sz)),
      static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, sz)),
      static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, sz)),
  };
}
