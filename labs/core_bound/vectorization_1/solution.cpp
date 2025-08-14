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
  using column_t = std::array<score_t, sequence_size_v + 1>;

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
  column_t score_column[sequence_count_v]{};
  column_t horizontal_gap_column[sequence_count_v]{};
  score_t last_vertical_gap[sequence_count_v]{};

  /*
    * Initialise the first column of the matrix.
    */
  for (size_t a = 0; a < sequence_count_v; ++a) {
    horizontal_gap_column[a][0] = gap_open;
    last_vertical_gap[a] = gap_open;
  }

  for (size_t i = 1; i <= sequence_size_v; ++i) {
    for (size_t a = 0; a < sequence_count_v; ++a) {
      score_column[a][i] = last_vertical_gap[a];
      horizontal_gap_column[a][i] = last_vertical_gap[a] + gap_open;
      last_vertical_gap[a] += gap_extension;
    }
  }

  /*
    * Compute the main recursion to fill the matrix.
    */
  for (unsigned col = 1; col <= sequence_size_v; ++col) {
    score_t last_diagonal_score[sequence_count_v];
    for (size_t a = 0; a < sequence_count_v; ++a) {
      last_diagonal_score[a] =
          score_column[a][0]; // Cache last diagonal score to compute this cell.
      score_column[a][0] = horizontal_gap_column[a][0];
      last_vertical_gap[a] = horizontal_gap_column[a][0] + gap_open;
      horizontal_gap_column[a][0] += gap_extension;
    }

    for (unsigned row = 1; row <= sequence_size_v; ++row) {
      for (size_t a = 0; a < sequence_count_v; ++a) {
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score[a] +
            (sequences1[a][row - 1] == sequences2[a][col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score = std::max(best_cell_score, last_vertical_gap[a]);
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[a][row]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[a] = score_column[a][row];
        score_column[a][row] = best_cell_score;
        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap[a] += gap_extension;
        horizontal_gap_column[a][row] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[a] = std::max(last_vertical_gap[a], best_cell_score);
        horizontal_gap_column[a][row] =
            std::max(horizontal_gap_column[a][row], best_cell_score);
      }
    }
  }

  for (size_t a = 0; a < sequence_count_v; ++a) {
    // Report the best score.
    result[a] = score_column[a].back();
  }

  return result;
}
