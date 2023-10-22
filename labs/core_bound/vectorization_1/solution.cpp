#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <iostream>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};
  using score_t = int16_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;
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
  std::vector<column_t> score_column(sequences_count);
  std::vector<column_t> horizontal_gap_column(sequences_count);
  std::vector<score_t> last_vertical_gap(sequences_count);
    
  /*
   * Initialise the first column of the matrix.
   */
  for (size_t seq_id = 0; seq_id < sequences_count; ++seq_id) {
    horizontal_gap_column[seq_id][0] = gap_open;
    last_vertical_gap[seq_id] = gap_open;

    for (size_t i = 1; i < score_column[seq_id].size(); ++i) {
      score_column[seq_id][i] = last_vertical_gap[seq_id];
      horizontal_gap_column[seq_id][i] = last_vertical_gap[seq_id] + gap_open;
      last_vertical_gap[seq_id] += gap_extension;
    }
  }

    //sequence_t const &sequence1 = sequences1[sequence_idx];
    //sequence_t const &sequence2 = sequences2[sequence_idx];

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= sequence_size_v /*sequence2.size()*/; ++col) {
    std::vector<score_t> last_diagonal_score(sequences_count);
    for (size_t seq_id = 0; seq_id < sequences_count; ++seq_id) {
      last_diagonal_score[seq_id] =
          score_column[seq_id][0]; // Cache last diagonal score to compute this cell.
      score_column[seq_id][0] = horizontal_gap_column[seq_id][0];
      last_vertical_gap[seq_id] = horizontal_gap_column[seq_id][0] + gap_open;
      horizontal_gap_column[seq_id][0] += gap_extension;
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
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[seq_id][row]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[seq_id] = score_column[seq_id][row];
        score_column[seq_id][row] = best_cell_score;
        //std::cout << "sq: " << seq_id << ", r: " << row << ", best score: " << best_cell_score << std::endl;
        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap[seq_id] += gap_extension;
        horizontal_gap_column[seq_id][row] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[seq_id] = std::max(last_vertical_gap[seq_id], best_cell_score);
        horizontal_gap_column[seq_id][row] =
            std::max(horizontal_gap_column[seq_id][row], best_cell_score);
      }
    }

  }
    for (size_t seq_id = 0; seq_id < sequences_count; ++seq_id) {
      // Report the best score.
      result[seq_id] = score_column[seq_id].back();
    }

  return result;
}
