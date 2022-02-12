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

struct S {
#ifdef SOLUTION
    // first  := [0, 100]
    // second := [0, 100]
    // s == second := [0, 100]
    // l == first * second => [0, 10000] ==> 14-bits
    // d == first / second               ==> use float (could really even do a 16-bit fixed floating point...)

    // XXX: honestly I feel like this is a little cheating since with basically
    //      any conversion warnings turned on this would fail to compile...
    //      Even though I dislike them in general, this is where having
    //      accessors would be nice to 1) do the correct casting, and 2)
    //      verify that the inputs match the expectations.
    u8  i;      // 1B [0, 100]
    u8  s;      // 1B [0, 100]
    u16 l : 14; // 2B [0, 10000]
    u8  b :  1; //    [0, 1]
    u8  _ :  1;
    f32 d;      // 4B  // XXX: theoretically could compress this more, but would very likely end up with padding anyways
                // ---
                // Total: 8B

    // NOTE: I don't think it's in the spirit of the lab, but ofc w.r.t.
    //     information theory, the only values that we actually need to store
    //     are first and second, and everything else can be computed from those
    //     values. We'll pretend that the calculation of those values is
    //     expensive so we want to cache them in this struct.

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
