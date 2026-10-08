#include <memory>
#include <iostream>
#include "common/unique_function.h"
struct Functor { int value; int operator()() const { return value; } };
int main() {
    Functor original{17};
    const Functor& const_original = original;
    Common::UniqueFunction<int> owned(const_original);
    original.value = 91;
    if (owned() != 17) return 1;
    auto delayed = [] {
        const auto local = [value = std::make_shared<int>(42)] { return *value; };
        return Common::UniqueFunction<int>(local);
    }();
    if (delayed() != 42) return 2;
    Common::UniqueFunction<int> movable([value = std::make_unique<int>(29)] { return *value; });
    auto moved = std::move(movable);
    if (movable || moved() != 29) return 3;
    owned = std::move(moved);
    if (moved || owned() != 29) return 4;
    Common::UniqueFunction<void, int&> args([](int& x) { ++x; });
    int value = 0; args(value);
    if (value != 1) return 5;
    std::cout << "Owned lvalue, expired-local, move-only, move assignment and argument tests passed\n";
}

