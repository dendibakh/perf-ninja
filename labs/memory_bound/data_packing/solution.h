
#include <array>

#include <stdint.h>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  unsigned l:  14;
  unsigned b:   2;
  unsigned s:   8;
  unsigned i:   8;
  float d;

  bool operator<(const S &s) const { return this->i < s.i; }
};

static_assert(sizeof(S) == 8);

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
