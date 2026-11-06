#include "arena.hpp"
#include <array>
#include <vector>

constexpr unsigned N = 10000;
using RandomRangeT = unsigned short;

struct List {
  List *next;
  unsigned value;
};

using ArenaListAllocator = Arena<List, sizeof(List) * N>;
List *getRandomList(ArenaListAllocator &allocator);
void printList(List *l);

unsigned getSumOfDigits(unsigned n);

unsigned solution_baseline(List *l1, List *l2);

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1<2>        39.1 ms         39.1 ms           17
// bench1<4>        22.9 ms         22.9 ms           33
// bench1<8>        14.6 ms         14.6 ms           48
// bench1<16>       10.7 ms         10.7 ms           63 <----- WINNER
// bench1<32>       36.7 ms         36.7 ms           18
// bench1<64>       30.4 ms         30.4 ms           23
template <size_t group_size>
unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;

  List *head_l1 = l1;
  List *head_l2 = l2;

  int length_l1 = 0;
  while(l1){
    length_l1++;
    l1 = l1->next;
  }

  l1 = head_l1;

  // Check values in l2 for group of group_size size;
  for(int i = 0; i < length_l1/group_size; i++)
  {
    // Get values for group
    std::array<unsigned, group_size> group_vals; 
    for(int j = 0; j < group_size; j++)
    {
      group_vals[j] = l1->value;
      l1 = l1->next;
    }

    // Go throught l2 and check for all values in the group
    int found_count = 0;
    l2 = head_l2;
    while (l2) 
    {
      for(int j = 0; j < group_size; j++)
      {
        if(group_vals[j] == l2->value){
          retVal += getSumOfDigits(group_vals[j]);

          if(++found_count == group_size){
            break;
          }
        }
      }
      l2 = l2->next;
    }
  }

  // Process reminder Values
  while (l1) {
    unsigned v = l1->value;
    l2 = head_l2;

    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }

  return retVal;
}