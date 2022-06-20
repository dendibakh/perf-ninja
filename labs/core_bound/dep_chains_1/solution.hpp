#include "arena.hpp"
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
unsigned solution(List *l1, List *l2);