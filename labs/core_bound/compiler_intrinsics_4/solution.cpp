#include "const.h"
#include "solution.h"
#include <array>
#include "immintrin.h"

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
  const auto offsets = _mm256_set_pd(3.0, 2.0, 1.0, 0.0);
  for (auto py = 0; py < data_height; ++py)
  {
    auto px = 0;
    constexpr auto UNROLL = 4;
    for (; px + UNROLL <= data_width; px += UNROLL)
    {
      const auto v_x = _mm256_add_pd(_mm256_set1_pd(px), offsets);
      const auto v_c_x = _mm256_fmadd_pd(_mm256_set1_pd((max_x - min_x) / data_width), v_x, _mm256_set1_pd(min_x));
      const auto c_y = min_y + (max_y - min_y) * py / data_height;
      const auto v_c_y = _mm256_set1_pd(c_y);
      auto v_z_x = _mm256_set1_pd(0.0), v_z_y = _mm256_set1_pd(0.0);
      auto v_iter_cnt = _mm256_set1_epi64x(0);
      for (auto iter_cnt = 0; iter_cnt < kMaxIterations; ++iter_cnt)
      {
        const auto v_z_xx = _mm256_mul_pd(v_z_x, v_z_x);
        const auto v_z_yy = _mm256_mul_pd(v_z_y, v_z_y);
        const auto mask = _mm256_cmp_pd(_mm256_add_pd(v_z_xx, v_z_yy), _mm256_set1_pd(kSquareBound), _CMP_LE_OQ);
        if (!_mm256_movemask_pd(mask))
          break;
        const auto v_z_xy = _mm256_mul_pd(v_z_x, v_z_y);
        v_z_x = _mm256_add_pd(_mm256_sub_pd(v_z_xx, v_z_yy), v_c_x);
        v_z_y = _mm256_add_pd(_mm256_add_pd(v_z_xy, v_z_xy), v_c_y);
        v_iter_cnt = _mm256_sub_epi64(v_iter_cnt, _mm256_castpd_si256(mask));
      }
      std::array<uint64_t, UNROLL> iter_cnts;
      _mm256_storeu_si256(reinterpret_cast<__m256i *>(&iter_cnts), v_iter_cnt);
      for (auto i = 0; i < UNROLL; ++i)
        result[result_idx++] = static_cast<short>(iter_cnts[i]);
    }
  
    for (; px < data_width; ++px)
    {
      const auto c_x = min_x + (max_x - min_x) * px / data_width;
      const auto c_y = min_y + (max_y - min_y) * py / data_height;
      auto z_x = 0.0;
      auto z_y = 0.0;
      auto iter_cnt = 0;
      for (; iter_cnt < kMaxIterations; ++iter_cnt)
      {
        const auto z_xx = z_x * z_x;
        const auto z_yy = z_y * z_y;
        if (z_xx + z_yy > kSquareBound)
          break;
        const auto z_xy = z_x * z_y;
        z_x = z_xx - z_yy + c_x;
        z_y = z_xy + z_xy + c_y;
      }
      result[result_idx++] = iter_cnt;
    }
  }
  return result;
}
