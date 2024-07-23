
#include "solution.h"
#include <algorithm>
#include <stdlib.h>
#include <tuple>

void solution(std::array<S, N> &arr) {
    std::sort(arr.data(), arr.data() + arr.size(), [](auto lhs, auto rhs) {
        return std::tie(lhs.key1, lhs.key2) < std::tie(rhs.key1, rhs.key2);
    });
}
