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
#include <cmath>
#include <cstdint>

namespace {
  #if defined(__x86_64__) || defined(_M_X64)
  #if defined(__AVX512F__)
  using Vec = __m512d;
  constexpr auto& vec_setzero = _mm512_setzero_pd;
  constexpr auto& vec_set1 = _mm512_set1_pd;
  constexpr auto& vec_set1_int = _mm512_set1_epi64;
  constexpr auto& vec_load = _mm512_loadu_pd;
  constexpr auto& vec_store_int = _mm512_storeu_si512;
  constexpr auto& vec_add = _mm512_add_pd;
  constexpr auto& vec_add_int = _mm512_add_epi64;
  constexpr auto& vec_sub = _mm512_sub_pd;
  constexpr auto& vec_mul = _mm512_mul_pd;
  constexpr auto vec_blend = [](auto a, auto b, auto m) { return _mm512_mask_blend_pd(m, a, b); };
  constexpr auto vec_cmpeq_int64 = [](auto a, auto b) { return _mm512_cmpeq_epi64_mask(a, b); };
  constexpr auto vec_cmpgt_double = [](auto a, auto b) { return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ); };
  constexpr auto vec_or_mask = [](auto a, auto b) { return a | b; };
  constexpr auto vec_movemask = [](auto m) { return m; };
  #elif defined(__AVX2__)
  using Vec = __m256d;
  constexpr auto& vec_setzero = _mm256_setzero_pd;
  constexpr auto& vec_set1 = _mm256_set1_pd;
  constexpr auto& vec_set1_int = _mm256_set1_epi64x;
  constexpr auto& vec_load = _mm256_loadu_pd;
  constexpr auto vec_store_int = [](auto* a, auto b) { _mm256_storeu_si256((__m256i*)a, b); };
  constexpr auto& vec_add = _mm256_add_pd;
  constexpr auto& vec_add_int = _mm256_add_epi64;
  constexpr auto& vec_sub = _mm256_sub_pd;
  constexpr auto& vec_mul = _mm256_mul_pd;
  constexpr auto& vec_blend = _mm256_blendv_pd;
  constexpr auto& vec_cmpeq_int64 = _mm256_cmpeq_epi64;
  constexpr auto vec_cmpgt_double = [](auto a, auto b) { return _mm256_cmp_pd(a, b, _CMP_GT_OQ); };
  constexpr auto& vec_or_mask = _mm256_or_si256;
  constexpr auto& vec_movemask = _mm256_movemask_pd;
  #else
  using Vec = __m128d;
  constexpr auto& vec_setzero = _mm_setzero_pd;
  constexpr auto& vec_set1 = _mm_set1_pd;
  constexpr auto& vec_set1_int = _mm_set1_epi64x;
  constexpr auto& vec_load = _mm_loadu_pd;
  constexpr auto vec_store_int = [](auto* a, auto b) { _mm_storeu_si128((__m128i*)a, b); };
  constexpr auto& vec_add = _mm_add_pd;
  constexpr auto& vec_add_int = _mm_add_epi64;
  constexpr auto& vec_sub = _mm_sub_pd;
  constexpr auto& vec_mul = _mm_mul_pd;
  constexpr auto& vec_blend = _mm_blendv_pd;
  constexpr auto& vec_cmpeq_int64 = _mm_cmpeq_epi64;
  constexpr auto& vec_cmpgt_double = _mm_cmpgt_pd;
  constexpr auto& vec_or_mask = _mm_or_si128;
  constexpr auto& vec_movemask = _mm_movemask_pd;
  #endif
  #else
  using Vec = float64x2_t;
  constexpr auto vec_setzero = []() { return vdupq_n_f64(0.0); };
  constexpr auto& vec_set1 = vdupq_n_f64;
  constexpr auto& vec_set1_int = vdupq_n_u64;
  constexpr auto vec_load = [](const auto* a) { return vld1q_f64(a); };
  constexpr auto vec_store_int = [](auto* a, auto b) { vst1q_u64(a, b); };
  constexpr auto& vec_add = vaddq_f64;
  constexpr auto& vec_add_int = vaddq_u64;
  constexpr auto& vec_sub = vsubq_f64;
  constexpr auto& vec_mul = vmulq_f64;
  constexpr auto vec_blend = [](auto a, auto b, auto m) { return vbslq_f64(m, b, a); };
  constexpr auto& vec_cmpeq_int64 = vceqq_u64;
  constexpr auto& vec_cmpgt_double = vcgtq_f64;
  constexpr auto& vec_or_mask = vorrq_u64;
  constexpr auto vec_movemask = [](auto m) {
    const auto bit0 = vgetq_lane_u64(m, 0) & 0b01;
    const auto bit1 = vgetq_lane_u64(m, 1) & 0b10;
    return bit1 | bit0;
  };
  #endif  // __x86_64__
  constexpr auto kVecSize = sizeof(Vec) / sizeof(double);
}  // namespace

//#define SOLUTION
#ifdef SOLUTION

std::vector<short> mandelbrot(int image_width, int image_height) {
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  const auto diameter_y = kDiameterX / image_width * image_height;
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;  
  const size_t data_size = data_width * data_height;
  std::vector<short> data(data_size);
  const auto squared_bound = vec_set1(kSquareBound);
  const auto max_iter = vec_set1_int(kMaxIterations);
  const auto iter_inc = vec_set1_int(1);
  auto px = 0;
  auto py = 0;
  std::array<double, kVecSize> c_x_arr;
  std::array<double, kVecSize> c_y_arr;
  std::array<size_t, kVecSize> res_idx;
  size_t data_idx = 0;
  size_t res_used = 0;
  auto next_data_item = [&](int idx) {
    if (data_idx < data_size) {
      c_x_arr[idx] = std::lerp(min_x, max_x, 1.0 * px / data_width);
      c_y_arr[idx] = std::lerp(min_y, max_y, 1.0 * py / data_height);
      if (++px == data_width) {
        px = 0;
        ++py;
      }
      res_idx[idx] = data_idx;
      ++data_idx;
      ++res_used;
    } else {
      c_x_arr[idx] = 0.0;
      c_y_arr[idx] = 0.0;
      res_idx[idx] = -1;
    }
  };
  for (int i = 0; i < kVecSize; ++i) {
    next_data_item(i);
  }
  auto c_x = vec_load(c_x_arr.data());
  auto c_y = vec_load(c_y_arr.data());
  auto z_x = vec_setzero();
  auto z_y = vec_setzero();
  auto iter_cnt = vec_setzero();
  while (true) {
    const auto max_iter_mask = vec_cmpeq_int64(iter_cnt, max_iter);
    auto z_xx = vec_mul(z_x, z_x);
    auto z_yy = vec_mul(z_y, z_y);
    const auto squared_bound_mask = vec_cmpgt_double(vec_add(z_xx, z_yy), squared_bound);
    const auto cond_mask = vec_or_mask(max_iter_mask, squared_bound_mask);
    if (uint8_t mask = vec_movemask(cond_mask); mask) {
      std::array<uint64_t, kVecSize> iter_cnt_arr;
      vec_store_int(iter_cnt_arr.data(), iter_cnt);
      for (; mask; mask &= mask - 1) {
        const auto ridx = std::countr_zero(mask);
        if (res_idx[ridx] != -1) {
          data[res_idx[ridx]] = iter_cnt_arr[ridx];
          if (--res_used == 0) {
            return data;
          }
        }
        next_data_item(ridx);
      }
      z_x = vec_blend(z_x, vec_setzero(), cond_mask);
      z_y = vec_blend(z_y, vec_setzero(), cond_mask);
      z_xx = vec_blend(z_xx, vec_setzero(), cond_mask);
      z_yy = vec_blend(z_yy, vec_setzero(), cond_mask);
      c_x = vec_blend(c_x, vec_load(c_x_arr.data()), cond_mask);
      c_y = vec_blend(c_y, vec_load(c_y_arr.data()), cond_mask);
      iter_cnt = vec_blend(iter_cnt, vec_setzero(), cond_mask);
    }
    const auto z_xy = vec_mul(z_x, z_y);
    z_x = vec_add(vec_sub(z_xx, z_yy), c_x);
    z_y = vec_add(vec_add(z_xy, z_xy), c_y);
    iter_cnt = vec_add_int(iter_cnt, iter_inc);
  }
}

#else

std::vector<short> mandelbrot(int image_width, int image_height) {
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  const auto diameter_y = kDiameterX / image_width * image_height;
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;
  std::vector<short> result(data_width * data_height);
  auto result_idx = 0;
  for (auto py = 0; py < data_height; ++py) {
    for (auto px = 0; px < data_width; ++px) {
      const auto c_x = std::lerp(min_x, max_x, 1.0 * px / data_width);
      const auto c_y = std::lerp(min_y, max_y, 1.0 * py / data_height);
      auto z_x = 0.0;
      auto z_y = 0.0;
      auto iter_cnt = 0;
      for (; iter_cnt < kMaxIterations; ++iter_cnt) {
        const auto z_xx = z_x * z_x;
        const auto z_yy = z_y * z_y;
        if (z_xx + z_yy > kSquareBound) {
          break;
        }
        const auto z_xy = z_x * z_y;
        z_x = z_xx - z_yy + c_x;
        z_y = z_xy + z_xy + c_y;
      }
      result[result_idx++] = iter_cnt;
    }
  }
  return result;
}

#endif
