#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  assert(sequences1.size() == sequences2.size() && sequences1.size() == sequence_count_v);

  std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v> transp1;
  std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v> transp2;

  for (unsigned col = 0; col < sequence_size_v; ++col) {
    for (size_t sequence_idx = 0; sequence_idx < sequence_count_v;
         ++sequence_idx) {
      transp1[col][sequence_idx] = sequences1[sequence_idx][col];
      transp2[col][sequence_idx] = sequences2[sequence_idx][col];
    }
  }

  using score_t = int16_t;
  /*
   * Initialise score values.
   */
  score_t match{6};
  score_t mismatch{-4};
  score_t gap_open{-11};
  score_t gap_extension{-1};

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  std::array<std::array<score_t, sequence_count_v>, sequence_size_v + 1> score_column;
  std::array<std::array<score_t, sequence_count_v>, sequence_size_v + 1> horizontal_gap_column;
  score_t last_vertical_gap[sequence_count_v];

  for (size_t sequence_idx = 0; sequence_idx < sequence_count_v;
       ++sequence_idx) {
    /*
     * Initialise the first column of the matrix.
     */
    score_column[0][sequence_idx] = 0;
    horizontal_gap_column[0][sequence_idx] = gap_open;
    last_vertical_gap[sequence_idx] = gap_open;
  }

  for (size_t i = 1; i <= sequence_size_v; ++i) {
    for (size_t sequence_idx = 0; sequence_idx < sequence_count_v;
         ++sequence_idx) {
      score_column[i][sequence_idx] = last_vertical_gap[sequence_idx];
      horizontal_gap_column[i][sequence_idx] = last_vertical_gap[sequence_idx] + gap_open;
      last_vertical_gap[sequence_idx] += gap_extension;
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= sequence_size_v; ++col) {
    score_t last_diagonal_score[sequence_count_v];
    for (size_t sequence_idx = 0; sequence_idx < sequence_count_v;
         ++sequence_idx) {
      last_diagonal_score[sequence_idx] = score_column[0][sequence_idx]; // Cache last diagonal score to compute this cell.
      score_column[0][sequence_idx] = horizontal_gap_column[0][sequence_idx];
      last_vertical_gap[sequence_idx] = horizontal_gap_column[0][sequence_idx] + gap_open;
      horizontal_gap_column[0][sequence_idx] += gap_extension;
    }

    for (unsigned row = 1; row <= sequence_size_v; ++row) {
      for (size_t sequence_idx = 0; sequence_idx < sequence_count_v;
           ++sequence_idx) {
      // Compute next score from diagonal direction with match/mismatch.
      score_t best_cell_score =
          last_diagonal_score[sequence_idx] +
          (transp1[row - 1][sequence_idx] == transp2[col - 1][sequence_idx] ? match : mismatch);
      // Determine best score from diagonal, vertical, or horizontal
      // direction.
      best_cell_score = std::max(best_cell_score, last_vertical_gap[sequence_idx]);
      best_cell_score = std::max(best_cell_score, horizontal_gap_column[row][sequence_idx]);
      // Cache next diagonal value and store optimum in score_column.
      last_diagonal_score[sequence_idx] = score_column[row][sequence_idx];
      score_column[row][sequence_idx] = best_cell_score;
      // Compute the next values for vertical and horizontal gap.
      best_cell_score += gap_open;
      last_vertical_gap[sequence_idx] += gap_extension;
      horizontal_gap_column[row][sequence_idx] += gap_extension;
      // Store optimum between gap open and gap extension.
      last_vertical_gap[sequence_idx] = std::max(last_vertical_gap[sequence_idx], best_cell_score);
      horizontal_gap_column[row][sequence_idx] =
          std::max(horizontal_gap_column[row][sequence_idx], best_cell_score);
      }
    }
  }

  for (size_t sequence_idx = 0; sequence_idx < sequence_count_v;
       ++sequence_idx) {
    // Report the best score.
    result[sequence_idx] = score_column.back()[sequence_idx];
  }

  return result;
}
