#include "solution.hpp"
#include <iostream>

static bool equals(const result_t &result, const result_t &expected) {
  size_t errors{};
  for (size_t i = 0; i < expected.size(); ++i) {
    if (expected[i] != result[i]) {
      ++errors;
      std::cerr << "Result[" << i << "] = " << result[i] << " Expected[" << i
                << "] = " << expected[i] << std::endl;
    }
  }
  return 0 == errors;
}

int main() {
  auto [sequences1, sequences2] = init();

  auto computed_result = compute_alignment(sequences1, sequences2);

  result_t expected_result{};
  for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
       ++sequence_idx) {
    using score_t = int16_t;
    using column_t = std::array<score_t, sequence_size_v + 1>;

    sequence_t const &sequence1 = sequences1[sequence_idx];
    sequence_t const &sequence2 = sequences2[sequence_idx];

    /*
     * Initialise score values.
     */
    score_t gap_open{-11};
    score_t gap_extension{-1};
    score_t match{6};
    score_t mismatch{-4};

    /*
     * Setup the matrix.
     * Note we can compute the entire matrix with just one column in memory,
     * since we are only interested in the last value of the last column in the
     * score matrix.
     */
    column_t score_column{};
    column_t horizontal_gap_column{};
    score_t last_vertical_gap{};

    /*
     * Initialise the first column of the matrix.
     */
    horizontal_gap_column[0] = gap_open;
    last_vertical_gap = gap_open;

    for (size_t i = 1; i < score_column.size(); ++i) {
      score_column[i] = last_vertical_gap;
      horizontal_gap_column[i] = last_vertical_gap + gap_open;
      last_vertical_gap += gap_extension;
    }

    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence2.size(); ++col) {
      score_t last_diagonal_score =
          score_column[0]; // Cache last diagonal score to compute this cell.
      score_column[0] = horizontal_gap_column[0];
      last_vertical_gap = horizontal_gap_column[0] + gap_open;
      horizontal_gap_column[0] += gap_extension;

      for (unsigned row = 1; row <= sequence1.size(); ++row) {
        score_t best_cell_score =
            last_diagonal_score +
            (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);

        best_cell_score = std::max(best_cell_score, last_vertical_gap);
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[row]);
        last_diagonal_score = score_column[row];
        score_column[row] = best_cell_score;

        best_cell_score += gap_open;
        last_vertical_gap += gap_extension;
        horizontal_gap_column[row] += gap_extension;

        last_vertical_gap = std::max(last_vertical_gap, best_cell_score);
        horizontal_gap_column[row] =
            std::max(horizontal_gap_column[row], best_cell_score);
      }
    }

    // Report the best score.
    expected_result[sequence_idx] = score_column.back();
  }

  if (!equals(computed_result, expected_result)) {
    std::cerr << "Validation Failed." << std::endl;
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
