
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct __attribute__((packed)) S{
  float d;
  uint8_t i : 7;  // 7 bits are enough to represent 0-100 range
  uint8_t s : 7;
  uint16_t l : 14;  // max value  99*99, 14 bits are enough
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
