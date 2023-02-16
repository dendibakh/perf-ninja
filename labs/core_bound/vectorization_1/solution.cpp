#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
#ifdef SOLUTION
  result_t result{};
  constexpr int batch_size_v = 8;
  using score_t = int16_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;
  constexpr score_t gap_open{-11};
  constexpr score_t gap_extension{-1};
  constexpr score_t match{6};
  constexpr score_t mismatch{-4};
  assert(sequences1.size() % batch_size_v == 0);

  for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
       sequence_idx += batch_size_v) {
    std::array<column_t, batch_size_v> score_column{};
    std::array<column_t, batch_size_v> horizontal_gap_column{};
    std::array<score_t,  batch_size_v> last_vertical_gap{};

    /*
     * Initialise the first column of the matrix.
     */
    for (size_t b{}; b < batch_size_v; ++b) {
      horizontal_gap_column[b][0] = gap_open;
      last_vertical_gap[b] = gap_open;
      for (size_t i = 1; i < sequence_size_v + 1; ++i) {
        score_column[b][i] = last_vertical_gap[b];
        horizontal_gap_column[b][i] = last_vertical_gap[b] + gap_open;
        last_vertical_gap[b] += gap_extension;
      }
    }

    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence_size_v; ++col) {
      std::array<score_t, batch_size_v> last_diagonal_score;
      for (size_t b{}; b < batch_size_v; ++b) {
        last_diagonal_score[b] = score_column[b][0];
        score_column[b][0] = horizontal_gap_column[b][0];
        last_vertical_gap[b] = horizontal_gap_column[b][0] + gap_open;
        horizontal_gap_column[b][0] += gap_extension;
      }

      for (unsigned row = 1; row <= sequence_size_v; ++row) {
        // Compute next score from diagonal direction with match/mismatch.
        std::array<score_t, batch_size_v> best_cell_score;
        #pragma clang loop vectorize(enable)
        for (size_t b{}; b < batch_size_v; ++b) {
          sequence_t const &sequence1 = sequences1[sequence_idx + b];
          sequence_t const &sequence2 = sequences2[sequence_idx + b];
          best_cell_score[b] = last_diagonal_score[b] +
              (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
          // Determine best score from diagonal, vertical, or horizontal
          // direction.
          best_cell_score[b] = std::max(best_cell_score[b], last_vertical_gap[b]);
          best_cell_score[b] = std::max(best_cell_score[b], horizontal_gap_column[b][row]);
          // Cache next diagonal value and store optimum in score_column.
          last_diagonal_score[b] = score_column[b][row];
          score_column[b][row] = best_cell_score[b];
          // Compute the next values for vertical and horizontal gap.
          best_cell_score[b] += gap_open;
          last_vertical_gap[b] += gap_extension;
          horizontal_gap_column[b][row] += gap_extension;
          // Store optimum between gap open and gap extension.
          last_vertical_gap[b] = std::max(last_vertical_gap[b], best_cell_score[b]);
          horizontal_gap_column[b][row] =
              std::max(horizontal_gap_column[b][row], best_cell_score[b]);
        }
      }
    }

    // Report the best score.
    for (size_t b{}; b < batch_size_v; ++b)
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
