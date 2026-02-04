#include "multiset/multiset.hpp"
#include <iostream>

int main() {
    std::cout << "=== Multiset Examples ===\n\n";
    
    Multiset<int> ms = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    
    std::cout << "Elements (sorted, with duplicates): ";
    for (int x : ms) std::cout << x << " ";
    std::cout << "\n";
    
    std::cout << "Count of 5: " << ms.count(5) << "\n";
    std::cout << "Count of 1: " << ms.count(1) << "\n";
    
    ms.erase(5);
    std::cout << "\nAfter erasing all 5s: ";
    for (int x : ms) std::cout << x << " ";
    std::cout << "\n";
    
    std::cout << "\n✓ Examples completed!\n";
    return 0;
}
