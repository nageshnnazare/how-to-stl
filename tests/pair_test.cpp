#include "pair/pair.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Pair Test Suite ===\n\n";
    
    int passed = 0, total = 10;
    
    std::cout << "Test 1: Basic construction... ";
    { Pair<int, std::string> p(1, "one"); if (p.first == 1 && p.second == "one") { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 2: make_pair... ";
    { Pair<int, double> p(2, 3.14); if (p.first == 2 && p.second == 3.14) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 3: Copy... ";
    { Pair<int, int> p1(1, 2); Pair<int, int> p2 = p1; if (p2.first == 1 && p2.second == 2) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 4: Equality... ";
    { Pair<int, int> p1(1, 2); Pair<int, int> p2(1, 2); if (p1 == p2) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 5: Inequality... ";
    { Pair<int, int> p1(1, 2); Pair<int, int> p2(2, 1); if (p1 != p2) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 6: Less than... ";
    { Pair<int, int> p1(1, 2); Pair<int, int> p2(2, 1); if (p1 < p2) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 7: Strings... ";
    { Pair<std::string, int> p("test", 42); if (p.first == "test") { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 8: Move... ";
    { Pair<std::string, int> p1(std::string("hello"), 10); Pair<std::string, int> p2 = std::move(p1); if (p2.first == "hello") { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 9: Assignment... ";
    { Pair<int, int> p1(1, 2); Pair<int, int> p2; p2 = p1; if (p2.first == 1) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "Test 10: Default... ";
    { Pair<int, int> p; if (p.first == 0 && p.second == 0) { std::cout << "PASSED\n"; ++passed; } else { std::cout << "FAILED\n"; return 1; } }
    
    std::cout << "\n" << passed << "/" << total << " tests passed\n";
    std::cout << "✓ All tests PASSED!\n";
    return 0;
}
