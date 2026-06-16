use criterion::{criterion_group, criterion_main, Criterion};
use std::fs::read_to_string;

use compiler_intrinsics_2::solution;

fn bench1(c: &mut Criterion) {
    let inputs = vec![
        /*"counter-example.txt" // input where sequential solution is faster*/
        "inputs/LoopVectorize.txt", // a large C++ file from the LLVM compiler.
        "inputs/MarkTwain-TomSawyer.txt", // a typical text file with long lines.
    ];

    let mut input_contents = Vec::with_capacity(inputs.len());
    for input in &inputs {
        let input_content = read_to_string(input).unwrap();
        input_contents.push(input_content);
    }

    c.bench_function("lab", |b| {
        b.iter(|| {
            for input_content in &input_contents {
                let output = solution(&input_content);
                std::hint::black_box(output);
            }
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
