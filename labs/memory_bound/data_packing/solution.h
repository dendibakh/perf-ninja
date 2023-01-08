#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr unsigned short minRandom = 0;
constexpr unsigned short maxRandom = 70;

// FIXME: this data structure can be reduced in size
struct S {
  float d;
  unsigned short l;
  unsigned short i : 8;
  unsigned short s : 7;
  bool b : 1;
  
  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
