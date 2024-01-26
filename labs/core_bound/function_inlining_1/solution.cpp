
#include "solution.h"
#include <algorithm>
#include <tuple>

[[gnu::always_inline]]
bool less(const S& a, const S& b) {
    return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
}

template <typename ItT>
void QuickSort(ItT beg, ItT end)
{
    if (end - beg < 2)
        return;
    ItT lft(beg);
    ItT rgt(end-1);
    auto pvt = *(lft + (rgt-lft)/2);
    if(less(*lft, pvt))
        while (less(*++lft, pvt));
    if(less(pvt, *rgt))
        while (less(pvt, *--rgt));

    while (lft < rgt)
    {
        std::iter_swap(lft, rgt);
        while (less(*++lft, pvt));
        while (less(pvt, *--rgt));
    }
    rgt++;
    QuickSort(beg, rgt);
    QuickSort(rgt, end);
}

void solution(std::array<S, N> &arr) {
    QuickSort(arr.begin(), arr.end());
//    std::sort(arr.begin(), arr.end(), &less);
}
