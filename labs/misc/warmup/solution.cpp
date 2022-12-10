
#include "solution.h"

int solution(int *arr, const int N)
{
  // All the numbers in array are positive and sequential
  // from 1 to N. Use a formula to calculate the sum of them
  return (N * (N + 1)) / 2;
}