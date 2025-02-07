#include <algorithm>
#include <array>
#include <cassert>
#include <type_traits>

#include "solution.hpp"

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  using score_t = int16_t;
  using sequences_t = std::array<score_t, sequence_count_v + 1>;
  using column_seq_t = std::array<std::array<score_t, sequence_count_v + 1>,
                                  sequence_size_v + 1>;

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
  column_seq_t score_column_seq{};
  column_seq_t horizontal_gap_column_seq{};
  sequences_t last_vertical_gap_seq{};

  /*
   * Initialise the first column of the matrix.
   */
  for (size_t i = 0; i < sequence_size_v; ++i) {
    for (size_t j = 0; j < sequence_count_v; ++j) {
      score_column_seq[i][j] = (i > 0 ? gap_open + gap_extension * (i - 1) : 0);
      horizontal_gap_column_seq[i][j] =
          gap_open + (i > 0 ? gap_extension * (i - 1) + gap_open : 0);
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= sequence_size_v; ++col) {
    sequences_t last_diagonal_score =
        score_column_seq[0];  // Cache last diagonal score to compute this cell.
    score_column_seq[0] = horizontal_gap_column_seq[0];
    for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
         ++sequence_idx) {
      last_vertical_gap_seq[sequence_idx] =
          horizontal_gap_column_seq[0][sequence_idx] + gap_open;
      horizontal_gap_column_seq[0][sequence_idx] += gap_extension;
    }

    for (unsigned row = 1; row <= sequence_size_v; ++row) {
      for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
           ++sequence_idx) {
        sequence_t const &sequence1 = sequences1[sequence_idx];
        sequence_t const &sequence2 = sequences2[sequence_idx];
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score[sequence_idx] +
            (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score =
            std::max(best_cell_score, last_vertical_gap_seq[sequence_idx]);
        best_cell_score = std::max(
            best_cell_score, horizontal_gap_column_seq[row][sequence_idx]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[sequence_idx] = score_column_seq[row][sequence_idx];
        score_column_seq[row][sequence_idx] = best_cell_score;
        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap_seq[sequence_idx] += gap_extension;
        horizontal_gap_column_seq[row][sequence_idx] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap_seq[sequence_idx] =
            std::max(last_vertical_gap_seq[sequence_idx], best_cell_score);
        horizontal_gap_column_seq[row][sequence_idx] = std::max(
            horizontal_gap_column_seq[row][sequence_idx], best_cell_score);
      }
    }
  }
  for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
       ++sequence_idx) {
    // Report the best score.
    result[sequence_idx] = score_column_seq.back()[sequence_idx];
  }

  return result;
}
