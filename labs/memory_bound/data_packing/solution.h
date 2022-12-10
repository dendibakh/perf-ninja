
#include <array>

// Assume those constants never change
constexpr short N = 10000;
constexpr uint8_t minRandom = 0;
constexpr uint8_t maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  // Use a union to alternate between the 4 bytes of maskedData or a float
  unsigned int maskedData; 
  float d;

  // Determine whether the first value is less than the second value
  // By extracting the 29th bit
  bool operator<(const S &s) const { 
    return (maskedData & 0x20000000) < (s.maskedData & 0x20000000);
  }
};

void init(std::array<S, N> &arr);
S create_entry(const uint8_t first_value, const uint8_t second_value);
void solution(std::array<S, N> &arr);
