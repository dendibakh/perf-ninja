
#include <array>
#include <cstddef>
#include <cstdint>

// Assume this constant never changes
constexpr size_t N = 10000;

struct S {
  uint32_t key1;
  uint32_t key2;

  bool operator<(const S &other) const {
          if (key1 < other.key1) {
            return true;
          }
          if (key1 > other.key1) {
            return false;
          }
          if (key2 < other.key2) {
            return true;
          }
          if (key2 > other.key2) {
            return false;
          }
      }

};

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
