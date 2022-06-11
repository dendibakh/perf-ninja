#include "solution.hpp"
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

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

    /*
     * Initialise the first column of the matrix.
     */
    horizontal_gap_column[0] = gap_open;
    score_t last_vertical_gap = gap_open;

    for (size_t i = 1; i < score_column.size(); ++i) {
      score_column[i] = last_vertical_gap;
      horizontal_gap_column[i] = last_vertical_gap + gap_open;
      last_vertical_gap += gap_extension;
    }

    /*
     * Compute the main recursion to fill the matrix.
     */

    for (unsigned col = 1; col <= sequence2.size(); ++col) {
      column_t old_score_column = score_column;
      score_t last_diagonal_score =
          score_column[0]; // Cache last diagonal score to compute this cell.
      score_column[0] = horizontal_gap_column[0];
      horizontal_gap_column[0] += gap_extension;

      for (unsigned row = 1; row <= sequence1.size(); ++row) {
        score_t best_cell_score = old_score_column[row - 1] + ((sequence1[row - 1] == sequence2[col - 1]) ? match : mismatch);
        score_column[row] = max(best_cell_score, horizontal_gap_column[row]);
      }

      last_vertical_gap = horizontal_gap_column[0] + gap_open;
      for (unsigned row = 1; row <= sequence1.size(); ++row) {
        score_column[row] = max(score_column[row], last_vertical_gap);
        last_vertical_gap = max(last_vertical_gap + gap_extension, score_column[row] + gap_open);
      }

      for (unsigned row = 1; row <= sequence1.size(); ++row) {
        horizontal_gap_column[row] = max(horizontal_gap_column[row] + gap_extension, score_column[row] + gap_open);
      }
    }

    // Report the best score.
    result[sequence_idx] = score_column.back();
  }

  return result;
}
