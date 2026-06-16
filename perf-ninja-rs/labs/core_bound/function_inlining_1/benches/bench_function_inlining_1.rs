use criterion::{criterion_group, criterion_main, Criterion};

use function_inlining_1::{init, solution};

fn bench1(c: &mut Criterion) {
    let arr = init();

    c.bench_function("lab", |b| {
        b.iter(|| {
            let mut copy = arr;
            solution(&mut copy);
            std::hint::black_box(copy);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
