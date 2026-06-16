// Assume this constant never changes
pub const N: usize = 64 * 1024;

pub trait BaseClass {
    fn handle(&self, data: &mut isize);
}

pub struct ClassA {}
impl BaseClass for ClassA {
    fn handle(&self, data: &mut isize) {
        *data += 1;
    }
}

pub struct ClassB {}
impl BaseClass for ClassB {
    fn handle(&self, data: &mut isize) {
        *data += 2;
    }
}

pub struct ClassC {}
impl BaseClass for ClassC {
    fn handle(&self, data: &mut isize) {
        *data += 3;
    }
}

pub type InstanceArray = Vec<Box<dyn BaseClass>>;

pub fn generate_objects() -> InstanceArray {
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
pub fn invoke(array: &InstanceArray, data: &mut isize) {
    for item in array {
        item.handle(data);
    }
}
