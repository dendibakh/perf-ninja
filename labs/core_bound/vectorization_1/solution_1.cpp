#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <type_traits>


// My solution.

using score_t = int16_t;

// Types that let us work on 4 elements at the time.
typedef int16_t score_4t __attribute__ ((vector_size (8)));
typedef uint8_t elem_4t __attribute__ ((vector_size (4)));

using column_4t = std::array<score_4t, sequence_size_v + 1>;

score_4t broadcast(score_t value)
{
  return score_4t {value, value, value, value};
}

score_4t max(score_4t lhs, score_4t rhs)
{
  return lhs > rhs ? lhs : rhs;
}

bool all_true(score_4t values)
{
  return values[0] && values[1] && values[2] && values[3];
}

std::ostream& operator<<(std::ostream& stream, score_4t value)
{
  stream << "{" << value[0] << ", " << value[1] << ", " << value[2] << ", " << value [3] << "}";
  return stream;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
  // result_t is an array of 16 int16_t, so we need 4 iterations to process all 16.
  // 4 elements per vector * 4 iterations = 16.
  result_t result {};

  for (size_t sequence_idx = 0; sequence_idx < sequence_count_v; sequence_idx += 4)
  {
    sequence_t const& sequence1_0 = sequences1[sequence_idx+0];
    sequence_t const& sequence1_1 = sequences1[sequence_idx+1];
    sequence_t const& sequence1_2 = sequences1[sequence_idx+2];
    sequence_t const& sequence1_3 = sequences1[sequence_idx+3];

    sequence_t const& sequence2_0 = sequences2[sequence_idx+0];
    sequence_t const& sequence2_1 = sequences2[sequence_idx+1];
    sequence_t const& sequence2_2 = sequences2[sequence_idx+2];
    sequence_t const& sequence2_3 = sequences2[sequence_idx+3];

    /*
      Initialize score values.
    */
    const score_4t gap_open {broadcast(-11)};
    const score_4t gap_extension{broadcast(-1)};
    const score_4t match{broadcast(6)};
    const score_4t mismatch{broadcast(-4)};

    /*
      Setup the matrix.
      Note we can compute the entire matrix with just one column in memory,
      since we are only interested in the last value of the last column in the
      score matrix.
    */
    column_4t score_column {};
    column_4t horizontal_gap_column {};
    score_4t last_vertical_gap {};

    /*
      Initialize the first column of the matrix.
    */
    horizontal_gap_column[0] = gap_open;
    last_vertical_gap = gap_open;

    for (size_t i = 1; i < sequence_size_v; ++i)
    {
      score_column[i] = last_vertical_gap;
      horizontal_gap_column[i] = last_vertical_gap + gap_open;
      last_vertical_gap += gap_extension;
    }

    /*
      Compute the main recursion to fill the matrix.
    */
    for (unsigned col = 1; col <= sequence_size_v; ++col)
    {
      // Cache last diagonal score to compute this cell.
      score_4t last_diagonal_score = score_column[0];

      score_column[0] = horizontal_gap_column[0];
      last_vertical_gap = horizontal_gap_column[0] + gap_open;
      horizontal_gap_column[0] += gap_extension;

      for (unsigned row = 1; row <= sequence_size_v; ++row)
      {
        elem_4t elems1 = {
          sequence1_0[row - 1],
          sequence1_1[row - 1],
          sequence1_2[row - 1],
          sequence1_3[row - 1],
          };
        elem_4t elems2 = {
          sequence2_0[col - 1],
          sequence2_1[col - 1],
          sequence2_2[col - 1],
          sequence2_3[col - 1],
        };

        // Compute the score from diagonal direction with match/mismatch.
        elem_4t equal_mask = elems1 == elems2;
        score_4t equal_mask_wide {
          (score_t)equal_mask[0],
          (score_t)equal_mask[1],
          (score_t)equal_mask[2],
          (score_t)equal_mask[3],
        };
        score_4t best_cell_score =
          last_diagonal_score + (equal_mask_wide ? match : mismatch);

        // I would like to do this instead of the line above, but getting the
        // following compiler error:
        //   vector condition type (vector of 4 'char' values) and result type
        //   (vector of 4 'int16_t' values) do not have elements of the same size
        // which is true, but for scalar code the condition in a conditional
        // operator doesn't need to be the same size as the return type.
        #if 0
        score_4t best_cell_score = last_diagonal_score + (elems1 == elems2 ? match : mismatch);
        #endif

        // Determine best core from diagonal, vertical, or horizontal direction.
        best_cell_score = max(best_cell_score, last_vertical_gap);
        best_cell_score = max(best_cell_score, horizontal_gap_column[row]);

        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score = score_column[row];
        score_column[row] = best_cell_score;

        // Compute the next values for vertical and horizontal gap.
        best_cell_score += gap_open;
        last_vertical_gap += gap_extension;
        horizontal_gap_column[row] += gap_extension;

        // Store optimum between gap open and gap extension.
        last_vertical_gap = max(last_vertical_gap, best_cell_score);
        horizontal_gap_column[row] = max(horizontal_gap_column[row], best_cell_score);
      }
    }

    // Report back best score.
    score_4t scores = score_column.back();
    result[sequence_idx + 0] = scores[0];
    result[sequence_idx + 1] = scores[1];
    result[sequence_idx + 2] = scores[2];
    result[sequence_idx + 3] = scores[3];
  }

  return result;
}
