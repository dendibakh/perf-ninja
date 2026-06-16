use criterion::{criterion_group, criterion_main, Criterion};

use dep_chains_1::{get_random_list, solution, Arena};

fn bench1(c: &mut Criterion) {
    let arena1 = Arena::new();
    let l1 = get_random_list(&arena1);
    let arena2 = Arena::new();
    let l2 = get_random_list(&arena2);
    c.bench_function("lab", |b| {
        b.iter(|| {
            let output = unsafe { solution(l1, l2) };
            std::hint::black_box(output);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
