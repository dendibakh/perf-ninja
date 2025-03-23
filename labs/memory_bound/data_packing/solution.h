// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

#define SOLUTION
#ifdef SOLUTION

#pragma pack(push, 1) // by default align by width of the fattest type
// FIXME: this data structure can be reduced in size
struct S {
  float d;
  unsigned int l : 14;
  unsigned short i : 7;
  unsigned short s : 7;
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#pragma pack(pop)

#else
// FIXME: this data structure can be reduced in size
struct S {
  int i;
  long long l;
  short s;
  double d;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#endif

// static_assert (sizeof(S) == 1024, "Size is not correct");

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
