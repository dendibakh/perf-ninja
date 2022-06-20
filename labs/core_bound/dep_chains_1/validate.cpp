
#include "solution.hpp"
#include <iostream>
#include <memory>

unsigned original_solution(List *l1, List *l2) {
  unsigned retVal = 0;

  List *head2 = l2;
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

  return retVal;
}

int main() {
  // Init benchmark data
  auto arena1 = ArenaListAllocator{};
  auto l1 = getRandomList(arena1);
  auto arena2 = ArenaListAllocator{};
  auto l2 = getRandomList(arena2);

  auto original_result = original_solution(l1, l2);
  auto result = solution(l1, l2);

  if (original_result != result) {
    std::cerr << "Validation Failed. Original result = " << original_result
              << "; Modified version returned = " << result << "\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
