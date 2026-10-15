#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
// struct S {      // 40
//   int i;        // 4
//                 // 4 Padding
//   long long l;  // 8
//   short s;      // 2  
//                 // 6 Padding
//   double d;     // 8
//   bool b;       // 1
//                 // 7 Padding

//   bool operator<(const S &s) const { return this->i < s.i; }
// };

struct S {      // 16
  float d;      // 4
  short l;      // 2
  short i;      // 2
  short s;      // 2  
  bool b;       // 1
                // 5 offset
  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
