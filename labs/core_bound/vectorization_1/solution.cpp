#include "solution.hpp"
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};


  /*
  * Initialise score values.
  */
  using score_t = int16_t;
  using score_group_t = std::array<score_t, sequence_count_v>;
  using column_group_t = std::array<std::array<score_t, sequence_count_v>, sequence_size_v + 1>;

  constexpr score_t gap_open{-11};
  constexpr score_t gap_extension{-1};
  constexpr score_t match{6};
  constexpr score_t mismatch{-4};

  column_group_t score_column{};
  column_group_t horizontal_gap_column{};
  score_group_t last_vertical_gap{};

  for (size_t k = 0; k < sequence_count_v; k++) {
    horizontal_gap_column[0][k] = gap_open;
    last_vertical_gap[k] = gap_open;

    for (size_t i = 1; i < score_column.size(); ++i) {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open;
      last_vertical_gap[k] += gap_extension;
    }
  }

  score_group_t last_diagonal_score{};
  score_group_t best_cell_score{};
  for (unsigned col = 1; col <= sequence_size_v; ++col) {
    for (size_t k = 0; k < sequence_count_v; k++) {
      last_diagonal_score[k] =
          score_column[0][k]; // Cache last diagonal score to compute this cell.
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open;
      horizontal_gap_column[0][k] += gap_extension;
    }

    for (unsigned row = 1; row <= sequence_size_v; ++row) {
      for (size_t k = 0; k < sequence_count_v; k++) {
        sequence_t const &sequence1 = sequences1[k];
        sequence_t const &sequence2 = sequences2[k];
        // Compute next score from diagonal direction with match/mismatch.
        best_cell_score[k] =
            last_diagonal_score[k] +
            (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[k] = max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = max(best_cell_score[k], horizontal_gap_column[row][k]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[k] = score_column[row][k];
        score_column[row][k] = best_cell_score[k];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[k] += gap_open;
        last_vertical_gap[k] += gap_extension;
        horizontal_gap_column[row][k] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[row][k] =
            max(horizontal_gap_column[row][k], best_cell_score[k]);
      }
    }

    // Report the best score.
    for (size_t k = 0; k < sequence_count_v; k++) {
      result[k] = score_column.back()[k];
    }
  }

  return result;
}
