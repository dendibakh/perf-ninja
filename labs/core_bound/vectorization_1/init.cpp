#include "solution.hpp"
#include <algorithm>
#include <random>

// Initialises a pair of sequence collections given a fixed sequence size.
std::pair<std::vector<sequence_t>, std::vector<sequence_t>> init() {
  std::mt19937_64 random_engine{};
  // Simulate DNA alphabet with 4 symbols.
  std::uniform_int_distribution<uint16_t> symbol_distribution(0u, 4u);

  auto generate_sequences = [&]() -> std::vector<sequence_t> {
    std::vector<sequence_t> sequences{};
    sequences.resize(sequence_count_v);

    for (sequence_t &sequence : sequences) {
      std::generate(sequence.begin(), sequence.end(),
                    [&]()
                    { return static_cast<uint8_t>(symbol_distribution(random_engine)); });
    }
    return sequences;
  };

  return std::pair{generate_sequences(), generate_sequences()};
}
