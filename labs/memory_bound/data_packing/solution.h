
#include <array>
#include <cstdint>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

struct S {
  float d;

  /**
  float d;
  
  uint32_t l : 16;
  uint32_t i : 8;
  uint32_t s : 7;
  uint32_t b : 1;
  **/
  unsigned i : 8;
  unsigned s : 8;
  unsigned b : 2;
  unsigned l : 14; 
  
  bool operator<(const S &s) const { return this->i < s.i; }
};

static_assert(sizeof(S) == 8, "size mismatch");

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
