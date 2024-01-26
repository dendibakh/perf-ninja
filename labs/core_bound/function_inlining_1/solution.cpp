
#include "solution.h"
#include <algorithm>
#include <tuple>

[[gnu::always_inline]]
bool less(const S& a, const S& b) {
    return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
}

[[gnu::always_inline]]
bool eq(const S& a, const S& b) {
    return a.key1 == b.key1 && a.key2 == b.key2;
}

[[gnu::always_inline]]
bool more(const S& a, const S& b) {
    return a.key1 > b.key1 || (a.key1 == b.key1 && a.key2 > b.key2);
}

template <typename RandomAccessIterator>
void quicksort(RandomAccessIterator first, RandomAccessIterator last) {
    if (first >= last) return;

    auto partition = [first, last] {
        auto pivot = *(last - 1);
        auto i = first;
        for (auto j = first; j < last - 1; ++j) {
            if (less(*j, pivot)) {
                std::iter_swap(i, j);
                ++i;
            }
        }
        std::iter_swap(i, last - 1);
        return i;
    };

    auto pivot = partition();

    quicksort(first, pivot);
    quicksort(pivot + 1, last);
}

void solution(std::array<S, N> &arr) {
    quicksort(arr.begin(), arr.end());
}
