use crate::solution;
use std::cmp::max;
use std::fs::read_to_string;

#[test]
fn validate() -> std::io::Result<()> {
    // feel free to comment out tests for debugging
    let inputs = vec![
        "inputs/test1.txt",               // basic test
        "inputs/test2.txt",               // no end-of-line in the end
        "inputs/test3.txt",               // small number of characters
        "inputs/LoopVectorize.txt",       // a large C++ file from the LLVM compiler.
        "inputs/MarkTwain-TomSawyer.txt", // a typical text file with long lines.
        "inputs/counter-example.txt",     // input where sequential solution is faster
    ];
    for input in &inputs {
        let input_contents = read_to_string(input)?;

        let original_result = original_solution(&input_contents);
        let result = solution(&input_contents);
        assert_eq!(original_result, result);
    }
    Ok(())
}

fn original_solution(input_contents: &str) -> u32 {
    let mut longest_line = 0;
    let mut cur_line_length = 0;
    for s in input_contents.chars() {
        longest_line = max(cur_line_length, longest_line);
        cur_line_length = if s == '\n' { 0 } else { cur_line_length + 1 };
    }

    // if no end-of-line in the end
    longest_line = max(cur_line_length, longest_line);
    return longest_line;
}
