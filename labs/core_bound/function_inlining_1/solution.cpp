
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

void solution(std::array<S, N> &arr) 
{
    struct 
    {
        bool operator()(S& a, S& b) const 
        {
          if (a.key1 < b.key1)
            return true;
          
          if (a.key1 > b.key1)
            return false;
          
          if (a.key2 < b.key2)
            return true;

          if (a.key2 > b.key2)
            return false;

          return false;
        }
    }
    customCompare;

  std::sort(arr.begin(), arr.end(), customCompare);
}
