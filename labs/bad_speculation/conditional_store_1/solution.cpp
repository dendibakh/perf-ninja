
#include "solution.h"

#define SOLUTION
#ifndef SOLUTION
// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper)
{
  std::size_t count = 0;
  for (const auto item : input)
  {
    if ((lower <= item.first) && (item.first <= upper))
    {
      output[count++] = item;
    }
  }
  return count;
}
#else
// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper)
{
  std::size_t count = 0;
  for (size_t i = 0; i < input.size(); i++)
  {
    const std::uint32_t cond = (lower <= input[i].first) * (input[i].first <= upper);

    output[count].first = input[i].first * cond;
    output[count].second = input[i].second * cond;

    count += cond;
  }
  return count;
}
#endif