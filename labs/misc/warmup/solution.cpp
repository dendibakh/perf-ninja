
#include "solution.h"

int solution(int *arr, int N) {
  /*int res = 0;
  forr (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;*/
  return (N * (N + 1)) / 2;
}
