
#include "solution.h"
#include <algorithm>

void solution(std::array<S, N> &arr) {
    std::sort(
        begin(arr)
        , end(arr)
        , [](const S &a, const S &b) {
            if (a.key1 < b.key1)
                return true;

            if (a.key1 > b.key1)
                return false;

            if (a.key2 < b.key2)
                return true;

            if (a.key2 > b.key2)
                return false;

            return false;
        });
}
