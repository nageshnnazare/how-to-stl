#include "pair/pair.hpp"
#include <iostream>
#include <string>

void example1_basic() {
    std::cout << "\n=== Example 1: Basic Pair ===\n";
    
    Pair<int, std::string> p(42, "answer");
    std::cout << "Pair: (" << p.first << ", " << p.second << ")\n";
}

void example2_make_pair() {
    std::cout << "\n=== Example 2: make_pair Helper ===\n";
    
    Pair<double, std::string> p(3.14, "pi");
    std::cout << "Pi: " << p.first << " = " << p.second << "\n";
}

void example3_coordinates() {
    std::cout << "\n=== Example 3: Coordinates ===\n";
    
    Pair<double, double> point(10.5, 20.3);
    std::cout << "Point: (" << point.first << ", " << point.second << ")\n";
}

void example4_key_value() {
    std::cout << "\n=== Example 4: Key-Value Store ===\n";
    
    Pair<std::string, int> entry("age", 25);
    std::cout << entry.first << " = " << entry.second << "\n";
}

void example5_comparisons() {
    std::cout << "\n=== Example 5: Comparisons ===\n";
    
    Pair<int, int> p1(1, 2);
    Pair<int, int> p2(1, 2);
    Pair<int, int> p3(2, 1);
    
    std::cout << "p1 == p2: " << (p1 == p2 ? "true" : "false") << "\n";
    std::cout << "p1 < p3: " << (p1 < p3 ? "true" : "false") << "\n";
}

int main() {
    std::cout << "=== Pair Examples ===\n";
    example1_basic();
    example2_make_pair();
    example3_coordinates();
    example4_key_value();
    example5_comparisons();
    std::cout << "\n✓ All examples completed!\n";
    return 0;
}
