#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

#define SOLUTION

#ifdef SOLUTION
struct S {
  float d;
  unsigned long long l:16;
  unsigned int i:8;
  unsigned short s:7;
  bool b:1;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#else
// FIXME: this data structure can be reduced in size
struct S {
  int i;
  long long l;
  short s;
  double d;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
#endif

template <int N>
class TD;
// never complies but shows the value of sizeof(S)
// TD<sizeof(S)> td;

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
