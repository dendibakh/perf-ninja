
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  // prefer use of byte sized types instead of 7 bit bitfield for ~5% better
  // performance at no increase of overall structure size
  short i;
  short s;

  bool operator<(const S &s) const { return this->i < s.i; }

  // Accessor methods that will still allow to retrieve values for l, d, b even
  // though they are not represented as members anymore
  long long get_l() const { return static_cast<short>(i * s); }
  double get_d() const { return static_cast<double>(i) / maxRandom; }
  bool get_b() const { return i < s; }
};

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
