use criterion::{criterion_group, criterion_main, Criterion};

use vectorization_2::{checksum, init, Blob, N};

fn bench1(c: &mut Criterion) {
    let mut blob: Blob = [0; N];
    init(&mut blob);

    c.bench_function("lab", |b| {
        b.iter(|| {
            let result = checksum(&blob);
            std::hint::black_box(result);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
