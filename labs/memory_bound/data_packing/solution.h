#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S
{
  float d;
  unsigned short l;
  unsigned char i;
  unsigned char s: 7;
  bool b :1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::vector<S> &arr);
//struct S {
//  int i;
//  long long l;
//  short s;
//  double d;
//  bool b;
//
//  bool operator<(const S &s) const { return this->i < s.i; }
//};

S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
