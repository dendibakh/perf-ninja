
#include "solution.h"

int solution(int *arr, int N) {
  // arr[i] = i + 1, 0 <= i < N
  // return sum of elements of arr
  return (N * (N + 1)) >> 1;
}
