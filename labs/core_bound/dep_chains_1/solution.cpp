#include "solution.hpp"
#include <array>
#include <iostream>

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

template <const size_t ParallelSearches>
class parallel_search {
  unsigned work_one(List *l, unsigned v) {
    while (l) {
      if (l->value == v) {
        return getSumOfDigits(v);
      }
      l = l->next;
    }
    return 0;
  }

  unsigned work_arr(List *l, const std::array<unsigned, ParallelSearches>& cur_chank) {
    unsigned retVal = 0;
    while (l) {
      for (auto& v : cur_chank) {
          if (l->value == v) {
            retVal += getSumOfDigits(v);
          }
        }
      l = l->next;
    }
    return retVal;
  }

  std::array<unsigned, ParallelSearches> get_chunk(List*& l) {
    std::array<unsigned, ParallelSearches> res;
    for (auto& v : res) {
      v = l->value;
      l = l->next;
    }
    return res;
  }

public:
  unsigned operator() (List *l1, List *l2)
  {
    unsigned retVal = 0;
    size_t l1_len = 0;
    for (List * cur = l1; cur; cur = cur->next) {
      ++l1_len;
    }
    const size_t chunks_count = l1_len / ParallelSearches;
    for (size_t i = 0; i < chunks_count; ++i) {
      auto cur_chank = get_chunk(l1);
      retVal += work_arr(l2, cur_chank);
    }
    while (l1) {
      retVal += work_one(l2, l1->value);
      l1 = l1->next;
    }
    return retVal;
  }
};

unsigned solution(List *l1, List *l2) {
  return parallel_search<8>{}(l1, l2);
}