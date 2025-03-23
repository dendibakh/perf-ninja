
#include "solution.hpp"
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <vector>


Position<std::uint32_t> original_solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  for (auto pos: input) {
    x += pos.x;
    y += pos.y;
    z += pos.z;
  }

  return {
          static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
  };
}

// adds 1000 random inputs with lengths from 1 to 1000
void addSmallRandomInputs(std::vector<std::vector<Position<std::uint32_t>>> &inputs) {
  std::vector<Position<std::uint32_t>> input;
  std::default_random_engine eng{};
  std::uniform_int_distribution<std::uint32_t> distr;
  for (std::size_t i = 0; i < 1000; ++i) {
    input.push_back(Position<std::uint32_t>{distr(eng), distr(eng), distr(eng)});
    inputs.push_back(input);
  }
}

int main(int argc, char **argv) {
  // feel free to comment out tests for debugging
  const std::uint32_t biggest_coord = std::numeric_limits<std::uint32_t>::max();

  const Position<std::uint32_t> biggest_possible_position = {biggest_coord, biggest_coord, biggest_coord};

  std::vector<std::vector<Position<std::uint32_t>>> inputs = {
          {}, // empty vector
          {{0, 0, 0}},
          {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}},
          {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}, {5, 5, 5}, {6, 6, 6}, {7, 7, 7}, {8, 8, 8}, {9, 9, 9}, {10, 10, 10}, {11, 11, 11}, {12, 12, 12}, {13, 13, 13}, {14, 14, 14}, {15, 15, 15}, {16, 16, 16}},
          {biggest_possible_position, biggest_possible_position}, // make changing the accumulators to u32 overflow
          {biggest_possible_position, {1, 1, 1}},
  };

  addSmallRandomInputs(inputs);

  bool valid = true;
  for (auto &input: inputs) {
    auto original_result = original_solution(input);
    auto result = solution(input);

    if (original_result != result) {
      std::stringstream iss;
      iss << '{';
      bool first = true;
      for (auto pos: input) {
        if (!first) iss << ", ";
        iss << pos;
        first = false;
      }
      std::cerr << "Validation Failed on test input: " << iss.str()
                << ". Original result = " << original_result
                << "; Modified version returned = " << result << "\n";
      valid = false;
    }
  }

  if (!valid)
    return 1;

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
