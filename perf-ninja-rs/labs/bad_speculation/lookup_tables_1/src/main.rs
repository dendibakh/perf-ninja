use lookup_tables_1::{histogram, init};

fn main() {
    let values = init();
    for _ in 0..1_000 {
        let output = histogram(&values);
        std::hint::black_box(output);
    }
}

#[cfg(test)]
mod tests {

    use lookup_tables_1::{histogram, init, NUM_BUCKETS};

    #[test]
    fn validate() {
        let values = init();

        let original_result = original_histogram(&values);
        let result = histogram(&values);

        assert_eq!(original_result, result);
    }

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

    fn original_histogram(values: &[isize]) -> [isize; NUM_BUCKETS] {
        let mut ret_buckets = [0; NUM_BUCKETS];
        for v in values {
            ret_buckets[map_to_bucket(*v)] += 1;
        }
        ret_buckets
    }
}
