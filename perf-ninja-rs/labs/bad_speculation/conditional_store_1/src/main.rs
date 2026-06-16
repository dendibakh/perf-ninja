use conditional_store_1::{init, select, N};

fn main() {
    let arr = init();

    let mut temp = [(0, 0); N];
    let lower = u32::MAX / 4 + 1;
    let upper = u32::MAX / 2 + lower;

    for _ in 0..10_000 {
        select(&mut temp, &arr, lower, upper);
        std::hint::black_box(&mut temp);
    }
}

#[cfg(test)]
mod tests {
    use conditional_store_1::{init, select, N, S};

    #[test]
    fn validate() {
        let arr = init();

        let lower = u32::MAX / 4 + 1;
        let upper = u32::MAX / 2 + lower;

        let mut expected = [(0, 0); N];
        let mut result = [(0, 0); N];
        let expected_size = original_select(&mut expected, &arr, lower, upper);
        let result_size = select(&mut result, &arr, lower, upper);

        assert_eq!(result_size, expected_size);
        assert_eq!(result, expected);
    }

    fn original_select(output: &mut [S; N], input: &[S; N], lower: u32, upper: u32) -> usize {
        let mut count = 0;
        for item in input {
            if (lower <= item.0) && (item.0 <= upper) {
                output[count] = *item;
                count += 1;
            }
        }
        count
    }
}
