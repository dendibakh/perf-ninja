#include "solution.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <numeric>
#include <x86intrin.h>

namespace {
#if defined(__AVX512F__)
  using vec_t = __m512i;
  constexpr auto& vec_setzero = _mm512_setzero_si512;
  constexpr auto& vec_load = _mm512_loadu_si512;
  constexpr auto& vec_store = _mm512_storeu_si512;
  constexpr auto& vec_add = _mm512_add_epi32;
  const auto vec_one = _mm512_set1_epi32(1);
#elif defined(__AVX2__)
  using vec_t = __m256i;
  constexpr auto& vec_setzero = _mm256_setzero_si256;
  constexpr auto& vec_load = _mm256_loadu_si256;
  constexpr auto& vec_store = _mm256_storeu_si256;
  constexpr auto& vec_add = _mm256_add_epi32;
  constexpr auto& vec_cmpeq = _mm256_cmpeq_epi32;
  constexpr auto& vec_max = _mm256_max_epu32;
#else
  using vec_t = __m128i;
  constexpr auto& vec_setzero = _mm_setzero_si128;
  constexpr auto& vec_load = _mm_loadu_si128;
  constexpr auto& vec_store = _mm_storeu_si128;
  constexpr auto& vec_add = _mm_add_epi32;
  constexpr auto& vec_cmpeq = _mm_cmpeq_epi32;
  constexpr auto& vec_max = _mm_max_epu32;
#endif
}  // namespace

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  constexpr int comp_cnt = 3;
  constexpr int vec_sz = sizeof(vec_t) / sizeof(uint32_t);
  constexpr int batch_sz = std::lcm(vec_sz, comp_cnt) / vec_sz;
  const uint32_t* data = reinterpret_cast<const uint32_t*>(input.data());
  const int data_sz = comp_cnt * input.size();

  std::array<vec_t, batch_sz> accum;
  std::array<vec_t, batch_sz> carry;
  for (int i = 0; i < batch_sz; ++i) {
    accum[i] = vec_setzero();
    carry[i] = vec_setzero();
  }

  int idx = 0;
  while (idx + batch_sz * vec_sz <= data_sz) {
    for (int i = 0; i < batch_sz; ++i, idx += vec_sz) {
      const auto d = vec_load((const vec_t*) (data + idx));
      accum[i] = vec_add(accum[i], d);
#if defined(__AVX512F__)
      carry[i] = _mm512_mask_add_epi32(carry[i], _mm512_cmplt_epu32_mask(accum[i], d), carry[i], vec_one);
#else
      carry[i] = vec_add(carry[i], vec_cmpeq(accum[i], vec_max(accum[i], d)));
#endif
    }
  }

#if defined(__AVX512F__)
  constexpr int carry_fix_up = 0;
#else
  const int carry_fix_up = data_sz / (batch_sz * vec_sz);
#endif

  std::array<uint64_t, comp_cnt> sum{};
  for (int i = 0, j = 0; i < batch_sz; ++i) {
    std::array<uint32_t, vec_sz> accum_res;
    vec_store((vec_t *)accum_res.data(), accum[i]);
    std::array<uint32_t, vec_sz> carry_res;
    vec_store((vec_t *)carry_res.data(), carry[i]);
    for (int i = 0; i < vec_sz; ++i) {
      sum[j++ % comp_cnt] += accum_res[i] | (((uint64_t)carry_res[i] + carry_fix_up) << 32);
    }
  }

  for (int i = 0; idx < data_sz; ++idx, ++i) {
    sum[i % comp_cnt] += *(data + idx);
  }

  return {
    static_cast<std::uint32_t>(sum[0] / std::max<std::uint64_t>(1, input.size())),
    static_cast<std::uint32_t>(sum[1] / std::max<std::uint64_t>(1, input.size())),
    static_cast<std::uint32_t>(sum[2] / std::max<std::uint64_t>(1, input.size())),
  };
}
