
#include <array>
#include <limits>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S_Old {
  double d;                     // 0-1
  short l;                      // (0-100)*(0-100) = 0-10000 -- 14
  signed char i;                // 0-100 -- 7
  signed char s;                // 0-100 -- 7
  bool b;                       // true/false -- 1

  bool operator<(const S_Old &s) const { return this->i < s.i; }
};
static_assert( std::numeric_limits<short>::max() >= 10000);
static_assert( std::numeric_limits<signed char>::max() >= 100);

struct S_bitpacked {
  float d;                      // 0-1
  unsigned l : 14;              // (0-100)*(0-100) = 0-10000 -- 14
  unsigned i : 7;               // 0-100 -- 7
  unsigned s : 7;               // 0-100 -- 7
  unsigned b : 1;               // true/false -- 1

  bool operator<(const S_bitpacked &s) const { return this->i < s.i; }
};
using S = S_Old;

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
