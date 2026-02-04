#include "unordered_map/unordered_map.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UnorderedMap Examples ===\n\n";
    
    UnorderedMap<std::string, int> ages;
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    ages["Charlie"] = 35;
    
    std::cout << "Ages:\n";
    std::cout << "  Alice: " << ages["Alice"] << "\n";
    std::cout << "  Bob: " << ages["Bob"] << "\n";
    std::cout << "  Charlie: " << ages["Charlie"] << "\n";
    
    std::cout << "\nSize: " << ages.size() << "\n";
    std::cout << "Contains Bob: " << (ages.contains("Bob") ? "yes" : "no") << "\n";
    
    ages.erase("Bob");
    std::cout << "\nAfter erasing Bob:\n";
    std::cout << "  Size: " << ages.size() << "\n";
    std::cout << "  Contains Bob: " << (ages.contains("Bob") ? "yes" : "no") << "\n";
    
    // Initializer list
    UnorderedMap<int, std::string> items = {{1, "one"}, {2, "two"}, {3, "three"}};
    std::cout << "\nItems[2]: " << items[2] << "\n";
    
    std::cout << "\n✓ Examples completed!\n";
    return 0;
}
