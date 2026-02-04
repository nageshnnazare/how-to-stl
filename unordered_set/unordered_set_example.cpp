#include "unordered_set/unordered_set.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UnorderedSet Examples ===\n\n";
    
    UnorderedSet<int> us = {3, 1, 4, 1, 5, 9, 2, 6};
    
    std::cout << "Size: " << us.size() << " (duplicates removed)\n";
    std::cout << "Contains 5: " << (us.contains(5) ? "yes" : "no") << "\n";
    std::cout << "Contains 99: " << (us.contains(99) ? "yes" : "no") << "\n";
    
    us.insert(10);
    us.erase(3);
    std::cout << "\nAfter insert(10) and erase(3):\n";
    std::cout << "  Size: " << us.size() << "\n";
    std::cout << "  Contains 3: " << (us.contains(3) ? "yes" : "no") << "\n";
    std::cout << "  Contains 10: " << (us.contains(10) ? "yes" : "no") << "\n";
    
    // String example
    UnorderedSet<std::string> words = {"apple", "banana", "cherry"};
    std::cout << "\nWord set size: " << words.size() << "\n";
    std::cout << "Contains 'banana': " << (words.contains("banana") ? "yes" : "no") << "\n";
    
    std::cout << "\n✓ Examples completed!\n";
    return 0;
}
