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

// Note:
//
// There are two solutions presented here: SOLUTION and SOLUTION_ALT.
//
// SOLUTION is straightforward and simply processes pixels in small batches.
//
// SOLUTION_ALT tries to minimize the number of times the inner loop is executed,
// at the expense of slightly more complicated code within that loop.
// This implementation, for example on ARM, processes two adjacent pixels simultaneously
// (because only two double-precision FP values fit in a 128-bit NEON vector).
// When one pixel in a vector runs for 100 iterations, while the adjacent pixel in
// the same vector runs for 200 iterations, it will replace the data for the first pixel
// once it finishes its execution (otherwise, the algorithm would crunch 
// dummy data, as SOLUTION does).
//
// In practice, however, SOLUTION is both simpler and faster than SOLUTION_ALT.
// Still, SOLUTION_ALT is provided here for educational purposes.
//
// Both solutions use a software pipelining technique (see kUnrollSz) to increase
// instruction-level parallelism, which provides a significant performance boost for this task.

#define SOLUTION
// #define SOLUTION_ALT

#ifdef SOLUTION

// Solution provided by Christian Oliveros (@maniatic0) and adapted by Oleg Makovski (@0legmak).

std::vector<short> mandelbrot(int image_width, int image_height) {
  // Calculate the dimensions of the data array, adding a border for anti-aliasing
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  // Compute the vertical diameter based on the aspect ratio
  const auto diameter_y = kDiameterX / image_width * image_height;
  // Set the bounds of the Mandelbrot set in the complex plane
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;
  // Batch size for vectorized processing
  const auto kBatchSize = kVecSize * kUnrollSz;
  // Total number of pixels to process
  const size_t data_size = data_width * data_height;
  // Allocate output array, rounded up to batch size for vectorization
  std::vector<short> data((data_size + kBatchSize - 1) / kBatchSize * kBatchSize);
  // Pixel coordinates
  auto px = 0;
  auto py = 0;
  // Precompute squared escape bound for Mandelbrot iterations
  const auto squared_bound = vec_set1(kSquareBound);

  // Process the image in batches for vectorization and software pipelining
  for (auto data_idx = 0; data_idx < data_size; data_idx += kBatchSize) {
    // Arrays for vectorized coordinates and results
    alignas(sizeof(Vec)) std::array<Vec, kUnrollSz> c_x, c_y, z_x, z_y;
    std::array<std::array<int, kVecSize>, kUnrollSz> res;
    std::array<uint32_t, kUnrollSz> active_mask;

    // Initialize coordinates and masks for each unrolled vector
    for (auto u = 0; u < kUnrollSz; ++u) {
      std::array<double, kVecSize> c_x_src;
      std::array<double, kVecSize> c_y_src;
      for (auto i = 0; i < kVecSize; ++i) {
        // Map pixel coordinates to the complex plane
        c_x_src[i] = min_x + (max_x - min_x) * px / data_width;
        c_y_src[i] = min_y + (max_y - min_y) * py / data_height;
        // Move to next pixel, wrap to next row if needed
        if (++px == data_width) {
          px = 0;
          ++py;
        }
      }
      // Load coordinates into vector registers
      c_x[u] = vec_load(c_x_src.data());
      c_y[u] = vec_load(c_y_src.data());
      // Initialize z values to zero (start of Mandelbrot iteration)
      z_x[u] = vec_setzero();
      z_y[u] = vec_setzero();
      // Fill result array with max iterations (default value)
      res[u].fill(kMaxIterations);
      // Set mask to all bits set (all pixels active)
      active_mask[u] = (1 << kVecSize) - 1;
    }

    // Track number of active vectors in the batch
    auto active_vec_cnt = kUnrollSz;
    // Iterate up to max iterations or until all pixels in batch are completed
    for (auto iter_cnt = 0; iter_cnt < kMaxIterations && active_vec_cnt != 0; ++iter_cnt) {
      for (auto u = 0; u < kUnrollSz; ++u) {
        // Skip vector if all pixels are completed
        if (active_mask[u] == 0) {
          continue;
        }
        // Compute z_x^2 and z_y^2 for Mandelbrot formula
        const auto z_xx = vec_mul(z_x[u], z_x[u]);
        const auto z_yy = vec_mul(z_y[u], z_y[u]);
        // Check which pixels have escaped (z_x^2 + z_y^2 > bound)
        // Ignore previously completed pixels by ANDing active_mask
        for (
          uint32_t mask = vec_movemask(vec_cmpgt(vec_add(z_xx, z_yy), squared_bound)) & active_mask[u];
          mask;
          mask &= mask - 1
        ) {
          // Find index of first set bit (pixel that escaped)
          const auto res_idx = std::countr_zero(mask);
          // Mark pixel as completed by clearing the bit in active_mask
          active_mask[u] &= ~((uint32_t)1 << res_idx);
          // Store iteration count for escaped pixel
          res[u][res_idx] = iter_cnt;
          // Decrement active vector count if all pixels in vector are completed
          active_vec_cnt -= active_mask[u] == 0;
        }
        // Mandelbrot iteration: z = z^2 + c
        const auto z_xy = vec_mul(z_x[u], z_y[u]);
        z_x[u] = vec_add(vec_sub(z_xx, z_yy), c_x[u]);
        z_y[u] = vec_add(vec_add(z_xy, z_xy), c_y[u]);
      }
    }
    // Copy results for this batch into output array
    for (auto u = 0; u < kUnrollSz; ++u) {
      std::copy(res[u].begin(), res[u].end(), data.begin() + data_idx + u * kVecSize);
    }
  }
  // Resize output to actual image size (remove padding)
  data.resize(data_size);
  return data;
}

#elif defined(SOLUTION_ALT)

// Solution provided by Oleg Makovski (@0legmak).

std::vector<short> mandelbrot(int image_width, int image_height) {
  // Add two extra pixels to the data array dimensions to simplify anti-aliasing code
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  // Calculate the range of the Mandelbrot set in the y-axis based on the aspect ratio
  const auto diameter_y = kDiameterX / image_width * image_height;
  // Define the bounds of the Mandelbrot set in the x and y axes
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;
  // Total size of the data array
  const size_t data_size = data_width * data_height;
  // Initialize the result array to store the iteration counts
  std::vector<short> data(data_size);
  // Precompute constants for vectorized operations
  const auto squared_bound = vec_set1(kSquareBound); // Squared escape radius
  const auto max_iter = vec_set1_int(kMaxIterations); // Maximum iterations
  const auto iter_inc = vec_set1_int(1); // Increment for the iteration count
  // Arrays to track pixel positions and data indices for each unrolled loop
  std::array<int, kUnrollSz> px, py;
  std::array<size_t, kUnrollSz> data_idx;
  // Arrays to store the coordinates of the complex numbers (c_x, c_y)
  alignas(sizeof(Vec)) std::array<std::array<double, kVecSize>, kUnrollSz> c_x_arr, c_y_arr;
  // Array to store the indices of the results
  std::array<std::array<size_t, kVecSize>, kUnrollSz> res_idx;
  // Counter to track the number of points still being processed
  size_t in_proc_cnt = 0;
  // Inner function to prepare the next data point for unrolled loop iteration 'u' at vector index 'idx'
  auto next_data_point = [&](int idx, int u) {
    if (data_idx[u] < data_size) {
      // Map pixel coordinates to the complex plane
      c_x_arr[u][idx] = min_x + (max_x - min_x) * px[u] / data_width;
      c_y_arr[u][idx] = min_y + (max_y - min_y) * py[u] / data_height;
      // Move to the next pixel
      // Pixels are distributed between unrolled loop iterations in a round-robin fashion
      px[u] += kUnrollSz;
      if (px[u] >= data_width) {
        px[u] -= data_width;
        ++py[u];
      }
      // Store the index of the current data point
      res_idx[u][idx] = data_idx[u];
      data_idx[u] += kUnrollSz;
      ++in_proc_cnt;
    } else {
      // If no more data points, set to dummy values
      c_x_arr[u][idx] = 0.0;
      c_y_arr[u][idx] = 0.0;
      res_idx[u][idx] = -1;
    }
  };
  // Arrays to store vectorized values for the complex numbers and iteration counts
  alignas(sizeof(Vec)) std::array<Vec, kUnrollSz> c_x, c_y, z_x, z_y;
  alignas(sizeof(VecInt)) std::array<VecInt, kUnrollSz> iter_cnt;
  // Initialize starting values for each unrolled loop iteration
  for (auto u = 0; u < kUnrollSz; ++u) {
    px[u] = u;
    py[u] = 0;
    if (px[u] >= data_width) {
      px[u] -= data_width;
      ++py[u];
    }
    data_idx[u] = u;
    for (auto i = 0; i < kVecSize; ++i) {
      next_data_point(i, u);
    }
    c_x[u] = vec_load(c_x_arr[u].data());
    c_y[u] = vec_load(c_y_arr[u].data());
    z_x[u] = vec_setzero();
    z_y[u] = vec_setzero();
    iter_cnt[u] = vec_set1_int(0);
  }
  alignas(sizeof(Vec)) std::array<uint64_t, kVecSize> iter_cnt_arr;
  // Main loop to compute the Mandelbrot set
  while (true) {
    // Manually unroll the loop to increase instruction-level parallelism
    // This is especially important for M1 processors
    for (auto u = 0; u < kUnrollSz; ++u) {
      // Check if the maximum iteration count is reached
      const auto max_iter_mask = vec_cmpeq_int(iter_cnt[u], max_iter);
      // Compute z_x^2 and z_y^2
      auto z_xx = vec_mul(z_x[u], z_x[u]);
      auto z_yy = vec_mul(z_y[u], z_y[u]);
      // Check if the escape condition is met (z_x^2 + z_y^2 > squared_bound)
      const auto squared_bound_mask = vec_cmpgt(vec_add(z_xx, z_yy), squared_bound);
      // Compute the logical OR for maximum iterations and escape conditions
      const auto cond_mask = vec_or_mask(max_iter_mask, squared_bound_mask);
      // Process points that meet the condition above
      if (uint32_t mask = vec_movemask(cond_mask); mask) {
        vec_store_int(iter_cnt_arr.data(), iter_cnt[u]);
        // Process every set bit in the mask
        for (; mask; mask &= mask - 1) {
          // Get the index of a set bit
          const auto idx = std::countr_zero(mask);
          if (res_idx[u][idx] != -1) { // If not dummy data
            // Store the computation result into the output array
            data[res_idx[u][idx]] = static_cast<short>(iter_cnt_arr[idx]);
            if (--in_proc_cnt == 0) {
              return data; // Return the result if all points are processed
            }
          }
          next_data_point(idx, u);
        }
        // Reset values for points that met the condition
        z_x[u] = vec_blend(z_x[u], vec_setzero(), cond_mask);
        z_y[u] = vec_blend(z_y[u], vec_setzero(), cond_mask);
        z_xx = vec_blend(z_xx, vec_setzero(), cond_mask);
        z_yy = vec_blend(z_yy, vec_setzero(), cond_mask);
        c_x[u] = vec_blend(c_x[u], vec_load(c_x_arr[u].data()), cond_mask);
        c_y[u] = vec_blend(c_y[u], vec_load(c_y_arr[u].data()), cond_mask);
        iter_cnt[u] = vec_blend_int(iter_cnt[u], vec_set1_int(0), cond_mask);
      }
      // Update z_x and z_y for the next iteration
      const auto z_xy = vec_mul(z_x[u], z_y[u]);
      z_x[u] = vec_add(vec_sub(z_xx, z_yy), c_x[u]);
      z_y[u] = vec_add(vec_add(z_xy, z_xy), c_y[u]);
      // Increment the iteration count
      iter_cnt[u] = vec_add_int(iter_cnt[u], iter_inc);
    }
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
      const auto c_x = min_x + (max_x - min_x) * px / data_width;
      const auto c_y = min_y + (max_y - min_y) * py / data_height;
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
