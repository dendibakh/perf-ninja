
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

// Assume this constant never changes
constexpr size_t N = 1000;

struct S {
    uint32_t key1;
    uint32_t key2;
};

void init(std::array<S, N> &arr);

static auto Compare(const S &a, const S &b) -> bool {
    return (a.key1 < b.key1) || ((a.key1 == b.key1) && (a.key2 < b.key2));
}

static void Solution(std::array<S, N> &arr) {
    std::sort(std::begin(arr), std::end(arr), Compare);
}
