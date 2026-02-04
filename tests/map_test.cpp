#include "map/map.hpp"
#include <iostream>

int main() {
    std::cout << "=== Map Test Suite ===\n";
    int passed = 0, total = 5;
    
    // Test 1
    std::cout << "Test 1: Insert and operator[]... ";
    Map<int, std::string> m;
    m[1] = "one";
    m[2] = "two";
    if (m.size() == 2 && m[1] == "one") { std::cout << "PASSED\n"; ++passed; }
    else { std::cout << "FAILED\n"; return 1; }
    
    // Test 2
    std::cout << "Test 2: Find... ";
    auto it = m.find(2);
    if (it != m.end() && it->second == "two") { std::cout << "PASSED\n"; ++passed; }
    else { std::cout << "FAILED\n"; return 1; }
    
    // Test 3
    std::cout << "Test 3: Erase... ";
    m.erase(1);
    if (m.size() == 1 && !m.contains(1)) { std::cout << "PASSED\n"; ++passed; }
    else { std::cout << "FAILED\n"; return 1; }
    
    // Test 4
    std::cout << "Test 4: Contains... ";
    if (m.contains(2) && !m.contains(99)) { std::cout << "PASSED\n"; ++passed; }
    else { std::cout << "FAILED\n"; return 1; }
    
    // Test 5
    std::cout << "Test 5: Clear... ";
    m.clear();
    if (m.empty() && m.size() == 0) { std::cout << "PASSED\n"; ++passed; }
    else { std::cout << "FAILED\n"; return 1; }
    
    std::cout << "\n" << passed << "/" << total << " tests passed\n✓ All tests PASSED!\n";
    return 0;
}
