pub const NUM_BUCKETS: usize = 7;
pub const NUM_VALUES: usize = 1024 * 1024;

fn map_to_bucket(v: isize) -> usize {
    // diff
    if v >= 0 && v < 13 {
        return 0;
    }
    // 13
    else if v >= 13 && v < 29 {
        return 1;
    }
    // 16
    else if v >= 29 && v < 41 {
        return 2;
    }
    // 12
    else if v >= 41 && v < 53 {
        return 3;
    }
    // 12
    else if v >= 53 && v < 71 {
        return 4;
    }
    // 18
    else if v >= 71 && v < 83 {
        return 5;
    }
    // 12
    else if v >= 83 && v < 100 {
        return 6;
    } // 17
    panic!("// let it crash");
}

pub fn histogram(values: &[isize]) -> [isize; NUM_BUCKETS] {
    let mut ret_buckets = [0; NUM_BUCKETS];
    for v in values {
        ret_buckets[map_to_bucket(*v)] += 1;
    }
    ret_buckets
}

pub fn init() -> Vec<isize> {
    use rand::distributions::Uniform;
    use rand::prelude::*;
    let mut generator = thread_rng();
    // generate random integer in the closed interval [0,99]
    let distribution = Uniform::from(0..=99);

    let mut values = Vec::with_capacity(NUM_VALUES);
    values.resize_with(NUM_VALUES, || distribution.sample(&mut generator));
    values
}
