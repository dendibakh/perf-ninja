
#include <array>
#include <cinttypes>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d{}; //32 bits
  unsigned l : 14; 
  unsigned i : 8; 
  unsigned s : 8; 
  unsigned b : 1;

  //total 46 + 16 + 1 = 63 round 64 = 8 bytes, we save 4 bytes

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
