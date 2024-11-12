// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

struct S {
  float d;          // 32 bits
  long long l: 15;  // 15 bits
  int       i: 8;   // 8 bits
  short     s: 8;   // 8 bits
  bool      b: 1;   // 1 bit

  bool operator<(const S &s) const { return this->i < s.i; }
}; /** __attribute__( ( packed, aligned( 8 ) ) ) */

/** Template instantiation for size calculation */
//template< int N >
//class TD;

/// Evaluate `sizeof(S)` to get the size of struct `S`. Causes compile error.
//TD< sizeof( S ) > td;

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
