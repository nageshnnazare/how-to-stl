#include "multimap/multimap.hpp"
#include <iostream>
int main() {
    Multimap<int, std::string> mm;
    mm.insert({1, "one"});
    mm.insert({1, "uno"});
    std::cout << "Multimap test: size=" << mm.size() << ", count(1)=" << mm.count(1) << "\n";
    if (mm.size() == 2 && mm.count(1) == 2) {
        std::cout << "✓ PASSED\n";
        return 0;
    }
    return 1;
}
