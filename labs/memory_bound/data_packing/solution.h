
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

struct S {
  float d;
  long long l : 16;
  char i : 8;
  char s : 7;
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

//template <int N> class TD;
//TD<sizeof(S)> td;

void init(std::array<S, N> &arr);
void solution(std::array<S, N> &arr);
