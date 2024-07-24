#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    InstanceArray A;
    InstanceArray B;
    InstanceArray C;

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            A.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            B.push_back(std::make_unique<ClassB>());
        } else {
            C.push_back(std::make_unique<ClassC>());
        }
    }

    for (auto&& v : A) {
        array.emplace_back(std::move(v));
    }
    for (auto&& v : B) {
        array.emplace_back(std::move(v));
    }
    for (auto&& v : C) {
        array.emplace_back(std::move(v));
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
