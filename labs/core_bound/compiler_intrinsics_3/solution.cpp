#include "solution.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

#define SOLUTION
#ifdef SOLUTION
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
#elif defined(__arm__) || defined(__aarch64__)
#if defined(__ARM_NEON)
  constexpr auto native_simd_size = 16;
#elif defined(__ARM_FEATURE_SVE)
  constexpr auto native_simd_size = 32;
#else
  constexpr auto native_simd_size = 0;
#endif
#endif
  static_assert(native_simd_size > 0, "make sure your target CPU supports SIMD!");

  constexpr auto vec_size = native_simd_size / sizeof(std::uint64_t);

  std::size_t i = 0;

  using VecU64 = SIMDVector<std::uint64_t, vec_size>;
  using VecU32 = SIMDVector<std::uint32_t, vec_size>;

  const auto unroll = 2;

  std::array<VecU64, 3 * unroll> real_accs = {};
  for (; i + unroll * vec_size <= input.size(); i += unroll * vec_size) {
    for (std::size_t k = 0; k < 3 * unroll; ++k) {
      real_accs[k] += VecU64{VecU32::load(&input[i].x + vec_size * k)};
    }
  }
  for (std::size_t j = 0; j < 3; ++j) {
    for (std::size_t k = 1; k < unroll; ++k) {
      real_accs[j] += real_accs[j + k * 3];
    }
  }

  alignas(native_simd_size) std::array<std::uint64_t, 3 * vec_size> acc;
  for (std::size_t j = 0; j < 3; ++j) {
    real_accs[j].store(acc.data() + vec_size * j);
  }

  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  for (std::size_t j = 0; j < vec_size; ++j) {
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
