#include <cstdint>
#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  float d;
  uint16_t l;

  uint_fast8_t i; // using  the fast integer (3.51 ms instead of 6.74 ms with uint8_t)
  // uint_fast8_t is at least 8 bites but can be larger (maybe unsigned int = 32 bits)
  // uint8_t i;
  
  uint8_t s : 7;
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

template <int N> class TD;

// never compiles but shows the value of sizeof(S)
// TD<sizeof(S)> td;

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
