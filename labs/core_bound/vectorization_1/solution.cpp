#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <iostream>

#define SOLUTION
// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
#ifdef SOLUTION
  result_t result{};
  constexpr size_t bs{8};
  for (size_t sequence_idx = 0; sequence_idx < sequences1.size(); sequence_idx += bs) {
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
    std::array<column_t, bs> score_column{};
    std::array<column_t, bs> horizontal_gap_column{};
    std::array<column_t, bs> vgap{};
    std::array<column_t, bs> best{};

    /*
     * Initialise the first column of the matrix.
     */
    for (size_t b{}; b < bs; ++b) {
      horizontal_gap_column[b][0] = gap_open;
      score_t last_vertical_gap{gap_open};
      for (size_t i = 1; i < sequence_size_v + 1; ++i) {
        score_column[b][i] = last_vertical_gap;
        horizontal_gap_column[b][i] = last_vertical_gap + gap_open;
        last_vertical_gap += gap_extension;
      }
    }

    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence_size_v; ++col) {
      std::array<score_t, bs> score_column_0;

      for (size_t b{}; b < bs; ++b) {
        score_column_0[b] = horizontal_gap_column[b][0];
        vgap[b][0] = horizontal_gap_column[b][0];
        horizontal_gap_column[b][0] += gap_extension;

        for (unsigned row = 1; row <= sequence_size_v; ++row) {
          // Compute next score from diagonal direction with match/mismatch.
          score_t best_cell_score =
              score_column[b][row - 1] +
              (sequences1[sequence_idx + b][row - 1] == sequences2[sequence_idx + b][col - 1] ? match : mismatch);
          // Determine best score from diagonal, vertical, or horizontal
          // direction.
          best[b][row-1] = std::max(best_cell_score, horizontal_gap_column[b][row]);
        }
      }

      for (unsigned row = 1; row <= sequence_size_v; ++row) {
        for (size_t b{}; b < bs; ++b) {
          vgap[b][row] = std::max<score_t>(vgap[b][row - 1] + gap_extension, best[b][row - 1]);
        }
      }


      for (size_t b{}; b < bs; ++b) {
        score_column[b][0] = score_column_0[b];
        //#pragma clang loop vectorize(enable)
        for (unsigned row = 1; row <= sequence_size_v; ++row) {
          score_column[b][row] = std::max<score_t>(best[b][row-1], vgap[b][row - 1] + gap_open);
        }
        for (unsigned row = 1; row <= sequence_size_v; ++row) {
          auto L = std::max<score_t>(best[b][row-1], vgap[b][row] + gap_open);
          horizontal_gap_column[b][row] = std::max<score_t>(horizontal_gap_column[b][row] + gap_extension, L + gap_open);
        }
      }
    }

    // Report the best score.
    for (size_t b{}; b < bs; ++b)
      result[sequence_idx + b] = score_column[b].back();
  }
#else
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
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score +
            (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score = std::max(best_cell_score, last_vertical_gap);
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[row]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score = score_column[row];
        score_column[row] = best_cell_score;
        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap += gap_extension;
        horizontal_gap_column[row] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap = std::max(last_vertical_gap, best_cell_score);
        horizontal_gap_column[row] =
            std::max(horizontal_gap_column[row], best_cell_score);
      }
    }

    // Report the best score.
    result[sequence_idx] = score_column.back();
  }
#endif
  return result;
}
