
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d; // 4 bytes
  unsigned long long l: 16; // 2 bytes
  unsigned int i: 8; // 1 bytes
  unsigned short s: 7; // 1 bytes
  bool b: 1; // 1 byte

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
