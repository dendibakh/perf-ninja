
#include <array>
#include <cstdint>

// Assume this constant never changes
constexpr size_t N = 10000;

struct S {
  uint32_t key1;
  uint32_t key2;
};
int compare(const void *lhs, const void *rhs);

void init(std::array<S, N> &arr);
inline void solution(std::array<S, N> &arr){
    qsort(arr.data(), arr.size(), sizeof(S), compare);
};
