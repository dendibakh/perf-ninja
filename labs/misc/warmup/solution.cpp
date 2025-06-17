
#include "solution.h"

#ifdef SOLUTION
int solution(int *arr, int N) {
  return (N * (N + 1)) / 2;
}
#else
int solution(int *arr, int N) {
  int res = 0;
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;
}
#endif
