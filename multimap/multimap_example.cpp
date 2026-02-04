#include "multimap/multimap.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Multimap Examples ===\n\n";
    
    Multimap<std::string, int> scores;
    scores.insert({"Alice", 90});
    scores.insert({"Bob", 85});
    scores.insert({"Alice", 95});  // Duplicate key allowed
    scores.insert({"Alice", 88});
    
    std::cout << "Scores:\n";
    for (const auto& p : scores) {
        std::cout << "  " << p.first << ": " << p.second << "\n";
    }
    
    std::cout << "\nCount for Alice: " << scores.count("Alice") << "\n";
    std::cout << "Count for Bob: " << scores.count("Bob") << "\n";
    
    std::cout << "\n✓ Examples completed!\n";
    return 0;
}
