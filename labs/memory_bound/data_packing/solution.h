
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

struct S {
  double d;
  short l;
  char i;
  char s;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};

//template <int N> class TD;
//TD<sizeof(S)> td;

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
