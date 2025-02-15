#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  using score_t = int16_t;
  using vec_score_t = std::array<score_t, sequence_count_v>;
  using vec_column_t = std::array<std::array<score_t, sequence_count_v>, sequence_size_v + 1>;

  /*
   * Initialise score values.
   */
  constexpr score_t gap_open{-11};
  constexpr score_t gap_extension{-1};
  constexpr score_t match{6};
  constexpr score_t mismatch{-4};

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  vec_column_t score_column{};
  vec_column_t horizontal_gap_column{};
  vec_score_t last_vertical_gap{};

  /*
   * Initialise the first column of the matrix.
   */
  for (int k = 0; k < sequence_count_v; ++k) {
    horizontal_gap_column[0][k] = gap_open;
    last_vertical_gap[k] = gap_open;
  }

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (int k = 0; k < sequence_count_v; ++k) {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open;
      last_vertical_gap[k] += gap_extension;
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= sequence_size_v; ++col) {
    vec_score_t last_diagonal_score;
    for (int k = 0; k < sequence_count_v; ++k) {
      // Cache last diagonal score to compute this cell.
      last_diagonal_score[k] = score_column[0][k];
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open;
      horizontal_gap_column[0][k] += gap_extension;
    }

    for (unsigned row = 1; row <= sequence_size_v; ++row) {
      for (int k = 0; k < sequence_count_v; ++k) {
        sequence_t const &sequence1 = sequences1[k];
        sequence_t const &sequence2 = sequences2[k];

        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score[k] +
            (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score = std::max(best_cell_score, last_vertical_gap[k]);
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[row][k]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[k] = score_column[row][k];
        score_column[row][k] = best_cell_score;
        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap[k] += gap_extension;
        horizontal_gap_column[row][k] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score);
        horizontal_gap_column[row][k] =
            std::max(horizontal_gap_column[row][k], best_cell_score);
      }
    }
  }

  // Report the best score.
  for (int k = 0; k < sequence_count_v; ++k) {
    result[k] = score_column.back()[k];
  }

  return result;
}
