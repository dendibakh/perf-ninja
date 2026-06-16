use virtual_call_mispredict::{generate_objects, invoke};

fn main() {
    let arr = generate_objects();

    for _ in 0..10_000 {
        let mut data = 0;
        invoke(&arr, &mut data);
        std::hint::black_box(data);
    }
}

#[cfg(test)]
mod tests {

    use virtual_call_mispredict::{
        generate_objects, invoke, BaseClass, ClassA, ClassB, ClassC, InstanceArray, N,
    };

    #[test]
    fn validate() {
        let arr = generate_objects();
        let mut data = 0;
        invoke(&arr, &mut data);

        let arr_reference = original_generate_objects();
        let mut reference = 0;
        original_invoke(&arr_reference, &mut reference);

        assert_eq!(data, reference);
    }

    fn original_generate_objects() -> InstanceArray {
        use rand::distributions::Uniform;
        use rand::prelude::*;

        let mut generator = StdRng::seed_from_u64(0);
        let distribution = Uniform::from(0..=2);

        let mut array: Vec<Box<dyn BaseClass>> = Vec::with_capacity(N);
        for _ in 0..N {
            match distribution.sample(&mut generator) {
                0 => array.push(Box::new(ClassA {})),
                1 => array.push(Box::new(ClassB {})),
                2 => array.push(Box::new(ClassC {})),
                _ => unreachable!(),
            }
        }
        array
    }

    // Invoke the `handle` method on all instances in `array`
    fn original_invoke(array: &InstanceArray, data: &mut isize) {
        for item in array {
            item.handle(data);
        }
    }
}
