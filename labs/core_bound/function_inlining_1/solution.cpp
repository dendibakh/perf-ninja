
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

static int compare(const S &a, const S &b) {

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

void solution(std::array<S, N> &arr) {
  //templated so in thoery it inlines..
  std::sort(arr.begin(), arr.end(), compare);
}
