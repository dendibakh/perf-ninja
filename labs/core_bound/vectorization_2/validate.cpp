
#include "solution.hpp"
#include <iostream>

uint16_t original_checksum(const Blob &blob) {
  uint16_t acc = 0;
  for (auto value : blob) {
    acc += value;
    acc += acc < value; // add carry
  }
  return acc;
}

int main() {
  // Init benchmark data
  Blob blob;
  init(blob);

  auto original_result = original_checksum(blob);
  auto result = checksum(blob);

  if (original_result != result) {
    std::cerr << std::hex;
    std::cerr << "Validation Failed. Original result = " << original_result
              << "; Modified version returned = " << result << "\n";
    std::cerr << "; Input:\n[";
    std::cerr << blob[0];
    for (size_t i = 1; i < blob.size(); ++i)
      std::cerr << "," << blob[i];
    std::cerr << "]\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
