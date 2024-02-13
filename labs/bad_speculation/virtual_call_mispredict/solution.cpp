#include "solution.h"

#include <random>

std::unique_ptr<BaseClass> fabric(int i) {
    if (i == 0) {
        return std::make_unique<ClassA>();
    } else if (i == 1) {
        return std::make_unique<ClassB>();
    }
    return std::make_unique<ClassC>();
}

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    InstanceArray parts[3];
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        parts[value].emplace_back(fabric(value));
    }
    for (std::size_t i = 0; i < 3; i++) {
        array.insert(array.end(),
                    std::make_move_iterator(parts[i].begin()), 
                    std::make_move_iterator(parts[i].end()));
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
