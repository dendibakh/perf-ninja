
#include "solution.h"
#include <algorithm>
#include <tuple>

bool less(const S& a, const S& b) {
//    return std::make_tuple(a.key1, a.key2) < std::make_tuple(b.key1, b.key2);
    if (a.key1 < b.key1)
        return true;

    if (a.key1 > b.key1)
        return false;

    if (a.key2 < b.key2)
        return true;

    if (a.key2 > b.key2)
        return false;

    return false;
}

void solution(std::array<S, N> &arr) {
    std::sort(arr.begin(), arr.end(), &less);
}
