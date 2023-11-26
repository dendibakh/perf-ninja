
#include <array>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

// FIXME: this data structure can be reduced in size
struct S {
    float d;                    // 4 max 1
    unsigned long long l : 15;  // 2 max 10k
    unsigned int i : 7;         // 1 max 100
    unsigned short s : 7;       // 1 max 100
    bool b : 1;

    bool operator<(const S &s) const { return this->i < s.i; }
};

template <int N>
class TD;

// TD<sizeof(S)> td;

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
