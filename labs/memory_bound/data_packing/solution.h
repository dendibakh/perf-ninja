// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include <cstdint>
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

#define SOLUTION

#ifdef SOLUTION

constexpr uint32_t all_mask = ~int32_t(0);
constexpr uint32_t lower_mask = all_mask >> 16;
constexpr uint32_t upper_mask = lower_mask << 16;

struct S {
  private:
  float d;  // 64 bit
  uint32_t li;
  uint32_t sb;

 public:
  bool getBool() const {
    return (bool)(sb >> 16);
  }
  uint16_t getShort() const {
    return (short)(sb & lower_mask);
  }
  void setBool(bool x) {
    sb &= lower_mask;
    sb |= (x << 16);
  }
  void setShort(uint16_t x) {
    sb &= upper_mask;
    sb |= x;
  }
  short getInt() const { return (short)(li & lower_mask);}
  short getLong() const { return (short)(li >> 16); }
  void setInt(short i) {
    li &= upper_mask;
    li |= (i & lower_mask);
  }
  void setLong(short l) {
    li &= lower_mask;
    li |= (l << 16 & upper_mask);
  }
  bool operator<(const S &s) const {
    // return this->i < s.i;
    return getInt() < s.getInt();
  }
  double getDouble() const {return d;}
  void setDouble(double x) {this->d = x;}
};

#else
// FIXME: this data structure can be reduced in size
struct S {
  private:
  int i; // 32
  long long l; // 64
  short s; // 16
  double d; // 64
  bool b;  // 1
  public:
  // sum = 32+64+16+64+1 =  177
  // : + padding = 32+64+16+64+8 = 184
  int getInt() const {return i;}
  void setInt(int i)  {this->i = i;}
  long long getLong() const {return l;}
  void setLong(long long l) {this->l = l;}
  short getShort() const {return s;}
  bool getBool() const {return b;}
  double getDouble() const {return d;}
  void setShort(short x) {this->s = x;}
  void setDouble(double x) {this->d = x;}
  void setBool(bool x) {this->b = x;}

  bool operator<(const S &s) const { return this->i < s.i; }
};
#endif


void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
