#include "unordered_set/unordered_set.hpp"
#include <iostream>
int main() {
    UnorderedSet<int> us = {1, 2, 3, 4, 5};
    std::cout << "UnorderedSet test: size=" << us.size() << ", contains(3)=" << us.contains(3) << "\n";
    if (us.size() == 5 && us.contains(3)) {
        std::cout << "✓ PASSED\n";
        return 0;
    }
    return 1;
}
