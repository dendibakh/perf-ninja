#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

//#pragma pack(push, 4)
struct S {
  int i       :8;
  short s     :8;
  long long l :16;
  float d;
  bool b; 

  bool operator<(const S &s) const { return this->i < s.i; }
};
//#pragma pack(pop)

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
