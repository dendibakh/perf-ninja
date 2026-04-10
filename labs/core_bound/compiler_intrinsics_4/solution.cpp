#include "const.h"
#include "solution.h"
#include <array>
#include <cstdint>
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

    const size_t data_size = data_width * data_height;

    constexpr auto VEC_SIZE = 4;                   // 4 doubles per SIMD register
    constexpr auto UNROLL = 4;                     // 4 registers per loop iteration
    constexpr auto BATCH_SIZE = VEC_SIZE * UNROLL; // 16 pixels processed at once

    // Padding to make the result vector a multiple of BATCH_SIZE
    std::vector<short> result((data_size + BATCH_SIZE - 1) / BATCH_SIZE * BATCH_SIZE);

    auto px = 0, py = 0;
    const auto squared_bound = _mm256_set1_pd(kSquareBound);

    // 1D Flattened Loop (Safely wrap across row boundaries)
    for (size_t data_idx = 0; data_idx < data_size; data_idx += BATCH_SIZE)
    {
        // Arrays of 4 SIMD registers
        std::array<__m256d, UNROLL> v_c_x, v_c_y, v_z_x, v_z_y;
        std::array<__m256i, UNROLL> v_iter_cnt;
        // Initialize batch of 16 pixels
        for (auto u = 0; u < UNROLL; ++u)
        {
            alignas(32) std::array<double, VEC_SIZE> c_x_src;
            alignas(32) std::array<double, VEC_SIZE> c_y_src;
            for (auto i = 0; i < VEC_SIZE; ++i)
            {
                c_x_src[i] = min_x + (max_x - min_x) * px / data_width;
                c_y_src[i] = min_y + (max_y - min_y) * py / data_height;
                if (++px == data_width)
                {
                    px = 0;
                    ++py;
                } // Wrap to next row
            }
            v_c_x[u] = _mm256_load_pd(c_x_src.data());
            v_c_y[u] = _mm256_load_pd(c_y_src.data());
            v_z_x[u] = _mm256_setzero_pd();
            v_z_y[u] = _mm256_setzero_pd();
            v_iter_cnt[u] = _mm256_setzero_si256();
        }

        // Unrolled Hot Loop
        for (auto iter_cnt = 0; iter_cnt < kMaxIterations; ++iter_cnt)
        {
            auto global_active_mask = 0x0;

            for (auto u = 0; u < UNROLL; ++u)
            {
                const auto v_z_xx = _mm256_mul_pd(v_z_x[u], v_z_x[u]);
                const auto v_z_yy = _mm256_mul_pd(v_z_y[u], v_z_y[u]);
                const auto mask = _mm256_cmp_pd(_mm256_add_pd(v_z_xx, v_z_yy), squared_bound, _CMP_LE_OQ);

                auto mmask = _mm256_movemask_pd(mask);
                global_active_mask |= mmask; // Track if ANY of the 16 pixels are still active

                // Only do the math if this specific register still has active pixels
                if (mmask)
                {
                    const auto v_z_xy = _mm256_mul_pd(v_z_x[u], v_z_y[u]);
                    v_z_x[u] = _mm256_add_pd(_mm256_sub_pd(v_z_xx, v_z_yy), v_c_x[u]);
                    v_z_y[u] = _mm256_add_pd(_mm256_add_pd(v_z_xy, v_z_xy), v_c_y[u]);
                    v_iter_cnt[u] = _mm256_sub_epi64(v_iter_cnt[u], _mm256_castpd_si256(mask));
                }
            }

            // Break entirely if all 16 pixels have escaped
            if (!global_active_mask)
                break;
        }

        // Extraction
        for (auto u = 0; u < UNROLL; ++u)
        {
            alignas(32) std::array<uint64_t, VEC_SIZE> iter_cnts;
            _mm256_store_si256(reinterpret_cast<__m256i *>(&iter_cnts), v_iter_cnt[u]);
            for (auto i = 0; i < VEC_SIZE; ++i)
                result[data_idx + u * VEC_SIZE + i] = static_cast<short>(iter_cnts[i]);
        }
    }

    // We didn't need tail loop since we had padding, but we do need to remove the dummy pixels at the end
    result.resize(data_size);
    return result;
}
