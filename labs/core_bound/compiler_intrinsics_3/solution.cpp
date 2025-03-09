#include "solution.hpp"
#include <algorithm>
#include <array>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <vector>

#define SOLUTION
#ifdef SOLUTION

template<std::size_t N, class T>
struct vector_helper {
  using type __attribute__((__vector_size__(N * sizeof(T)))) = T; // get around GCC restriction
};
template<std::size_t N, class T>
using builtin_vector = typename vector_helper<N, T>::type;

template<std::size_t N, class T>
builtin_vector<N, T> unaligned_load(const T *ptr) {
  builtin_vector<N, T> vec;
  std::memcpy(&vec, ptr, sizeof(vec));
  return vec;
}

template<std::size_t N, class From, class To>
builtin_vector<N, To> convert(builtin_vector<N, From> x) { // using __builtin_convertvector gives horrible performance on gcc
#if __GNUC__
  From copied[N];
  std::memcpy(copied, &x, sizeof(copied));

  To temp[N];
  for (std::size_t i = 0; i < N; ++i) temp[i] = copied[i];

  builtin_vector<N, To> res;
  std::memcpy(&res, &temp, sizeof(res));

  return res;
#else
  return __builtin_convertvector(x, builtin_vector<N, To>);
#endif
}

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
#if __x86_64__
 #if defined(__AVX512F__)
  constexpr auto unroll = 8;
  constexpr auto native_simd_size = 64;
 #elif defined(__AVX__)
  constexpr auto unroll = 4;
  constexpr auto native_simd_size = 32;
 #elif defined(__SSE__)
  constexpr auto unroll = 4;
  constexpr auto native_simd_size = 16;
 #else
  constexpr auto unroll = 0;
  constexpr auto native_simd_size = 0;
 #endif
#elif defined(__arm__) || defined(__aarch64__)
 #if defined(__ARM_NEON)
  constexpr auto unroll = 8;
  constexpr auto native_simd_size = 16;
 #elif defined(__ARM_FEATURE_SVE)
  constexpr auto unroll = 8;
  constexpr auto native_simd_size = 32;
 #else
  constexpr auto unroll = 0;
  constexpr auto native_simd_size = 0;
 #endif
#endif
  static_assert(native_simd_size > 0, "make sure your target CPU supports SIMD!");

  constexpr auto vec_size = native_simd_size / sizeof(std::uint64_t);

  std::size_t i = 0;

  std::array<builtin_vector<vec_size, std::uint64_t>, 3 * unroll> accs{};
  for (; i + unroll * vec_size <= input.size(); i += unroll * vec_size) {
#if __GNUC__
#pragma GCC unroll 8
#else
#pragma unroll 8
#endif
    for (std::size_t k = 0; k < unroll; ++k) {
      const auto xyzx_u32 = unaligned_load<vec_size, std::uint32_t>(&input[i].x + vec_size * (3 * k + 0));
      const auto yzxy_u32 = unaligned_load<vec_size, std::uint32_t>(&input[i].x + vec_size * (3 * k + 1));
      const auto zxyz_u32 = unaligned_load<vec_size, std::uint32_t>(&input[i].x + vec_size * (3 * k + 2));

      const auto xyzx_u64 = convert<vec_size, std::uint32_t, std::uint64_t>(xyzx_u32);
      const auto yzxy_u64 = convert<vec_size, std::uint32_t, std::uint64_t>(yzxy_u32);
      const auto zxyz_u64 = convert<vec_size, std::uint32_t, std::uint64_t>(zxyz_u32);

      accs[3 * k + 0] += xyzx_u64;
      accs[3 * k + 1] += yzxy_u64;
      accs[3 * k + 2] += zxyz_u64;
    }
  }

  // colled the unrolled accumulators with the same layout to the first 3 accumulators
  for (std::size_t k = 1; k < unroll; ++k) {
    accs[0] += accs[k * 3 + 0];
    accs[1] += accs[k * 3 + 1];
    accs[2] += accs[k * 3 + 2];
  }

  // flatten the accumulators for easier reduction, this only copies the first three `VecU64`s from `accs`
  alignas(native_simd_size) std::array<std::uint64_t, 3 * vec_size> first_three_accs;
  std::memcpy(first_three_accs.data(), accs.data(), sizeof(first_three_accs));

  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  // reduce the first three accumulators from `acc`
  for (std::size_t j = 0; j < vec_size; ++j) {
    x += first_three_accs[3 * j + 0];
    y += first_three_accs[3 * j + 1];
    z += first_three_accs[3 * j + 2];
  }

  for (; i < input.size(); ++i) {
    x += input[i].x;
    y += input[i].y;
    z += input[i].z;
    asm ("" : "+r"(i)); // prevents unrolling of this loop, measurable speedup, at least on zen5
  }

  return {
          static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
  };
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
