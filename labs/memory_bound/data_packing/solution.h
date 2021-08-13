
#include <array>
#include <limits>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S_Old {
  float d;                     // 0-1
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
  unsigned l : 16;              // (0-100)*(0-100) = 0-10000 -- 14
  unsigned s : 8;               // 0-100 -- 7
  bool b : 1;                   // true/false -- 1
  unsigned i : 7;               // 0-100 -- 7

  bool operator<(const S_bitpacked &s) const { return this->i < s.i; }
};
using S = S_bitpacked;

template<int N>
class TD;

// TD<sizeof(S_Old)> td1;
// TD<sizeof(S_bitpacked)> td2;

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
