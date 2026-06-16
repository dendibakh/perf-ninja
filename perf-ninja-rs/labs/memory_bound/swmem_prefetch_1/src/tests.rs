use crate::{init, solution, HashMapT, HASH_MAP_SIZE};

#[test]
fn validate() {
    // Init benchmark data
    let mut hash_map = HashMapT::new(HASH_MAP_SIZE);
    let lookups = init(&mut hash_map);

    let original_result = original_solution(&hash_map, &lookups);
    let result = solution(&hash_map, &lookups);

    assert_eq!(original_result, result);
}

fn get_sum_of_digits(mut n: i32) -> i32 {
    let mut sum = 0;
    while n != 0 {
        sum += n % 10;
        n /= 10;
    }
    sum
}

fn original_solution(hash_map: &HashMapT, lookups: &[i32]) -> i32 {
    let mut result = 0;

    for &val in lookups {
        if hash_map.find(val) {
            result += get_sum_of_digits(val);
        }
    }

    result
}
