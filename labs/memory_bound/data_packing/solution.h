#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// Force solution for CI
#define SOLUTION

// FIXME: this data structure can be reduced in size
struct S {
#ifdef SOLUTION
  double d;
  long long l;
  int i;
  short s;
  bool b;
#elif defined(SOLUTION2)
  float d;
  unsigned long long l:16;
  unsigned int i:7;
  unsigned short s:7;
  bool b:1;
#else
  int i;
  long long l;
  short s;
  double d;
  bool b;
#endif

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
