
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;


// Baseline ==> 363
// struct S {
//   int i;
//   long long l;
//   short s;
//   double d;
//   bool b;

//   bool operator<(const S &s) const { return this->i < s.i; }
// };


// solution 1 ==> 303/311
// struct S {
//   double d;
//   long long l;
//   int i;
//   short s;
//   bool b;

//   bool operator<(const S &s) const { return this->i < s.i; }
// };


// solution 2 ==> 279
struct S {
  bool operator<(const S &s) const { return this->i < s.i; }
  bool b;       // 1
  short i;      // 2
  short s;      // 2
  int l;        // 4
  float d;      // 8
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
