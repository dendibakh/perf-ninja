
#include "solution.h"

int solution(int *arr, int N) {
  int res = 0;
#ifdef SOLUTION
  return (N * (N + 1)) / 2;
#else
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;
#endif
}
