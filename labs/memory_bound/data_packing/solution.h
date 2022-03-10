
#include <array>

// Assume those constants never change
// comments
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// this data structure can be reduced in size
struct S {
  unsigned i:8;
  unsigned b:2;
  unsigned l:14;
  unsigned s:8;
  float d;

  bool operator<(const S &s) const { return this->i < s.i; }
};//}__attribute__((packed));


void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
