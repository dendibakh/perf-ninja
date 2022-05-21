
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
#pragma pack(push, 1)
struct S {
  float d;
  unsigned long long l:16;
  unsigned int i:8;
  unsigned short s:7;
  unsigned b:1;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#pragma pack(pop)

/*template <int N>
class TD;

TD<sizeof(S)> td;*/

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
