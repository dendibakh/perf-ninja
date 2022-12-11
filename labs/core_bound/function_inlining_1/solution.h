
#include <array>
#include <cstddef>
#include <cstdint>
#include <algorithm>

// Assume this constant never changes
constexpr size_t N = 10000;

struct S {
  uint32_t key1;
  uint32_t key2;
};

void init(std::array<S, N> &arr);

inline void solution(std::array<S, N> &arr)
{
  // Sort the array using std::sort and a lambda
  std::sort(arr.begin(), arr.end(), [](const S &a, const S &b) {
    // Use a one-liner to compare the two keys
    return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
  });
}
