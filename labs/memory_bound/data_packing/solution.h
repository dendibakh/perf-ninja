#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
#define SOLUTION
#ifdef SOLUTION
struct S {
  float d;
  long long l : 16;
  int i : 8;
  short s : 7;
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#else
struct S {
  int i;
  long long l;
  short s;
  double d;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#endif

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
