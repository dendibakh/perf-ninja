
#include "solution.h"
#include <algorithm>
#include <stdlib.h>


#ifndef SOLUTION
#  define SOLUTION 1
#endif

#if SOLUTION == 0
// Baseline.

static int compare(const void *lhs, const void *rhs)
{
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

void solution(std::array<S, N> &arr) {
  qsort(arr.data(), arr.size(), sizeof(S), compare);
}

#elif SOLUTION == 1
// My solution.

#include <algorithm>


struct Compare_s
{
  bool operator()(const S& lhs, const S& rhs)
  {
    if (lhs.key1 < rhs.key1)
      return true;

    if (lhs.key1 > rhs.key1)
      return false;

    if (lhs.key2 < rhs.key2)
      return true;

    if (lhs.key2 > rhs.key2)
      return false;

    return false;
  }
};

void solution(std::array<S, N>& array)
{
  std::sort(std::begin(array), std::end(array), Compare_s());
}

#elif SOLUTION == 2
// Facit.

void solution(std::array<S, N>& arr)
{
	std::sort(arr.begin(), arr.end(), [](S&a, S&b)
	{
		return a.key1 < b.key1 || (a.key1 == b.key1) && (a.key2 < b.key2);
	});
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif
