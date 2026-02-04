#include "map/map.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Map Examples ===\n\n";
    
    // Example 1: Basic operations
    Map<std::string, int> ages;
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    ages["Charlie"] = 35;
    
    std::cout << "Ages:\n";
    for (const auto& p : ages) {
        std::cout << "  " << p.first << ": " << p.second << "\n";
    }
    
    // Example 2: Find and at
    if (ages.contains("Bob")) {
        std::cout << "\nBob's age: " << ages.at("Bob") << "\n";
    }
    
    // Example 3: Erase
    ages.erase("Bob");
    std::cout << "\nAfter erasing Bob, size: " << ages.size() << "\n";
    
    // Example 4: Initializer list
    Map<int, std::string> items = {{1, "one"}, {2, "two"}, {3, "three"}};
    std::cout << "\nItems:\n";
    for (const auto& p : items) {
        std::cout << "  " << p.first << " -> " << p.second << "\n";
    }
    
    std::cout << "\n✓ All examples completed!\n";
    return 0;
}
