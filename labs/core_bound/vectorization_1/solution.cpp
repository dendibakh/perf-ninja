#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <iostream>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment_base(std::vector<sequence_t> const &sequences1,
                                std::vector<sequence_t> const &sequences2)
{
  result_t result{};

  for (size_t sequence_idx = 0; sequence_idx < sequences1.size(); ++sequence_idx) {
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
      score_t last_diagonal_score = score_column[0]; // Cache last diagonal score to compute this cell.
      score_column[0] = horizontal_gap_column[0];
      last_vertical_gap = horizontal_gap_column[0] + gap_open;
      horizontal_gap_column[0] += gap_extension;

      for (unsigned row = 1; row <= sequence1.size(); ++row) {
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score = last_diagonal_score + (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
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
        horizontal_gap_column[row] = std::max(horizontal_gap_column[row], best_cell_score);
      }
    }

    // Report the best score.
    result[sequence_idx] = score_column.back();
  }

  return result;
}

namespace simd {

using score_t = std::array<int16_t, sequence_count_v>;
using column_t = std::array<score_t, sequence_size_v + 1>;
using simd_seq_t = std::array<score_t, sequence_size_v>;

simd_seq_t transpose(std::vector<sequence_t> const & sequences) {
  simd_seq_t ans{};
  for (size_t i = 0; i < ans.size(); ++i)
    for (size_t j = 0; j < sequences.size(); ++j) {
      ans[i][j] = sequences[j][i];
    }
  return ans;
}

result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
  result_t result{};

  score_t gap_open; gap_open.fill(-11);
  score_t gap_extension; gap_extension.fill(-1);
  score_t match; match.fill(6);
  score_t mismatch; mismatch.fill(-4);

  column_t score_column{};
  column_t horizontal_gap_column{};
  score_t last_vertical_gap{};

  auto seq1 = transpose(sequences1);
  auto seq2 = transpose(sequences2);

  horizontal_gap_column[0] = gap_open;
  last_vertical_gap = gap_open;

  constexpr size_t n_seq = sequence_count_v;

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t k = 0; k < n_seq; ++k) {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
      last_vertical_gap[k] += gap_extension[k];
    }
  }

  for (unsigned col = 1; col <= seq2.size(); ++col) {
   score_t last_diagonal_score = score_column[0]; // Cache last diagonal score to compute this cell.
   for (size_t k = 0; k < n_seq; ++k) {
     score_column[0][k] = horizontal_gap_column[0][k];
     last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
     horizontal_gap_column[0][k] += gap_extension[k];
   }

   for (unsigned row = 1; row <= seq1.size(); ++row) {
     // score_t best_cell_score;
     for (size_t k = 0; k < n_seq; ++k) {
       int16_t best_cell_score = last_diagonal_score[k] + (seq1[row - 1][k] == seq2[col - 1][k] ? match[k] : mismatch[k]);
       // Determine best score from diagonal, vertical, or horizontal
       // direction.
       best_cell_score = std::max(best_cell_score, last_vertical_gap[k]);
       best_cell_score = std::max(best_cell_score, horizontal_gap_column[row][k]);
       // Cache next diagonal value and store optimum in score_column.
       last_diagonal_score[k] = score_column[row][k];
       score_column[row][k] = best_cell_score;
       // Compute the next values for vertical and horizontal gap.
       best_cell_score += gap_open[k];
       last_vertical_gap[k] += gap_extension[k];
       horizontal_gap_column[row][k] += gap_extension[k];
       // Store optimum between gap open and gap extension.
       last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score);
       horizontal_gap_column[row][k] = std::max(horizontal_gap_column[row][k], best_cell_score);
     }
   }
 }

  for (size_t k = 0; k < n_seq; ++k) {
    result[k] = score_column.back()[k];
  }
  return result;
}

}  // end namespace simd




result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
#ifdef SOLUTION
  return simd::compute_alignment(sequences1, sequences2);
#else
  return compute_alignment_base(sequences1, sequences2);
#endif
}
