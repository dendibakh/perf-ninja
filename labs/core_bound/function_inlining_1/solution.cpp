#include "solution.h"
#include <algorithm>
#include <tuple>

bool less(const S& a, const S& b) {
    return std::make_tuple(a.key1, a.key2) < std::make_tuple(b.key1, b.key2);
}

void solution(std::array<S, N> &arr) {
    std::sort(arr.begin(), arr.end(), &less);
}
