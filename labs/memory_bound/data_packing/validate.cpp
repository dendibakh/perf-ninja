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

  if (entry.i != first) {
    reportError("i", entry.i, first, first, second);
    isValid = false;
  }

  if (entry.s != second) {
    reportError("s", entry.s, second, first, second);
    isValid = false;
  }

  const auto expected_l = static_cast<short>(first * second);
  if (entry.l != expected_l) {
    reportError("l", entry.l, expected_l, first, second);
    isValid = false;
  }
  
  const auto expected_d = static_cast<double>(first) / maxRandom;
  if (std::abs(float(entry.d - expected_d)) > 0.001) {
    reportError("d", entry.d, expected_d, first, second);
    isValid = false;
  }

  const auto expected_b = (first < second);
  if (entry.b != expected_b) {
    reportError("b", entry.b, expected_b, first, second);
    isValid = false;
  }

  return isValid;
}

std::ostream& operator<<(std::ostream& os, const S& s) {
  os << "{ i: " << s.i << ", s: " << s.s << ", l: " << s.l << ", d: " << s.d << ", b: " << s.b << " }";
  return os;
}

int main() {
  std::vector<S> arr(N);
  init(arr);

  auto expected = arr;
  solution(arr);
  if (!std::is_sorted(arr.begin(), arr.end())) {
    std::cerr << "Validation Failed. Array is not properly sorted." << std::endl;
    return 1;
  }
  auto cmp_eq = [](const S a, const S b) {
    return std::tie(a.i, a.s, a.l, a.d, a.b) == std::tie(b.i, b.s, b.l, b.d, b.b);
  };
  auto cmp_less = [](const S a, const S b) {
    return std::tie(a.i, a.s, a.l, a.d, a.b) < std::tie(b.i, b.s, b.l, b.d, b.b);
  };
  std::sort(expected.begin(), expected.end(), cmp_less);
  std::sort(arr.begin(), arr.end(), cmp_less);

  for (int i = 0; i < N; i++) {
    if (!cmp_eq(arr[i], expected[i])) {
      std::cerr << "Validation Failed. Result[" << i << "] = " << arr[i]
                << ". Expected[" << i << "] = " << expected[i] << std::endl;
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
