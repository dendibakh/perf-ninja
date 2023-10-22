#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  using score_t = int16_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;
  using matrix_t = std::array<std::array<score_t, sequence_count_v>, sequence_size_v + 1>;
  /*
   * Initialise score values.
   */
  score_t gap_open{-11};
  score_t gap_extension{-1};
  score_t match{6};
  score_t mismatch{-4};

  const size_t sequences_count = sequences1.size();
  
  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  matrix_t score_column {};
  matrix_t horizontal_gap_column {};
  std::array<score_t, sequence_count_v> last_vertical_gap {};
    
  /*
   * Initialise the first column of the matrix.
   */
  for (size_t seq_id = 0; seq_id < sequences_count; ++seq_id) {
    horizontal_gap_column[0][seq_id] = gap_open;
    last_vertical_gap[seq_id] = gap_open;

    for (size_t i = 1; i <= sequence_size_v; ++i) {
      score_column[i][seq_id] = last_vertical_gap[seq_id];
      horizontal_gap_column[i][seq_id] = last_vertical_gap[seq_id] + gap_open;
      last_vertical_gap[seq_id] += gap_extension;
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= sequence_size_v /*sequence2.size()*/; ++col) {
    std::array<score_t, sequence_count_v> last_diagonal_score {};
    for (size_t seq_id = 0; seq_id < sequences_count; ++seq_id) {
      last_diagonal_score[seq_id] =
          score_column[0][seq_id]; // Cache last diagonal score to compute this cell.
      score_column[0][seq_id] = horizontal_gap_column[0][seq_id];
      last_vertical_gap[seq_id] = horizontal_gap_column[0][seq_id] + gap_open;
      horizontal_gap_column[0][seq_id] += gap_extension;
   }

    for (unsigned row = 1; row <= sequence_size_v /*sequence1.size()*/; ++row) {
      for (size_t seq_id = 0; seq_id < sequences_count; ++seq_id) {
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score[seq_id] +
            (sequences1[seq_id][row - 1] == sequences2[seq_id][col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score = std::max(best_cell_score, last_vertical_gap[seq_id]);
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[row][seq_id]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[seq_id] = score_column[row][seq_id];
        score_column[row][seq_id] = best_cell_score;
        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap[seq_id] += gap_extension;
        horizontal_gap_column[row][seq_id] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[seq_id] = std::max(last_vertical_gap[seq_id], best_cell_score);
        horizontal_gap_column[row][seq_id] =
            std::max(horizontal_gap_column[row][seq_id], best_cell_score);
      }
    }

  }

  return score_column.back();
}
