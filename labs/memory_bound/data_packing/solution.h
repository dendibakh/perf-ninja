#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
  unsigned i : 7;
  unsigned l : 14;
  unsigned s : 7;
  unsigned b : 1;
  float d;

  bool operator<(const S &s) const { return this->i < s.i; }
};

static_assert(sizeof(S) == 8, "Bit packing failed, performance improvement is not guaranteed");

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
