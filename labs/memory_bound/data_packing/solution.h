
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct alignas(8) S {
  float d;
  unsigned int l: 16; // 0..10000
  unsigned int i: 8; // 0..100
  unsigned int s: 7; // 0..100
  bool b: 1; // 0..1

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
