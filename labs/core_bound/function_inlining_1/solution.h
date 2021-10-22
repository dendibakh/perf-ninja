
#include <array>
#include <cstddef>
#include <cstdint>

// Assume this constant never changes
constexpr size_t N = 10000;

struct S {
  uint32_t key1;
  uint32_t key2;
};

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
