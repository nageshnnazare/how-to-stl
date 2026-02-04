#include "unordered_map/unordered_map.hpp"
#include <iostream>
int main() {
    UnorderedMap<int, std::string> um;
    um[1] = "one";
    um[2] = "two";
    std::cout << "UnorderedMap test: size=" << um.size() << ", um[1]=" << um[1] << "\n";
    if (um.size() == 2 && um[1] == "one") {
        std::cout << "✓ PASSED\n";
        return 0;
    }
    return 1;
}
