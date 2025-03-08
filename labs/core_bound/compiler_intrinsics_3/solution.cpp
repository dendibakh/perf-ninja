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

    std::array<VecU64, 6> real_accs = {};
    for (; i + unroll <= input.size(); i += unroll) {
        for (std::size_t k = 0; k < 6; ++k) {
            real_accs[k] += VecU64{vmovl_u32(vld1_u32(&input[i].x + unroll * k))};
        }
    }

    alignas(128) std::array<std::uint64_t, 6 * unroll> acc;
    for (std::size_t k = 0; k < 6; ++k) {
        real_accs[k].store(acc.data() + unroll * k);
    }

    std::uint64_t x = 0;
    std::uint64_t y = 0;
    std::uint64_t z = 0;

    for (std::size_t j = 0; j < unroll * 2; ++j) {
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