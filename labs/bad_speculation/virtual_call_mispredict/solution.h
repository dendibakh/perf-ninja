#include <cstdint>
#include <vector>
#include <memory>

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

struct BaseClass {
    const enum class ClassType {
        CLASS_A = 1,
        CLASS_B,
        CLASS_C
    } classType;

    BaseClass(ClassType type) : classType(type) {}

    virtual ~BaseClass() = default;

    virtual void handle(std::size_t& data) const = 0;
};

struct ClassA : public BaseClass {
    ClassA() : BaseClass(ClassType::CLASS_A) {}

    void handle(std::size_t& data) const override {
        data += 1;
    }
};

struct ClassB : public BaseClass {
    ClassB() : BaseClass(ClassType::CLASS_B) {}

    void handle(std::size_t& data) const override {
        data += 2;
    }
};

struct ClassC : public BaseClass {
    ClassC() : BaseClass(ClassType::CLASS_C) {}

    void handle(std::size_t& data) const override {
        data += 3;
    }
};

using InstanceArray = std::vector<std::unique_ptr<BaseClass>>;

void generateObjects(InstanceArray& array);
void invoke(InstanceArray& array, std::size_t& data);
