
#include <array>
#include <cstdint>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d;
  uint16_t l : 15;
  uint16_t b : 1;
  uint8_t i;
  uint8_t s;

  bool operator<(const S &s) const { return this->i < s.i; }
};

static_assert(sizeof(S) == 8, "The size of S must be 8 bytes.");

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
