
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d;
  unsigned l : 16;
  unsigned i : 8;
  unsigned s : 7;
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

template<int N>
class TD;

// TD<sizeof(S)> td;

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
