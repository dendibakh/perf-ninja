// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {

  float d; // 32 bits = 4 bytes ; double d; // 8
  long long l:15;// only need 16 bits // 64 bits = 8 bytes ?
  int i:8; // only need 8 bits // 4 
  short s:8;// only need 7 bits  // 2
  bool b:1; // 8 bits = 1 byte // no dataype can be smaller than 8,but S is our datatype
            // so we can make this smaller
  bool operator<(const S &s) const { return this->i < s.i; }
};

// template <int N>
// class TD;

// // never compiles but shows the value of sizeof(s)
// TD<sizeof(S)> td;

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
