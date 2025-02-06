#include "solution.hpp"
#include <vector>
#include <algorithm>
#include <iostream>

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

// Task: lookup all the values from l2 in l1.
// For every found value, find the sum of its digits.
// Return the sum of all digits in every found number.
// Both lists have no duplicates and elements placed in *random* order.
// Do NOT sort any of the lists. Do NOT store elements in a hash_map/sets.

// Hint: Traversing a linked list is a long data dependency chain:
//       to get the node N+1 you need to retrieve the node N first.
//       Think how you can execute multiple dependency chains in parallel.
// since we know that it is stored with an arena allocator, we can reinterpret_cast them to arrays so the search isnt dependendt
// then we can store the result into an array rather than the same value, and sum it up at the end
// Well that is not possible since we do not know the exact length
// or we could do as dennis suggestedd as run multiple searches at the same time
unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;

#define SOLUTION_2
#ifdef SOLUTION_1
  const std::array<List, N>& arrayList1 = *reinterpret_cast<std::array<List, N>*>(l1);
  const std::array<List, N>& arrayList2 = *reinterpret_cast<std::array<List, N>*>(l2);
  for(const auto entry : arrayList1)
  {
    if(std::find_if(arrayList2.begin(), arrayList2.end(), [_val =  entry.value](const auto list_head){ return list_head.value == _val;}) != arrayList2.end())
    {
      retVal += getSumOfDigits(entry.value);
    }
  }
#endif
#ifdef SOLUTION_2
  unsigned size = 128; // speed tops out at 128, must mean it moves out of cache
  std::vector<unsigned> values;
  values.reserve(size);
  List *latest_head1 = l1;
  List *head2 = l2;

  while(latest_head1)
  {
    for(int i = 0; i < size; i++)
    {
      values.push_back(latest_head1->value);

      latest_head1 = latest_head1->next;
      if(!latest_head1) break;
    }

    l2 = head2;
    //int uncheck_values = values.size(); // this actually makes it slower
    while (l2) //&& uncheck_values != 0) 
    {
      auto iter = std::find(values.begin(), values.end(), l2->value);
      if (iter != values.end()) 
      {
        //uncheck_values--;
        retVal += getSumOfDigits(l2->value);
      }
      l2 = l2->next;
    }

    values.clear();
  }
#else
  List *head2 = l2;
  // O(N^2) algorithm:
  while (l1) {
    unsigned v = l1->value;
    l2 = head2;
    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }
#endif
  return retVal;
}
