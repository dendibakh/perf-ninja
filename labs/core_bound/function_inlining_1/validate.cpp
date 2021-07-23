
#include "solution.h"
#include <iostream>
#include <stdlib.h>

static int originalCompare(const void *lhs, const void *rhs) {
  auto &a = *reinterpret_cast<const S *>(lhs);
  auto &b = *reinterpret_cast<const S *>(rhs);

  if (a.key1 < b.key1)
    return -1;

  if (a.key1 > b.key1)
    return 1;

  if (a.key2 < b.key2)
    return -1;

  if (a.key2 > b.key2)
    return 1;

  return 0;
}

static void originalSolution(std::array<S, N> &arr) {
  qsort(arr.data(), arr.size(), sizeof(S), originalCompare);
}

int main() {
  std::array<S, N> arr;
  init(arr);

  auto expected = arr;
  solution(arr);
  originalSolution(expected);

  for (size_t i = 0; i < N; i++) {
    if (arr[i].key1 != expected[i].key1) {
      std::cerr << "Validation Failed. Result[" << i
                << "].key1 = " << arr[i].key1 << ". Expected[" << i
                << "].key1 = " << expected[i].key1 << std::endl;
      return 1;
    }

    if (arr[i].key2 != expected[i].key2) {
      std::cerr << "Validation Failed. Result[" << i
                << "].key2 = " << arr[i].key2 << ". Expected[" << i
                << "].key2 = " << expected[i].key2 << std::endl;
      return 1;
    }
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
