use criterion::{criterion_group, criterion_main, Criterion};

use io_opt1::{solution, /*LARGE_DATA_PATH, MEDIUM_DATA_PATH,*/ SMALL_DATA_PATH};

const FILE_NAMES: [(&str, &str); 1] = [ // change 1 to 3
    ("Small file", SMALL_DATA_PATH),
    //("Medium file", MEDIUM_DATA_PATH),
    //("Large file", LARGE_DATA_PATH),
];

fn bench1(c: &mut Criterion) {
    let mut group = c.benchmark_group("io_opt1");

    for (label, file_name) in FILE_NAMES {
        group.bench_with_input(label, file_name, |b, input| {
            b.iter(|| {
                let output = solution(input);
                std::hint::black_box(output);
            });
        });
    }
}

criterion_group!(benches, bench1);
criterion_main!(benches);
