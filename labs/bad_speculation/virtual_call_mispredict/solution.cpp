#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    // Group classes by type of subclass so that when invoke is called, all ClassA's are handled first
    // followed by ClassB then ClassC. This helps the CPU predict where to branch to when making
    // virtual calls into the subclass.
    InstanceArray class_a, class_b, class_c;

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            class_a.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            class_b.push_back(std::make_unique<ClassB>());
        } else {
            class_c.push_back(std::make_unique<ClassC>());
        }
    }

    array.reserve(N);
    for (auto& a : class_a) {
      array.push_back(std::move(a));
    }
    for (auto& b : class_b) {
      array.push_back(std::move(b));
    }
    for (auto& c : class_c) {
      array.push_back(std::move(c));
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
