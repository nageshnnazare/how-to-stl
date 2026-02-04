#include "multiset/multiset.hpp"
#include <iostream>
int main() {
    Multiset<int> ms = {1, 1, 2, 3, 3, 3};
    std::cout << "Multiset test: size=" << ms.size() << ", count(3)=" << ms.count(3) << "\n";
    if (ms.size() == 6 && ms.count(3) == 3) {
        std::cout << "✓ PASSED\n";
        return 0;
    }
    return 1;
}
