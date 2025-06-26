#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

//#pragma pack(push, 4)
struct S {
  float d;
  unsigned long long l :16;
  unsigned int i       :8;
  unsigned short s     :7;
  bool b               :1; 

  bool operator<(const S &s) const { return this->i < s.i; }
};
//#pragma pack(pop)

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
