
#include "solution.h"
#include <algorithm>
#include <iostream>

int main() {
  std::array<S, N> arr;
  init(arr);

  auto expected = arr;
  solution(arr);
  std::sort(expected.begin(), expected.end());

  for (int i = 0; i < N; i++) {
    // we only check i components since sort can be unstable
    if (arr[i].i != expected[i].i) {
      std::cerr << "Validation Failed. Result[" << i << "].i = " << arr[i].i
                << ". Expected[" << i << "].i = " << expected[i].i << std::endl;
      return 1;
    }
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
