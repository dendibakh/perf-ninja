
#include <array>
#include <limits>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  double d;                     // 0-1
  short l;                      // (0-100)*(0-100) = 0-10000
  signed char i;                // 0-100
  signed char s;                // 0-100
  bool b;                       // true/false

  bool operator<(const S &s) const { return this->i < s.i; }
};
static_assert( std::numeric_limits<short>::max() >= 10000);
static_assert( std::numeric_limits<signed char>::max() >= 100);

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
