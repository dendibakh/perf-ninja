
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

inline bool operator<(const S& a, const S& b) {
  if (a.key1 != b.key1) {
    return a.key1 < b.key1;
  }
  return a.key2 < b.key2;
}

template<typename Iter>
void MyInsertionSort(Iter beg, Iter end) {
  if (beg == end) {
    return;
  }
  for (Iter it = std::next(beg); it != end; ++it) {
    auto value = *it;
    Iter i = it;
    Iter j = std::prev(it);
    while (value < *j) {
      *i = *j;
      --i;
      if (j == beg)
        break;
      --j;
    }
    *i = value;
  }
}

template<typename Iter>
void MyQsort(Iter beg, Iter end, uint32_t from, uint32_t to) {
  const size_t N = std::distance(beg, end);
  if (N < 50) {
    MyInsertionSort(beg, end);
    return;
  }
  uint32_t pivot = from + (to-from)/2;
  Iter mid = std::partition(beg, end, [pivot](const auto& elem){
    return elem.key1 < pivot;
  });
  MyQsort(beg, mid, from, pivot);
  MyQsort(mid, end, pivot, to);
}

void solution(std::array<S, N> &arr) {
  MyQsort(arr.begin(), arr.end(), 0, std::numeric_limits<uint32_t>::max());
}
