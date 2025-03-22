#include "solution.hpp"
#include <algorithm>

#define SOLUTION_DENIS
#ifdef SOLUTION_DENIS
#ifdef __x86_64__
  #include <immintrin.h>
#else
  #include <arm_neon.h>
#endif
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

  // [Denis]: I implemented this naive reduction.
  // Perhaps you can do it better with vpermq or similar, but I was lazy.
  // But does it really matter? I guess you would see no difference
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
#else
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
  // we will process 8 Positions per iteration (8 * 12 bytes or 12 * sizeof(std::uint64_t) )
  // [Denis]: Apple M1 needs 8 accumulators to achieve maximum throughput.
  // We have dependent uaddw.2d instructions in the loop.
  // Accodring to https://dougallj.github.io/applecpu/firestorm-simd.html,
  // uaddw.2d throughput is 4 per cycle and latency is 2, 
  // thus we need 2 * 4 accumulators for maximum performance.
  // For our algorithm, it is easier to have 12 accumulators.
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

  // [Denis]: again, maybe this reduction can be improved.
  // But I guess it would not make a difference/
  uint64x2_t acc_XY = vaddq_u64(vaddq_u64(acc_XY1, acc_XY2), vaddq_u64(acc_XY3, acc_XY4));
  uint64x2_t acc_ZX = vaddq_u64(vaddq_u64(acc_ZX1, acc_ZX2), vaddq_u64(acc_ZX3, acc_ZX4));
  uint64x2_t acc_YZ = vaddq_u64(vaddq_u64(acc_YZ1, acc_YZ2), vaddq_u64(acc_YZ3, acc_YZ4));

  x += vgetq_lane_u64(acc_XY, 0);
  y += vgetq_lane_u64(acc_XY, 1);

  z += vgetq_lane_u64(acc_ZX, 0);
  x += vgetq_lane_u64(acc_ZX, 1);

  y += vgetq_lane_u64(acc_YZ, 0);
  z += vgetq_lane_u64(acc_YZ, 1);
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
#elif defined(SOLUTION_JONATHAN)
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>
template<class To, class From>
std::enable_if_t<
        sizeof(To) == sizeof(From) &&
                std::is_trivially_copyable_v<From> &&
                std::is_trivially_copyable_v<To>,
        To>
// constexpr support needs compiler magic
bit_cast(const From &src) noexcept {
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

template<class T, std::size_t N>
struct SIMDVector {
    struct vector_helper {
        using type __attribute__((__vector_size__(N * sizeof(T)))) = T; // get around GCC restriction
    };
    using builtin_vector = typename vector_helper::type;

    alignas(N * sizeof(T)) std::array<T, N> data{};

    constexpr SIMDVector() {}
    constexpr SIMDVector(T value) {
        for (T &x: this->data) x = value;
    }

    constexpr SIMDVector(std::array<T, N> data) {
        this->data = data;
    }

    constexpr SIMDVector(builtin_vector bv) : SIMDVector{from_builtin_vector(bv)} {
    }

    template<class G>
    constexpr SIMDVector(SIMDVector<G, N> const &other) : SIMDVector{__builtin_convertvector(other.to_builtin_vector(), builtin_vector)} {
    }

    SIMDVector from_builtin_vector(builtin_vector bv) {
        return {bit_cast<std::array<T, N>>(bv)};
    }

    builtin_vector to_builtin_vector() const {
        return bit_cast<builtin_vector>(*this);
    }

    static constexpr SIMDVector load(T const *ptr) {
        SIMDVector res;
        for (std::size_t i = 0; i < N; ++i) res.data[i] = ptr[i];
        return res;
    }

    constexpr void store(T *ptr) {
        for (std::size_t i = 0; i < N; ++i) ptr[i] = this->data[i];
    }

    constexpr SIMDVector operator+(SIMDVector rhs) const {
        return bit_cast<SIMDVector>(to_builtin_vector() + rhs.to_builtin_vector());
    }

    constexpr SIMDVector &operator+=(SIMDVector rhs) {
        return *this = *this + rhs;
    }
};

#if defined(__arm__) || defined(__aarch64__)
#include <arm_neon.h>
#endif

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
#if __x86_64__
#if defined(__AVX512F__)
    constexpr auto native_simd_size = 64;
#elif defined(__AVX__)
    constexpr auto native_simd_size = 32;
#elif defined(__SSE__)
    constexpr auto native_simd_size = 16;
#else
    constexpr auto native_simd_size = 0;
#endif
    static_assert(native_simd_size > 0, "make sure your target CPU supports SIMD!");

    constexpr auto unroll = native_simd_size / sizeof(std::uint64_t);

    std::size_t i = 0;

    using VecU64 = SIMDVector<std::uint64_t, unroll>;
    using VecU32 = SIMDVector<std::uint32_t, unroll>;

    std::array<VecU64, 3> real_accs = {};
    for (; i + unroll <= input.size(); i += unroll) {
        std::array<std::uint32_t, 3 * unroll> casted;
        for (std::size_t j = 0; j < unroll; ++j) {
            casted[3 * j + 0] = input[i + j].x;
            casted[3 * j + 1] = input[i + j].y;
            casted[3 * j + 2] = input[i + j].z;
        }

        for (std::size_t k = 0; k < 3; ++k) {
            real_accs[k] += VecU64{VecU32::load(casted.data() + unroll * k)};
        }
    }

    alignas(128) std::array<std::uint64_t, 3 * unroll> acc;
    for (std::size_t k = 0; k < 3; ++k) {
        real_accs[k].store(acc.data() + unroll * k);
    }

    std::uint64_t x = 0;
    std::uint64_t y = 0;
    std::uint64_t z = 0;

    for (std::size_t j = 0; j < unroll; ++j) {
        x += acc[3 * j + 0];
        y += acc[3 * j + 1];
        z += acc[3 * j + 2];
    }

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
#elif defined(__arm__) || defined(__aarch64__)
    constexpr auto native_simd_size = 16;
    static_assert(native_simd_size > 0, "make sure your target CPU supports SIMD!");

    constexpr auto unroll = native_simd_size / sizeof(std::uint64_t);

    std::size_t i = 0;

    using VecU64 = SIMDVector<std::uint64_t, unroll>;
    using VecU32 = SIMDVector<std::uint32_t, unroll>;

    std::array<VecU64, 12> real_accs = {};
    for (; i + 4 * unroll <= input.size(); i += 4 * unroll) {
        for (std::size_t k = 0; k < 12; ++k) {
            real_accs[k] += VecU64{vmovl_u32(vld1_u32(&input[i].x + unroll * k))};
        }
    }
    for (std::size_t k = 0; k < 3; ++k) {
        real_accs[k] += real_accs[k + 3];
        real_accs[k] += real_accs[k + 6];
        real_accs[k] += real_accs[k + 9];
    }

    alignas(128) std::array<std::uint64_t, 3 * unroll> acc;
    for (std::size_t k = 0; k < 3; ++k) {
        real_accs[k].store(acc.data() + unroll * k);
    }

    std::uint64_t x = 0;
    std::uint64_t y = 0;
    std::uint64_t z = 0;

    for (std::size_t j = 0; j < unroll; ++j) {
        x += acc[3 * j + 0];
        y += acc[3 * j + 1];
        z += acc[3 * j + 2];
    }

    for (; i < input.size(); ++i) {
        x += input[i].x;
        y += input[i].y;
        z += input[i].z;
        asm ("" : "+r"(i));
    }

    return {
            static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
            static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
            static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
    };
#endif
}
#else
Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  for (auto pos: input) {
    x += pos.x;
    y += pos.y;
    z += pos.z;
  }

  return {
          static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
  };
}
#endif