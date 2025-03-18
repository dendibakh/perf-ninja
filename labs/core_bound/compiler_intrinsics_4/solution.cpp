#include "solution.h"
#include "const.h"

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

namespace {
#if defined(__x86_64__) || defined(_M_X64)
#if defined(__AVX512F__)
using Vec = __m512d;
constexpr auto& vec_setzero = _mm512_setzero_pd;
constexpr auto& vec_set1 = _mm512_set1_pd;
constexpr auto& vec_load = _mm512_loadu_pd;
constexpr auto& vec_store = _mm512_storeu_pd;
constexpr auto& vec_add = _mm512_add_pd;
constexpr auto& vec_sub = _mm512_sub_pd;
constexpr auto& vec_mul = _mm512_mul_pd;
constexpr auto vec_cmpgt_mask = [](auto a, auto b) { return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ); };
#elif defined(__AVX2__)
using Vec = __m256d;
constexpr auto& vec_setzero = _mm256_setzero_pd;
constexpr auto& vec_set1 = _mm256_set1_pd;
constexpr auto& vec_load = _mm256_loadu_pd;
constexpr auto& vec_store = _mm256_storeu_pd;
constexpr auto& vec_add = _mm256_add_pd;
constexpr auto& vec_sub = _mm256_sub_pd;
constexpr auto& vec_mul = _mm256_mul_pd;
constexpr auto vec_cmpgt_mask = [](auto a, auto b) { return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_GT_OQ)); };
#else
using Vec = __m128d;
constexpr auto& vec_setzero = _mm_setzero_pd;
constexpr auto& vec_set1 = _mm_set1_pd;
constexpr auto& vec_load = _mm_loadu_pd;
constexpr auto& vec_store = _mm_storeu_pd;
constexpr auto& vec_add = _mm_add_pd;
constexpr auto& vec_sub = _mm_sub_pd;
constexpr auto& vec_mul = _mm_mul_pd;
constexpr auto vec_cmpgt_mask = [](auto a, auto b) { return _mm_movemask_pd(_mm_cmpgt_pd(a, b)); };
#endif
#else
using Vec = float64x2_t;
constexpr auto vec_setzero = []() { return vdupq_n_f64(0.0); };
constexpr auto& vec_set1 = vdupq_n_f64;
constexpr auto vec_load = [](const auto* a) { return vld1q_f64(a); };
constexpr auto vec_store = [](auto* a, auto b) { vst1q_f64(a, b); };
constexpr auto& vec_add = vaddq_f64;
constexpr auto& vec_sub = vsubq_f64;
constexpr auto& vec_mul = vmulq_f64;
constexpr auto vec_cmpgt_mask = [](auto a, auto b) {
  const auto cmp = vcgtq_f64(a, b);
  const auto bit0 = vgetq_lane_u64(cmp, 0) & 0b01;
  const auto bit1 = vgetq_lane_u64(cmp, 1) & 0b10;
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
  const size_t data_size = (data_width * data_height + kVecSize - 1) / kVecSize * kVecSize;
  std::vector<short> data(data_size);
  auto px = 0;
  auto py = 0;
  const auto squared_bound = vec_set1(kSquareBound);
  for (int data_idx = 0; data_idx < data_size; data_idx += kVecSize) {
    std::array<double, kVecSize> c_x_src;
    std::array<double, kVecSize> c_y_src;
    for (int i = 0; i < kVecSize; ++i) {
      c_x_src[i] = min_x + (max_x - min_x) * px / data_width;
      c_y_src[i] = min_y + (max_y - min_y) * py / data_height;
      if (++px == data_width) {
        px = 0;
        ++py;
      }
    }
    const auto c_x = vec_load(c_x_src.data());
    const auto c_y = vec_load(c_y_src.data());
    auto z_x = vec_setzero();
    auto z_y = vec_setzero();
    std::array<int, kVecSize> res;
    res.fill(kMaxIterations);
    auto res_cnt = 0;
    for (int iter_cnt = 0; iter_cnt < kMaxIterations; ++iter_cnt) {
      const auto z_xx = vec_mul(z_x, z_x);
      const auto z_yy = vec_mul(z_y, z_y);
      for (unsigned mask = vec_cmpgt_mask(vec_add(z_xx, z_yy), squared_bound); mask; ) {
        const auto res_idx = std::countr_zero(mask);
        res_cnt += res[res_idx] == kMaxIterations;
        res[res_idx] = std::min(res[res_idx], iter_cnt);
        mask -= 1 << res_idx;
      }
      if (res_cnt == kVecSize) {
        break;
      }
      const auto z_xy = vec_mul(z_x, z_y);
      z_x = vec_add(vec_sub(z_xx, z_yy), c_x);
      z_y = vec_add(vec_add(z_xy, z_xy), c_y);
    }
    std::copy(res.begin(), res.end(), data.begin() + data_idx);
  }
  data.resize(data_width * data_height);
  return data;
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
