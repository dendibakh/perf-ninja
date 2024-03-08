
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d;
  long long l: 16;
  int i: 8;
  short s: 2;
  bool b: 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

// template <int N>
// class Test;
// Test<sizeof(S)> test;

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
