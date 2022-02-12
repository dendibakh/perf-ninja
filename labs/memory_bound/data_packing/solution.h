#include <cstdint>
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

#define SOLUTION

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

// given that maxRandom = 100, almost all of these can be stored in i8

// FIXME: this data structure can be reduced in size
struct S {
#ifdef SOLUTION
    // // XXX: l holds result `short * short` of which is only 4B so could
    // //      reduce this to an int?
    // i64  l;  // 8B
    // f64  d;  // 8B
    // i32  i;  // 4B
    // i16  s;  // 2B // XXX: this is compared against `second` which is an int...
    // bool b;  // 1B
    // // pad   // 1B

    // first  := [0, 100]
    // second := [0, 100]
    // s == second := [0, 100]
    // l == first * second => [0, 10000] ==> 14-bits
    // d == first / second               ==> use float (could really even do a 16-bit fixed floating point...)

    // XXX: honestly I feel like this is a little cheating since with basically
    //      any conversion warnings turned on this ould fail to compile...
    u8  i;      // 1B [0, 100]
    u8  s;      // 1B [0, 100]
    u16 l : 14; // 2B [0, 10000]
    u8  b :  1; //    [0, 1]
    u8  _ :  1;
    f32 d;      // 4B  // XXX: theoretically could compress this more, but would very likely end up with padding anyways
                // ---
                // Total: 8B

#else
  int i;         // 4B
  // padding     // 4B
  long long l;   // 8B
  short s;       // 2B
  // padding     // 6B
  double d;      // 8B
  bool b;        // 1B
  // padding     // 7B
#endif

  bool operator<(const S &s) const { return this->i < s.i; }
};
#ifdef SOLUTION
// static_assert(sizeof(S) == 24);
static_assert(sizeof(S) == 8);
#else
static_assert(sizeof(S) == 40);
#endif

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
