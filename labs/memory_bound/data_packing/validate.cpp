
#include "solution.h"
#include <algorithm>
#include <iostream>

template <typename Received_t, typename Expected_t>
static void report_failure(const char *var_name, Received_t received,
                           Expected_t expected, int first_value,
                           int second_value) {
  std::cerr << "Validation Failed. Value " << var_name << " is " << received
            << ". Expected is " << expected << " for intialization values "
            << first_value << " and " << second_value << std::endl;
}

bool check_entry(int first_value, int second_value) {
  S entry = create_entry(first_value, second_value);

  if (entry.i != first_value) {
    report_failure("i", entry.i, first_value, first_value, second_value);
    return false;
  }

  if (entry.s != second_value) {
    report_failure("s", entry.s, second_value, first_value, second_value);
    return false;
  }

  const auto expected_l = static_cast<short>(first_value * second_value);
  if (entry.get_l() != expected_l) {
    report_failure("l", entry.get_l(), expected_l, first_value, second_value);
    return false;
  }

  const auto expected_d = static_cast<double>(first_value) / maxRandom;
  if (entry.get_d() != expected_d) {
    report_failure("d", entry.get_d(), expected_d, first_value, second_value);
    return false;
  }

  const auto expected_b = (first_value < second_value);
  if (entry.get_b() != expected_b) {
    report_failure("b", entry.get_b(), expected_b, first_value, second_value);
    return false;
  }

  return true;
}

int main() {
  std::array<S, N> arr;
  init(arr);

  auto expected = arr;
  solution(arr);
  std::sort(expected.begin(), expected.end());

  for (int i = 0; i < N; i++) {
    // we only check i components since sort can be unstable
    if (arr[i].i != expected[i].i) {
      std::cerr << "Validation Failed. Result[" << i << "].i = " << arr[i].i
                << ". Expected[" << i << "].i = " << expected[i].i << std::endl;
      return 1;
    }
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
