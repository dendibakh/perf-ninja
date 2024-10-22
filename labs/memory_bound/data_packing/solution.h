// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <array>
#include <cstdint>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// Smaller does not mean better :-)
// If the size of the structure is divisible by 16, the compiler will use VMOVUPS instructions. Otherwise, it will have to use slower MOVs.
struct S {
  float d;
  unsigned int l : 14;
  unsigned int i : 7;
  unsigned int s : 7;
  unsigned int b : 1;
  // std::int64_t l; // [0; 9801]; 2 bytes are enough but this will be slower
  // std::int8_t i;  // [0; 99]
  // std::int8_t s;  // [0; 99]
  // std::int_fast8_t b; // [0; 1]; can pack into a bit field, but this does not really makes sense

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
