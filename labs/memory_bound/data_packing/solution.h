#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

struct S {
  unsigned l:14;   // 14 Bits
  unsigned i:7;    // 7 Bits
  unsigned s:7;    // 7 Bits
  bool b:1;        // 1 Bit
  float d;         // 4 Bytes

  // 29 Bits (+ Padding) + 4 Bytes = 8Bytes

  bool operator<(const S &s) const { return this->i < s.i; }
};

template <int N>
class TD;

// // Never compiles but shows the value of sizeof(S)
// TD<sizeof(S)> td;

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
