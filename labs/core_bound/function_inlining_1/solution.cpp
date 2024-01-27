
#include "solution.h"
#include <algorithm>

#ifdef STRUCT
struct Comp {
  bool operator() (const S& a, const S& b) const { 
    return a.key1 < b.key1 ? true :
       a.key1 > b.key1 ? false :
       a.key2 < b.key2;
  }
};
#endif

void solution(std::array<S, N> &arr) {
#ifdef STRUCT
  std::sort(arr.begin(), arr.end(), Comp());
#else
  auto lc = [](const S& a, const S& b){
     return a.key1 < b.key1 ? true :
       a.key1 > b.key1 ? false :
       a.key2 < b.key2;
  };
  std::sort(arr.begin(), arr.end(), lc);
#endif
}
