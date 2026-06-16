use std::cmp::max;
use crate::{
    compute_alignment, init, AlignResult, Sequence, SEQUENCE_COUNT, SEQUENCE_SIZE,
};

#[test]
fn validate() {
    let (sequences1, sequences2) = init();

    let computed_result = compute_alignment(&sequences1, &sequences2);

    let mut expected_result: AlignResult = [0i16; SEQUENCE_COUNT];
    for sequence_idx in 0..sequences1.len() {
        type Score = i16;
        type Column = [Score; SEQUENCE_SIZE + 1];

        let sequence1: &Sequence = &sequences1[sequence_idx];
        let sequence2: &Sequence = &sequences2[sequence_idx];

        /*
         * Initialise score values.
         */
        let gap_open: Score = -11;
        let gap_extension: Score = -1;
        let match_score: Score = 6;
        let mismatch: Score = -4;

        /*
         * Setup the matrix.
         * Note we can compute the entire matrix with just one column in memory,
         * since we are only interested in the last value of the last column in the
         * score matrix.
         */
        let mut score_column: Column = [0; SEQUENCE_SIZE + 1];
        let mut horizontal_gap_column: Column = [0; SEQUENCE_SIZE + 1];
        let mut last_vertical_gap: Score;

        /*
         * Initialise the first column of the matrix.
         */
        horizontal_gap_column[0] = gap_open;
        last_vertical_gap = gap_open;

        for i in 1..score_column.len() {
            score_column[i] = last_vertical_gap;
            horizontal_gap_column[i] = last_vertical_gap + gap_open;
            last_vertical_gap += gap_extension;
        }

        /*
         * Compute the main recursion to fill the matrix.
         */
        for col in 1..=sequence2.len() {
            let mut last_diagonal_score: Score = score_column[0]; // Cache last diagonal score to compute this cell.
            score_column[0] = horizontal_gap_column[0];
            last_vertical_gap = horizontal_gap_column[0] + gap_open;
            horizontal_gap_column[0] += gap_extension;

            for row in 1..=sequence1.len() {
                // Compute next score from diagonal direction with match/mismatch.
                let mut best_cell_score = last_diagonal_score
                    + (if sequence1[row - 1] == sequence2[col - 1] {
                        match_score
                    } else {
                        mismatch
                    });

                best_cell_score = max(best_cell_score, last_vertical_gap);
                best_cell_score = max(best_cell_score, horizontal_gap_column[row]);

                last_diagonal_score = score_column[row];
                score_column[row] = best_cell_score;

                best_cell_score += gap_open;
                last_vertical_gap += gap_extension;
                horizontal_gap_column[row] += gap_extension;

                last_vertical_gap = max(last_vertical_gap, best_cell_score);
                horizontal_gap_column[row] = max(horizontal_gap_column[row], best_cell_score);
            }
        }

        // Report the best score.
        expected_result[sequence_idx] = *score_column.last().unwrap();
    }

    assert_eq!(computed_result, expected_result);
}
