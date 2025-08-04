#include "const.h"
#include "solution.h"

#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#else
#include <arm_neon.h>
#endif

#include <array>
#include <bit>
#include <cstdint>

namespace {
  #if defined(__x86_64__) || defined(_M_X64)
  #if defined(__AVX512F__)
  using Vec = __m512d;
  using VecInt = __m512i;
  constexpr auto vec_setzero = []() { return _mm512_setzero_pd(); };
  constexpr auto vec_set1 = [](auto a) { return _mm512_set1_pd(a); };
  constexpr auto vec_set1_int = [](auto a) { return _mm512_set1_epi64(a); };
  constexpr auto vec_load = [](auto* a) { return _mm512_load_pd(a); };
  constexpr auto vec_store_int = [](auto* a, auto b) { _mm512_store_si512((VecInt*)a, b); };
  constexpr auto vec_add = [](auto a, auto b) { return _mm512_add_pd(a, b); };
  constexpr auto vec_add_int = [](auto a, auto b) { return _mm512_add_epi64(a, b); };
  constexpr auto vec_sub = [](auto a, auto b) { return _mm512_sub_pd(a, b); };
  constexpr auto vec_mul = [](auto a, auto b) { return _mm512_mul_pd(a, b); };
  constexpr auto vec_blend = [](auto a, auto b, auto m) { return _mm512_mask_blend_pd(m, a, b); };
  constexpr auto vec_blend_int = [](auto a, auto b, auto m) { return _mm512_mask_blend_epi64(m, a, b); };
  constexpr auto vec_cmpeq_int = [](auto a, auto b) { return _mm512_cmpeq_epi64_mask(a, b); };
  constexpr auto vec_cmpgt = [](auto a, auto b) { return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ); };
  constexpr auto vec_or_mask = [](auto a, auto b) { return a | b; };
  constexpr auto vec_movemask = [](auto m) { return m; };
  #elif defined(__AVX2__)
  using Vec = __m256d;
  using VecInt = __m256i;
  constexpr auto vec_setzero = []() { return _mm256_setzero_pd(); };
  constexpr auto vec_set1 = [](auto a) { return _mm256_set1_pd(a); };
  constexpr auto vec_set1_int = [](auto a) { return _mm256_set1_epi64x(a); };
  constexpr auto vec_load = [](auto* a) { return _mm256_load_pd(a); };
  constexpr auto vec_store_int = [](auto* a, auto b) { _mm256_store_si256((VecInt*)a, b); };
  constexpr auto vec_add = [](auto a, auto b) { return _mm256_add_pd(a, b); };
  constexpr auto vec_add_int = [](auto a, auto b) { return _mm256_add_epi64(a, b); };
  constexpr auto vec_sub = [](auto a, auto b) { return _mm256_sub_pd(a, b); };
  constexpr auto vec_mul = [](auto a, auto b) { return _mm256_mul_pd(a, b); };
  constexpr auto vec_blend = [](auto a, auto b, auto m) { return _mm256_blendv_pd(a, b, m); };
  constexpr auto vec_blend_int = [](auto a, auto b, auto m) { return _mm256_castpd_si256(_mm256_blendv_pd(_mm256_castsi256_pd(a), _mm256_castsi256_pd(b), m)); };
  constexpr auto vec_cmpeq_int = [](auto a, auto b) { return _mm256_castsi256_pd(_mm256_cmpeq_epi64(a, b)); };
  constexpr auto vec_cmpgt = [](auto a, auto b) { return _mm256_cmp_pd(a, b, _CMP_GT_OQ); };
  constexpr auto vec_or_mask = [](auto a, auto b) { return _mm256_or_si256(a, b); };
  constexpr auto vec_movemask = [](auto m) { return _mm256_movemask_pd(m); };
  #else
  using Vec = __m128d;
  using VecInt = __m128i;
  constexpr auto vec_setzero = []() { return _mm_setzero_pd(); };
  constexpr auto vec_set1 = [](auto a) { return _mm_set1_pd(a); };
  constexpr auto vec_set1_int = [](auto a) { return _mm_set1_epi64x(a); };
  constexpr auto vec_load = [](auto* a) { return _mm_load_pd(a); };
  constexpr auto vec_store_int = [](auto* a, auto b) { _mm_store_si128((VecInt*)a, b); };
  constexpr auto vec_add = [](auto a, auto b) { return _mm_add_pd(a, b); };
  constexpr auto vec_add_int = [](auto a, auto b) { return _mm_add_epi64(a, b); };
  constexpr auto vec_sub = [](auto a, auto b) { return _mm_sub_pd(a, b); };
  constexpr auto vec_mul = [](auto a, auto b) { return _mm_mul_pd(a, b); };
  constexpr auto vec_blend = [](auto a, auto b, auto m) { return _mm_blendv_pd(a, b, m); };
  constexpr auto vec_blend_int = [](auto a, auto b, auto m) { return _mm_castpd_si128(_mm_blendv_pd(_mm_castsi128_pd(a), _mm_castsi128_pd(b), m)); };
  constexpr auto vec_cmpeq_int = [](auto a, auto b) { return _mm_castsi128_pd(_mm_cmpeq_epi64(a, b)); };
  constexpr auto vec_cmpgt = [](auto a, auto b) { return _mm_cmpgt_pd(a, b); };
  constexpr auto vec_or_mask = [](auto a, auto b) { return _mm_or_pd(a, b); };
  constexpr auto vec_movemask = [](auto m) { return _mm_movemask_pd(m); };
  #endif
  #else
  using Vec = float64x2_t;
  using VecInt = uint64x2_t;
  constexpr auto vec_setzero = []() { return vdupq_n_f64(0.0); };
  constexpr auto vec_set1 = [](auto a) { return vdupq_n_f64(a); };
  constexpr auto vec_set1_int = [](auto a) { return vdupq_n_u64(a); };
  constexpr auto vec_load = [](const auto* a) { return vld1q_f64(a); };
  constexpr auto vec_store_int = [](auto* a, auto b) { vst1q_u64(a, b); };
  constexpr auto vec_add = [](auto a, auto b) { return vaddq_f64(a, b); };
  constexpr auto vec_add_int = [](auto a, auto b) { return vaddq_u64(a, b); };
  constexpr auto vec_sub = [](auto a, auto b) { return vsubq_f64(a, b); };
  constexpr auto vec_mul = [](auto a, auto b) { return vmulq_f64(a, b); };
  constexpr auto vec_blend = [](auto a, auto b, auto m) { return vbslq_f64(m, b, a); };
  constexpr auto vec_blend_int = [](auto a, auto b, auto m) { return vbslq_u64(m, b, a); };
  constexpr auto vec_cmpeq_int = [](auto a, auto b) { return vceqq_u64(a, b); };
  constexpr auto vec_cmpgt = [](auto a, auto b) { return vcgtq_f64(a, b); };
  constexpr auto vec_or_mask = [](auto a, auto b) { return vorrq_u64(a, b); };
  constexpr auto vec_movemask = [](auto m) {
    const auto bit0 = vgetq_lane_u64(m, 0) & 0b01;
    const auto bit1 = vgetq_lane_u64(m, 1) & 0b10;
    return bit1 | bit0;
  };
  #endif  // __x86_64__
  constexpr auto kVecSize = sizeof(Vec) / sizeof(double);
  constexpr int kUnrollSz = 4;
}  // namespace


std::vector<short> mandelbrot(int image_width, int image_height) {
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  const auto diameter_y = kDiameterX / image_width * image_height;
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;
  const auto kBatchSize = kVecSize * kUnrollSz;
  const size_t data_size = data_width * data_height;
  std::vector<short> data((data_size + kBatchSize - 1) / kBatchSize * kBatchSize);
  auto px = 0;
  auto py = 0;
  const auto squared_bound = vec_set1(kSquareBound);
  for (auto data_idx = 0; data_idx < data_size; data_idx += kBatchSize) {
    std::array<Vec, kUnrollSz> c_x, c_y, z_x, z_y;
    std::array<std::array<int, kVecSize>, kUnrollSz> res;
    std::array<uint32_t, kUnrollSz> finished_mask;
    for (auto u = 0; u < kUnrollSz; ++u) {
      std::array<double, kVecSize> c_x_src;
      std::array<double, kVecSize> c_y_src;
      for (auto i = 0; i < kVecSize; ++i) {
        c_x_src[i] = min_x + (max_x - min_x) * px / data_width;
        c_y_src[i] = min_y + (max_y - min_y) * py / data_height;
        if (++px == data_width) {
          px = 0;
          ++py;
        }
      }
      c_x[u] = vec_load(c_x_src.data());
      c_y[u] = vec_load(c_y_src.data());
      z_x[u] = vec_setzero();
      z_y[u] = vec_setzero();
      res[u].fill(kMaxIterations);
      finished_mask[u] = (1 << kVecSize) - 1;
    }
    auto active_cnt = kUnrollSz;
    for (auto iter_cnt = 0; iter_cnt < kMaxIterations && active_cnt != 0; ++iter_cnt) {
      for (auto u = 0; u < kUnrollSz; ++u) {
        if (finished_mask[u] == 0) {
          continue;
        }
        const auto z_xx = vec_mul(z_x[u], z_x[u]);
        const auto z_yy = vec_mul(z_y[u], z_y[u]);
        for (
          uint32_t mask = vec_movemask(vec_cmpgt(vec_add(z_xx, z_yy), squared_bound)) & finished_mask[u];
          mask;
          mask &= mask - 1
        ) {
          const auto res_idx = std::countr_zero(mask);
          finished_mask[u] &= ~((uint32_t)1 << res_idx);
          res[u][res_idx] = iter_cnt;
          active_cnt -= finished_mask[u] == 0;
        }
        const auto z_xy = vec_mul(z_x[u], z_y[u]);
        z_x[u] = vec_add(vec_sub(z_xx, z_yy), c_x[u]);
        z_y[u] = vec_add(vec_add(z_xy, z_xy), c_y[u]);
      }
    }
    for (auto u = 0; u < kUnrollSz; ++u) {
      std::copy(res[u].begin(), res[u].end(), data.begin() + data_idx + u * kVecSize);
    }
  }
  data.resize(data_size);
  return data;
}
