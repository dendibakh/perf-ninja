use criterion::{criterion_group, criterion_main, Criterion};

use vectorization_1::{compute_alignment, init};

fn bench1(c: &mut Criterion) {
    let (sequences1, sequences2) = init();
    c.bench_function("lab", |b| {
        b.iter(|| {
            let result = compute_alignment(&sequences1, &sequences2);
            std::hint::black_box(result);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
