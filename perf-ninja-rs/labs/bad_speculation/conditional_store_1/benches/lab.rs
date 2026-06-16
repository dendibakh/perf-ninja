use criterion::{criterion_group, criterion_main, Criterion};

use conditional_store_1::{init, select, N};

fn bench1(c: &mut Criterion) {
    let arr = init();

    let mut temp = [(0, 0); N];
    let lower = u32::MAX / 4 + 1;
    let upper = u32::MAX / 2 + lower;

    c.bench_function("lab", |b| {
        b.iter(|| {
            select(&mut temp, &arr, lower, upper);
            std::hint::black_box(&mut temp);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
