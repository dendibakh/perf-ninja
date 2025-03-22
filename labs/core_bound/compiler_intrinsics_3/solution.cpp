#include "solution.hpp"
#include <algorithm>

#ifdef __x86_64__
  #include <immintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
  #include <arm_neon.h>
#elif defined(__riscv) || defined(__riscv__) || defined(__riscv64)
  #include <riscv_vector.h>
#endif

// The solution that we present here is "readable" (explicit) but not necessary the best performant.

// The first issue is that it does not adapt well to every microarchitecture. For example,
// we need to have enough vector accumulators to maximize execution throughput. This solution
// has fixed number of accumulators, which may not scale well with future microarchitectures.
// More generic solution (by Jonathan Hallström) can be found here: 
// https://github.com/dendibakh/perf-ninja/tree/compiler_intrinsics_3_solution

// Also, we can make it even faster if we don't extend 32 bits into 64 bits, 
// but do 32-bit unsinged additions and count overflows (wraparounds). E.g.:
// uint32_t x = 0;
// uint32_t x_ovflw = 0;
// ...
//   x += input[i].x;
//   if (x < input[i].x)
//     x_ovflw += 1; // wraparound happend
// When this code is expressed in vector form, it becomes faster than the presented solution 
// thanks to 2x improved addition throughput: we add vectors of 32-bit vectors instead of 64-bit vectors.

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  int i = 0;
#ifdef __x86_64__
  __m256i acc_XYZX = _mm256_setzero_si256();
  __m256i acc_YZXY = _mm256_setzero_si256();
  __m256i acc_ZXYZ = _mm256_setzero_si256();
  // we will process 4 Positions per iteration (4 * 12 bytes or 3 * sizeof(XMM) )
  constexpr int UNROLL = 4;
  auto input_ptr = reinterpret_cast<const __m128i *>(&input[0].x);
  for (; i + UNROLL - 1 < input.size(); i += UNROLL) {
    __m128i XMM_XYZX = _mm_load_si128(input_ptr + 0); // load 128 bits
    __m128i XMM_YZXY = _mm_load_si128(input_ptr + 1);  
    __m128i XMM_ZXYZ = _mm_load_si128(input_ptr + 2);  
    input_ptr += 3;
    __m256i YMM_XYZX = _mm256_cvtepu32_epi64(XMM_XYZX); // 32 bit -> 64 bit (vpmovsxdq)
    __m256i YMM_YZXY = _mm256_cvtepu32_epi64(XMM_YZXY); 
    __m256i YMM_ZXYZ = _mm256_cvtepu32_epi64(XMM_ZXYZ);
    acc_XYZX = _mm256_add_epi64(acc_XYZX, YMM_XYZX); // accumulation
    acc_YZXY = _mm256_add_epi64(acc_YZXY, YMM_YZXY);
    acc_ZXYZ = _mm256_add_epi64(acc_ZXYZ, YMM_ZXYZ);
  }

  // Perhaps this naive reduction can be improved with vpermq or similar,
  // but does it really matter? Perhaps you would see no difference
  // since this code not in the hot loop.

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
#elif defined(__aarch64__) || defined(_M_ARM64)
  uint64x2_t acc_XY1 = vdupq_n_u64(0);
  uint64x2_t acc_ZX1 = vdupq_n_u64(0);
  uint64x2_t acc_YZ1 = vdupq_n_u64(0);
  uint64x2_t acc_XY2 = vdupq_n_u64(0);
  uint64x2_t acc_ZX2 = vdupq_n_u64(0);
  uint64x2_t acc_YZ2 = vdupq_n_u64(0);
  uint64x2_t acc_XY3 = vdupq_n_u64(0);
  uint64x2_t acc_ZX3 = vdupq_n_u64(0);
  uint64x2_t acc_YZ3 = vdupq_n_u64(0);
  uint64x2_t acc_XY4 = vdupq_n_u64(0);
  uint64x2_t acc_ZX4 = vdupq_n_u64(0);
  uint64x2_t acc_YZ4 = vdupq_n_u64(0);  
  // Apple M1 needs 8 accumulators to achieve maximum throughput.
  // We have dependent uaddw.2d instructions in the loop.
  // Accodring to https://dougallj.github.io/applecpu/firestorm-simd.html,
  // uaddw.2d throughput is 4 per cycle and latency is 2, 
  // thus we need 2 * 4 accumulators for maximum performance.
  // For our algorithm, it is easier to have 12 accumulators.
  // we will process 8 Positions per iteration (8 * 12 bytes or 12 * sizeof(std::uint64_t) )
  constexpr int UNROLL = 8;
  auto input_ptr = reinterpret_cast<const std::uint64_t *>(&input[0].x);
  for (; i + UNROLL - 1 < input.size(); i += UNROLL) {
    uint32x2_t XY1 = vld1_u64(input_ptr + 0); // load 64 bits
    uint32x2_t ZX1 = vld1_u64(input_ptr + 1); 
    uint32x2_t YZ1 = vld1_u64(input_ptr + 2); 
    uint32x2_t XY2 = vld1_u64(input_ptr + 3); 
    uint32x2_t ZX2 = vld1_u64(input_ptr + 4); 
    uint32x2_t YZ2 = vld1_u64(input_ptr + 5); 
    uint32x2_t XY3 = vld1_u64(input_ptr + 6);
    uint32x2_t ZX3 = vld1_u64(input_ptr + 7); 
    uint32x2_t YZ3 = vld1_u64(input_ptr + 8); 
    uint32x2_t XY4 = vld1_u64(input_ptr + 9); 
    uint32x2_t ZX4 = vld1_u64(input_ptr + 10); 
    uint32x2_t YZ4 = vld1_u64(input_ptr + 11);     
    input_ptr += 12;
    uint64x2_t eXY1 = vmovl_u32(XY1); // 32 bit -> 64 bit
    uint64x2_t eZX1 = vmovl_u32(ZX1); 
    uint64x2_t eYZ1 = vmovl_u32(YZ1); 
    uint64x2_t eXY2 = vmovl_u32(XY2); 
    uint64x2_t eZX2 = vmovl_u32(ZX2); 
    uint64x2_t eYZ2 = vmovl_u32(YZ2); 
    uint64x2_t eXY3 = vmovl_u32(XY3);
    uint64x2_t eZX3 = vmovl_u32(ZX3); 
    uint64x2_t eYZ3 = vmovl_u32(YZ3); 
    uint64x2_t eXY4 = vmovl_u32(XY4); 
    uint64x2_t eZX4 = vmovl_u32(ZX4); 
    uint64x2_t eYZ4 = vmovl_u32(YZ4);     
    acc_XY1 = vaddq_u64(acc_XY1, eXY1); // accumulation
    acc_ZX1 = vaddq_u64(acc_ZX1, eZX1); 
    acc_YZ1 = vaddq_u64(acc_YZ1, eYZ1); 
    acc_XY2 = vaddq_u64(acc_XY2, eXY2); 
    acc_ZX2 = vaddq_u64(acc_ZX2, eZX2);     
    acc_YZ2 = vaddq_u64(acc_YZ2, eYZ2); 
    acc_XY3 = vaddq_u64(acc_XY3, eXY3);
    acc_ZX3 = vaddq_u64(acc_ZX3, eZX3); 
    acc_YZ3 = vaddq_u64(acc_YZ3, eYZ3); 
    acc_XY4 = vaddq_u64(acc_XY4, eXY4); 
    acc_ZX4 = vaddq_u64(acc_ZX4, eZX4);     
    acc_YZ4 = vaddq_u64(acc_YZ4, eYZ4);     
  }

  // Maybe this reduction can be improved but perhaps it would not make a difference.
  uint64x2_t acc_XY = vaddq_u64(vaddq_u64(acc_XY1, acc_XY2), vaddq_u64(acc_XY3, acc_XY4));
  uint64x2_t acc_ZX = vaddq_u64(vaddq_u64(acc_ZX1, acc_ZX2), vaddq_u64(acc_ZX3, acc_ZX4));
  uint64x2_t acc_YZ = vaddq_u64(vaddq_u64(acc_YZ1, acc_YZ2), vaddq_u64(acc_YZ3, acc_YZ4));

  x += vgetq_lane_u64(acc_XY, 0);
  y += vgetq_lane_u64(acc_XY, 1);

  z += vgetq_lane_u64(acc_ZX, 0);
  x += vgetq_lane_u64(acc_ZX, 1);

  y += vgetq_lane_u64(acc_YZ, 0);
  z += vgetq_lane_u64(acc_YZ, 1);
#elif defined(__riscv) || defined(__riscv__) || defined(__riscv64)
  // [Denis]: This my first attempt. May not be an optimal solution. I'm still learning RISC-V ISA.
  // On BananaPi (Spacemit X60) I see 25% speedup (70 microsec -> 53 microsec)
  vuint64m2_t acc_XYZX = __riscv_vmv_v_x_u64m2(0, 4);
  vuint64m2_t acc_YZXY = __riscv_vmv_v_x_u64m2(0, 4);
  vuint64m2_t acc_ZXYZ = __riscv_vmv_v_x_u64m2(0, 4);
  // we will process 4 Positions per iteration (4 * 12 bytes or 3 * 256 bits )
  constexpr int UNROLL = 4;
  auto input_ptr = reinterpret_cast<const std::uint32_t *>(&input[0].x);
  for (; i + UNROLL - 1 < input.size(); i += UNROLL) {
    vuint32m1_t XMM_XYZX = __riscv_vle32_v_u32m1(input_ptr + 0, 4); // load 128 bits
    vuint32m1_t XMM_YZXY = __riscv_vle32_v_u32m1(input_ptr + 4, 4);
    vuint32m1_t XMM_ZXYZ = __riscv_vle32_v_u32m1(input_ptr + 8, 4);
    input_ptr += 12;
    vuint64m2_t YMM_XYZX = __riscv_vwcvtu_x_x_v_u64m2(XMM_XYZX, 4); // 32 bit -> 64 bit
    vuint64m2_t YMM_YZXY = __riscv_vwcvtu_x_x_v_u64m2(XMM_YZXY, 4);
    vuint64m2_t YMM_ZXYZ = __riscv_vwcvtu_x_x_v_u64m2(XMM_ZXYZ, 4);
    acc_XYZX = __riscv_vadd_vv_u64m2(acc_XYZX, YMM_XYZX, 4); // accumulation
    acc_YZXY = __riscv_vadd_vv_u64m2(acc_YZXY, YMM_YZXY, 4);
    acc_ZXYZ = __riscv_vadd_vv_u64m2(acc_ZXYZ, YMM_ZXYZ, 4);
  }

  // Perhaps this naive reduction can be improved with vpermq or similar,
  // but does it really matter? Perhaps you would see no difference
  // since this code not in the hot loop.

  // reduce acc_XYZX
  x += __riscv_vmv_x_s_u64m2_u64(acc_XYZX);
  acc_XYZX = __riscv_vslide1down_vx_u64m2(acc_XYZX, 0, 4);
  y += __riscv_vmv_x_s_u64m2_u64(acc_XYZX);
  acc_XYZX = __riscv_vslide1down_vx_u64m2(acc_XYZX, 0, 4);
  z += __riscv_vmv_x_s_u64m2_u64(acc_XYZX);
  acc_XYZX = __riscv_vslide1down_vx_u64m2(acc_XYZX, 0, 4);
  x += __riscv_vmv_x_s_u64m2_u64(acc_XYZX);

  // reduce acc_YZXY
  y += __riscv_vmv_x_s_u64m2_u64(acc_YZXY);
  acc_YZXY = __riscv_vslide1down_vx_u64m2(acc_YZXY, 0, 4);
  z += __riscv_vmv_x_s_u64m2_u64(acc_YZXY);
  acc_YZXY = __riscv_vslide1down_vx_u64m2(acc_YZXY, 0, 4);
  x += __riscv_vmv_x_s_u64m2_u64(acc_YZXY);
  acc_YZXY = __riscv_vslide1down_vx_u64m2(acc_YZXY, 0, 4);
  y += __riscv_vmv_x_s_u64m2_u64(acc_YZXY);

  // reduce acc_ZXYZ
  z += __riscv_vmv_x_s_u64m2_u64(acc_ZXYZ);
  acc_ZXYZ = __riscv_vslide1down_vx_u64m2(acc_ZXYZ, 0, 4);
  x += __riscv_vmv_x_s_u64m2_u64(acc_ZXYZ);
  acc_ZXYZ = __riscv_vslide1down_vx_u64m2(acc_ZXYZ, 0, 4);
  y += __riscv_vmv_x_s_u64m2_u64(acc_ZXYZ);
  acc_ZXYZ = __riscv_vslide1down_vx_u64m2(acc_ZXYZ, 0, 4);
  z += __riscv_vmv_x_s_u64m2_u64(acc_ZXYZ);
#endif

  // remainder
  for (; i < input.size(); ++i) {
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
