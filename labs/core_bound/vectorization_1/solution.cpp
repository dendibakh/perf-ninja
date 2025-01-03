#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

#define SOLUTION
#ifdef SOLUTION

using wide_score_t = std::array<uint8_t, sequence_count_v>;
using wide_sequences_set_t = std::array<wide_score_t, sequence_size_v>;

wide_sequences_set_t transpose_sequence_set(std::vector<sequence_t> const &sequences)
{
  assert(sequences.size() == sequence_count_v);
  assert(sequences[0].size() == sequence_size_v);

  wide_sequences_set_t res{};
  for (size_t i = 0; i < sequence_count_v; ++i)
  {
    for (size_t j = 0; j < sequence_size_v; ++j)
    {
      res[j][i] = sequences[i][j];
    }
  }

  return res;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
  result_t result{};

  const auto seqs1Tr = transpose_sequence_set(sequences1);
  const auto seqs2Tr = transpose_sequence_set(sequences2);

  using scalar_score_t = int16_t;
  using score_t = std::array<int16_t, sequence_count_v>;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  /*
   * Initialise score values.
   */
  constexpr scalar_score_t gap_open{-11};
  constexpr scalar_score_t gap_extension{-1};
  constexpr scalar_score_t match{6};
  constexpr scalar_score_t mismatch{-4};

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
  for (size_t k = 0; k < sequence_count_v; ++k)
  {
    horizontal_gap_column[0][k] = gap_open;
    last_vertical_gap[k] = gap_extension * ((scalar_score_t)(score_column.size() - 1) - 1) + gap_open;
  }

  for (size_t i = 1; i < score_column.size(); ++i)
  {
    for (size_t k = 0; k < sequence_count_v; ++k)
    {
      const auto prev_vertical_gap = gap_extension * ((scalar_score_t)i - 1) + gap_open;
      score_column[i][k] = prev_vertical_gap;
      horizontal_gap_column[i][k] = prev_vertical_gap + gap_open;
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= seqs2Tr.size(); ++col)
  {
    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.
    for (size_t k = 0; k < sequence_count_v; ++k)
    {
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open;
      horizontal_gap_column[0][k] += gap_extension;
    }

    for (unsigned row = 1; row <= seqs1Tr.size(); ++row)
    {
      // Compute next score from diagonal direction with match/mismatch.
      score_t best_cell_score = last_diagonal_score;
      for (size_t k = 0; k < sequence_count_v; ++k)
      {
        best_cell_score[k] += (seqs1Tr[row - 1][k] == seqs2Tr[col - 1][k] ? match : mismatch);
      }
      for (size_t k = 0; k < sequence_count_v; ++k)
      {
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[k] = score_column[row][k];
        score_column[row][k] = best_cell_score[k];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[k] += gap_open;
        last_vertical_gap[k] += gap_extension;
        horizontal_gap_column[row][k] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[row][k] =
            std::max(horizontal_gap_column[row][k], best_cell_score[k]);
      }
    }
  }

  // Report the best score.
  for (size_t k = 0; k < sequence_count_v; ++k)
  {
    result[k] = score_column.back()[k];
  }

  return result;
}

#elif defined(SOLUTION2)
// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
  result_t result{};

  using score_t = int16_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  /*
   * Initialise score values.
   */
  constexpr score_t gap_open{-11};
  constexpr score_t gap_extension{-1};
  constexpr score_t match{6};
  constexpr score_t mismatch{-4};

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  column_t score_column_ori{};
  column_t horizontal_gap_column_ori{};
  score_t last_vertical_gap_ori{};

  /*
   * Initialise the first column of the matrix.
   */
  horizontal_gap_column_ori[0] = gap_open;
  last_vertical_gap_ori = gap_open + gap_extension * (score_t)score_column_ori.size();

  for (size_t i = 1; i < score_column_ori.size(); ++i)
  {
    const score_t vertical_gap = gap_open + ((score_t)i - 1) * (score_t)gap_extension;
    score_column_ori[i] = vertical_gap;
    horizontal_gap_column_ori[i] = vertical_gap + gap_open;
  }

  for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
       ++sequence_idx)
  {

    sequence_t const &sequence1 = sequences1[sequence_idx];
    sequence_t const &sequence2 = sequences2[sequence_idx];

    /*
     * Setup the matrix.
     * Note we can compute the entire matrix with just one column in memory,
     * since we are only interested in the last value of the last column in the
     * score matrix.
     */
    /*
     * Initialise the first column of the matrix.
     */
    column_t score_column{score_column_ori};
    column_t horizontal_gap_column{horizontal_gap_column_ori};
    score_t last_vertical_gap{last_vertical_gap_ori};
    column_t matches{};

    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence2.size(); ++col)
    {
      score_t last_diagonal_score =
          score_column[0]; // Cache last diagonal score to compute this cell.
      score_column[0] = horizontal_gap_column[0];
      last_vertical_gap = horizontal_gap_column[0] + gap_open;
      horizontal_gap_column[0] += gap_extension;

      const auto &col2 = sequence2[col - 1];
      for (unsigned row = 1; row <= sequence1.size(); ++row)
      {
        // Compute next score from diagonal direction with match/mismatch.
        matches[row] = (sequence1[row - 1] == col2 ? match : mismatch);
      }

      for (unsigned row = 1; row <= sequence1.size(); ++row)
      {
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score +
            matches[row];
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

  return result;
}

#elif defined(SOLUTION_DENIS)
// Introduce a vector of scores.
using simd_score_t = std::array<int16_t, sequence_count_v>;
using simd_sequence_t = std::array<simd_score_t, sequence_size_v>;

// Transpose a collection of sequences to a sequence of simd vectors.
simd_sequence_t transpose(std::vector<sequence_t> const &sequences)
{
  assert(sequences.size() == sequence_count_v);

  simd_sequence_t simd_sequence{};

  for (size_t i = 0; i < simd_sequence.size(); ++i)
    for (size_t j = 0; j < sequences.size(); ++j)
      simd_sequence[i][j] = sequences[j][i];

  return simd_sequence;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
  result_t result{};

  // Transpose the sequences.
  auto trSeq1 = transpose(sequences1);
  auto trSeq2 = transpose(sequences2);

  using score_t = simd_score_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  /*
   * Initialise score values.
   */
  score_t gap_open{};
  gap_open.fill(-11);
  score_t gap_extension{};
  gap_extension.fill(-1);
  score_t match{};
  match.fill(6);
  score_t mismatch{};
  mismatch.fill(-4);

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

  for (size_t i = 1; i < score_column.size(); ++i)
  {
    for (size_t k = 0; k < sequence_count_v; ++k)
    {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
      last_vertical_gap[k] += gap_extension[k];
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= trSeq2.size(); ++col)
  {
    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.
    for (size_t k = 0; k < sequence_count_v; ++k)
    {
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
      horizontal_gap_column[0][k] += gap_extension[k];
    }

    for (unsigned row = 1; row <= trSeq1.size(); ++row)
    {
      // Compute next score from diagonal direction with match/mismatch.
      score_t best_cell_score = last_diagonal_score;
      for (size_t k = 0; k < sequence_count_v; ++k)
        best_cell_score[k] += (trSeq1[row - 1][k] == trSeq2[col - 1][k] ? match[k] : mismatch[k]);
      for (size_t k = 0; k < sequence_count_v; ++k)
      {
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[k] = score_column[row][k];
        score_column[row][k] = best_cell_score[k];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[k] += gap_open[k];
        last_vertical_gap[k] += gap_extension[k];
        horizontal_gap_column[row][k] += gap_extension[k];
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[row][k] =
            std::max(horizontal_gap_column[row][k], best_cell_score[k]);
      }
    }
  }

  // Report the best score.
  for (size_t k = 0; k < sequence_count_v; ++k)
    result[k] = score_column.back()[k];

  return result;
}

#else

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
  result_t result{};

  for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
       ++sequence_idx)
  {
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

    for (size_t i = 1; i < score_column.size(); ++i)
    {
      score_column[i] = last_vertical_gap;
      horizontal_gap_column[i] = last_vertical_gap + gap_open;
      last_vertical_gap += gap_extension;
    }

    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence2.size(); ++col)
    {
      score_t last_diagonal_score =
          score_column[0]; // Cache last diagonal score to compute this cell.
      score_column[0] = horizontal_gap_column[0];
      last_vertical_gap = horizontal_gap_column[0] + gap_open;
      horizontal_gap_column[0] += gap_extension;

      for (unsigned row = 1; row <= sequence1.size(); ++row)
      {
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

  return result;
}
#endif