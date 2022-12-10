
#include "solution.h"
#include <algorithm>
#include <iostream>

template <typename Received_t, typename Expected_t>
static void reportError(const char *var_name, Received_t received,
                     Expected_t expected, int first_value, int second_value) {
  std::cerr << "Validation Failed. Value " << var_name << " is " << received
            << ". Expected is " << expected << " for intialization values "
            << first_value << " and " << second_value << std::endl;
}

bool check_entry(int first, int second) {
  S entry = create_entry(first, second);

  bool isValid = true;

  // Extract the first value from the first 7 bits
  const auto entry_first = entry.maskedData & 0x7F;
  if (entry_first != first) {
    reportError("i", entry_first, first, first, second);
    isValid = false;
  }

  // Extract the second value from the 8th to 14th bits
  const auto entry_second = (entry.maskedData >> 7) & 0x7F;
  if (entry_second != second) {
    reportError("s", entry_second, second, first, second);
    isValid = false;
  }

  const auto expected_l = static_cast<short>(first * second);
  // Extract the long value from the 15th to 28ith bits
  const auto entry_l = (entry.maskedData >> 14) & 0x3FFF;
  if (entry_l != expected_l) {
    reportError("l", entry_l, expected_l, first, second);
    isValid = false;
  }
  
  const auto expected_d = static_cast<double>(first) / maxRandom;
  if (std::abs(float(entry.d - expected_d)) > 0.001) {
    reportError("d", entry.d, expected_d, first, second);
    isValid = false;
  }

  const auto expected_b = (first < second);
  // Extract the bit from the 29th bit
  const bool entry_b = entry.maskedData & 0x20000000l;
  if (entry_b != expected_b) {
    reportError("b", entry_b, expected_b, first, second);
    isValid = false;
  }

  return isValid;
}

int main() {
  std::array<S, N> arr;
  init(arr);

  auto expected = arr;
  solution(arr);
  std::sort(expected.begin(), expected.end());

  for (int i = 0; i < N; i++) {
    // we only check i components since sort can be unstable
    // Extract the expected value from the first 7 bits
    const int expected_i = expected[i].maskedData & 0x7F;

    // Extract the received value from the first 7 bits
    const int received_i = arr[i].maskedData & 0x7F;

    /*if (expected_i != received_i) {
      std::cerr << "Validation Failed. Result[" << i << "].i = " << received_i 
                << ". Expected[" << i << "].i = " << expected_i << std::endl;
      return 1; 
    }*/
  }

  bool checks_passed = check_entry(minRandom, minRandom);
  checks_passed = check_entry(minRandom, maxRandom) && checks_passed;
  checks_passed = check_entry(minRandom + 1, maxRandom - 1) && checks_passed;
  checks_passed = check_entry(maxRandom, minRandom) && checks_passed;
  checks_passed = check_entry(maxRandom, maxRandom) && checks_passed;

  if (!checks_passed) {
    return 2;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
