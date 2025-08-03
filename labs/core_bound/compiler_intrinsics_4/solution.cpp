#include "const.h"
#include "solution.h"
#include <cmath>

#define SOLUTION
#if defined(SOLUTION) && defined(__AVX2__)

#include <immintrin.h>

#include <array>

inline double lerp(const double a, const double b, const double t)
{
  // a * (1 - t) + b * t
  // a - a * t + b * t
  // a + (b - a) * t
  return a + (b - a) * t;
}

inline __m256d lerp(const __m256d a, const __m256d b, const __m256d t)
{
  // a * (1 - t) + b * t
  // a - a * t + b * t
  // a + (b - a) * t
  const __m256d ba = _mm256_sub_pd(b, a);
  return _mm256_fmadd_pd(ba, t, a);
}

std::vector<short> mandelbrot(int image_width, int image_height)
{
  constexpr auto kUnrollSize = 4;
  constexpr auto kVecSize = 4;

  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;

  const auto unaligned_width = data_width % (kVecSize * kUnrollSize);

  const auto diameter_y = kDiameterX / image_width * image_height;

  const auto inv_data_width = 1. / data_width;
  const auto inv_data_height = 1. / data_height;
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;

  const __m256d kSquareBound_wide = _mm256_set1_pd(kSquareBound);
  const __m256d inv_data_width_wide = _mm256_set1_pd(inv_data_width);
  const __m256d inv_data_height_wide = _mm256_set1_pd(inv_data_height);
  const __m256d min_x_wide = _mm256_set1_pd(min_x);
  const __m256d max_x_wide = _mm256_set1_pd(max_x);
  const __m256d min_y_wide = _mm256_set1_pd(min_y);
  const __m256d max_y_wide = _mm256_set1_pd(max_y);

  std::vector<short> result(data_width * data_height);
  auto result_idx = 0;
  for (auto py = 0; py < data_height; ++py)
  {
    auto px = 0;

    const auto c_y = lerp(min_y, max_y, 1.0 * py * inv_data_height);

    const __m256d py_wide = _mm256_set1_pd((double)py);
    const __m256d c_y_wide = lerp(min_y_wide, max_y_wide, _mm256_mul_pd(py_wide, inv_data_height_wide));

    // Unaligned data
    for (; px < unaligned_width; ++px)
    {
      const auto c_x = lerp(min_x, max_x, 1.0 * px * inv_data_width);
      auto z_x = 0.0;
      auto z_y = 0.0;
      auto iter_cnt = 0;
      for (; iter_cnt < kMaxIterations; ++iter_cnt)
      {
        const auto z_xx = z_x * z_x;
        const auto z_yy = z_y * z_y;
        if (z_xx + z_yy > kSquareBound)
        {
          break;
        }
        const auto z_xy = z_x * z_y;
        z_x = z_xx - z_yy + c_x;
        z_y = z_xy + z_xy + c_y;
      }
      result[result_idx++] = iter_cnt;
    }

    std::array<__m256d, kUnrollSize> c_x_wide_arr;
    std::array<__m256d, kUnrollSize> z_x_wide_arr;
    std::array<__m256d, kUnrollSize> z_y_wide_arr;
    std::array<int, kUnrollSize> prev_mask_arr;

    for (; px < data_width; px += kVecSize * kUnrollSize)
    {
      for (size_t u = 0; u < kUnrollSize; ++u)
      {
        const __m256d px_wide = _mm256_set_pd(((double)px + u * kVecSize + 3), ((double)px + u * kVecSize + 2), ((double)px + u * kVecSize + 1), ((double)px + u * kVecSize + 0));
        c_x_wide_arr[u] = lerp(min_x_wide, max_x_wide, _mm256_mul_pd(px_wide, inv_data_width_wide));

        z_x_wide_arr[u] = _mm256_setzero_pd();
        z_y_wide_arr[u] = _mm256_setzero_pd();

        prev_mask_arr[u] = 0xF;
      }

      // Use to count if any vector is still alive (doing useful work)
      auto alive = kUnrollSize;
      auto iter_cnt = 0;
      for (; iter_cnt < kMaxIterations && alive > 0; ++iter_cnt)
      {
        for (size_t u = 0; u < kUnrollSize; ++u)
        {
          if (prev_mask_arr[u] == 0)
          {
            // Skip dead vectors
            continue;
          }

          const __m256d z_xx_wide = _mm256_mul_pd(z_x_wide_arr[u], z_x_wide_arr[u]);
          const __m256d z_yy_wide = _mm256_mul_pd(z_y_wide_arr[u], z_y_wide_arr[u]);
          const __m256d lte_mask_wide = _mm256_cmp_pd(_mm256_add_pd(z_xx_wide, z_yy_wide), kSquareBound_wide, _CMP_LE_OQ);
          const int lte_mask = _mm256_movemask_pd(lte_mask_wide);

          const int mask_change = lte_mask ^ prev_mask_arr[u];
          if (mask_change != 0)
          {
            for (auto i = 0; i < kVecSize; i++)
            {
              if ((mask_change >> i) & 1)
              {
                result[result_idx + u * kVecSize + i] = iter_cnt;
              }
            }

            // No one passed
            if (lte_mask == 0)
            {
              // Mark as dead the vector
              --alive;
            }
          }
          prev_mask_arr[u] = lte_mask;

          const __m256d z_xy_wide = _mm256_mul_pd(z_x_wide_arr[u], z_y_wide_arr[u]);
          z_x_wide_arr[u] = _mm256_add_pd(_mm256_sub_pd(z_xx_wide, z_yy_wide), c_x_wide_arr[u]);
          z_y_wide_arr[u] = _mm256_add_pd(_mm256_add_pd(z_xy_wide, z_xy_wide), c_y_wide);
        }
      }

      // Survivors
      for (size_t u = 0; u < kUnrollSize; ++u)
      {
        for (auto i = 0; i < kVecSize; i++)
        {
          if ((prev_mask_arr[u] >> i) & 1)
          {
            result[result_idx + u * kVecSize + i] = iter_cnt;
          }
        }
      }

      result_idx += kVecSize * kUnrollSize;
    }
  }
  return result;
}
#else
std::vector<short> mandelbrot(int image_width, int image_height)
{
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  const auto diameter_y = kDiameterX / image_width * image_height;
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;
  std::vector<short> result(data_width * data_height);
  auto result_idx = 0;
  for (auto py = 0; py < data_height; ++py)
  {
    for (auto px = 0; px < data_width; ++px)
    {
      const auto c_x = std::lerp(min_x, max_x, 1.0 * px / data_width);
      const auto c_y = std::lerp(min_y, max_y, 1.0 * py / data_height);
      auto z_x = 0.0;
      auto z_y = 0.0;
      auto iter_cnt = 0;
      for (; iter_cnt < kMaxIterations; ++iter_cnt)
      {
        const auto z_xx = z_x * z_x;
        const auto z_yy = z_y * z_y;
        if (z_xx + z_yy > kSquareBound)
        {
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
