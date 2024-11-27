// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d; // [0-.99999]
  unsigned long long l : 16; // [0-9 801]
  unsigned int i : 8; // [0-99]
  unsigned short s : 7; // [0-99]
  bool b : 1; // [0-1]

  bool operator<(const S &s) const { return this->i < s.i; }
};

// Scott meyers trick

template<int N, typename T>
class TSize;

// never compiles but shows the value of sizeof(s)
//TSize<sizeof(S), decltype(S::l)> td;


void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
