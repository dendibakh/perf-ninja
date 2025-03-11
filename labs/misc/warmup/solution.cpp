
#include "solution.h"

int solution(int *arr, int N) {
  int res = 0;
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return (N * (N + 1)) / 2;
}
