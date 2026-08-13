#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};
  std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v> tsequences1{};
  std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v> tsequences2{};
  for (size_t sequence_idx = 0; sequence_idx < sequences1.size(); ++sequence_idx) {
    for (size_t j = 0; j < sequences1[sequence_idx].size(); j++) {
      tsequences1[j][sequence_idx] = sequences1[sequence_idx][j];
      tsequences2[j][sequence_idx] = sequences2[sequence_idx][j];
    }
  }

  using score_t = int16_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;
  using seq_t = std::array<score_t, sequence_count_v>;
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
    std::array<seq_t, sequence_size_v + 1> score_column{};
    std::array<seq_t, sequence_size_v + 1> diagonal_score_column{};
    std::array<seq_t, sequence_size_v + 1> horizontal_gap_column{};
    std::array<seq_t, sequence_size_v + 1> last_vertical_gap_column{};
    std::array<seq_t, sequence_size_v + 1> last_vertical_gap{};

    /*
     * Initialise the first column of the matrix.
     */
    for (size_t xs = 0; xs < sequence_count_v; ++xs) {
      horizontal_gap_column[0][xs] = gap_open;
      last_vertical_gap[0][xs] = gap_open;
      for (size_t i = 1; i < sequence_size_v + 1; ++i) {
        score_column[i][xs] = last_vertical_gap[i - 1][xs];
        horizontal_gap_column[i][xs] = last_vertical_gap[i - 1][xs] + gap_open;
        last_vertical_gap[i][xs] = last_vertical_gap[i - 1][xs] + gap_extension;
      }

    }


    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence_size_v; ++col) {
      for (size_t xs = 0; xs < sequence_count_v; ++xs) {
        diagonal_score_column[0][xs] =
            score_column[0][xs];
        score_column[0][xs] = horizontal_gap_column[0][xs];
        last_vertical_gap[0][xs] = horizontal_gap_column[0][xs] + gap_open;
        horizontal_gap_column[0][xs] += gap_extension;
      }

      for (unsigned row = 1; row <= sequence_size_v; ++row) {
        // Compute next score from diagonal direction with match/mismatch.
        for (size_t sequence_idx = 0; sequence_idx < sequence_count_v; ++sequence_idx) {
          score_t best_cell_score =
              diagonal_score_column[row - 1][sequence_idx] +
              match * (tsequences1[row - 1][sequence_idx] == tsequences2[col - 1][sequence_idx]) + mismatch * (tsequences1[row - 1][sequence_idx] != tsequences2[col - 1][sequence_idx]);
          // Determine best score from diagonal, vertical, or horizontal
          // direction.
          best_cell_score = std::max(best_cell_score, last_vertical_gap[row - 1][sequence_idx]);
          best_cell_score = std::max(best_cell_score, horizontal_gap_column[row][sequence_idx]);
          // Cache next diagonal value and store optimum in score_column.
          diagonal_score_column[row][sequence_idx] = score_column[row][sequence_idx];
          score_column[row][sequence_idx] = best_cell_score;
          // Compute the next values for vertical and horizontal gap.
          best_cell_score += gap_open;
          last_vertical_gap[row - 1][sequence_idx] += gap_extension;
          horizontal_gap_column[row][sequence_idx] += gap_extension;
          // Store optimum between gap open and gap extension.
          last_vertical_gap[row][sequence_idx] = std::max(last_vertical_gap[row - 1][sequence_idx], best_cell_score);
          horizontal_gap_column[row][sequence_idx] =
              std::max(horizontal_gap_column[row][sequence_idx], best_cell_score);
      }
    }
  }
  // Report the best score.
  for (size_t sequence_idx = 0; sequence_idx < sequence_count_v; ++sequence_idx) {
    result[sequence_idx] = score_column[sequence_size_v][sequence_idx];
  }

  return result;
}
