#include <vector>

// Assume those constants never change
constexpr int N = 1000000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
/*
4 + pad(4) + 8 + 2 + pad(6) + 8 + 1 + pad(7) = 40 [original]
8 + 8 + 4 + 2 + 1 + 1 = 24 [after rearrangement]
4 + 4 = 8 [bit packing]

NOTE: For bit packing, l needs 14 bits, i needs 7 bits and s needs 7 bits. 
But they are all signed, hence we should reserve one extra bit for all of them.
l -> 15, i -> 8, s -> 8
the final 1 bit can then be used for b.  
double is changed to float because according to the input, the value that was
being stored in double could be represented by a float just as well. 
*/
struct S {
  float d;
  long long l : 15;
  int i : 8;
  short s : 8;
  bool b : 1;

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::vector<S> &arr);
S create_entry(int first_value, int second_value);
void solution(std::vector<S> &arr);
