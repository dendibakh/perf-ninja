// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;


struct S_orig {
  int i;
  long long l;
  short s;
  double d;
  bool b;

  bool operator<(const S_orig &s) const { return this->i < s.i; }
};

struct S {
  float d; // 4
  unsigned l:16; // 2
  unsigned i:8; // 1
  unsigned s:7;
  unsigned b:1; // 1

  bool operator<(const S &s) const { return this->i < s.i; }
};

// template<int T>
// class Temp;
// Temp<sizeof(S)> tem;
// perf stat --topdown -a taskset -c 0 ./lab

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
