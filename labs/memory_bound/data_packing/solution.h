
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size

struct S {
  long long l;
  int i;
  double d;
  short s;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};


void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
