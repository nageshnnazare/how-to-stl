/**
 * @file set_example.cpp
 * @brief Examples demonstrating the Set class implementation
 */

#include "set/set.hpp"
#include <iostream>
#include <string>

void example1_basic_operations() {
    std::cout << "\n=== Example 1: Basic Operations ===\n";
    
    Set<int> s;
    
    // Insert elements
    s.insert(5);
    s.insert(2);
    s.insert(8);
    s.insert(1);
    s.insert(9);
    
    std::cout << "Set after insertions: ";
    for (int x : s) {
        std::cout << x << " ";
    }
    std::cout << "\n";
    
    // Try to insert duplicate
    auto result = s.insert(5);
    std::cout << "Insert duplicate 5: " << (result.second ? "success" : "already exists") << "\n";
    
    std::cout << "Size: " << s.size() << "\n";
}

void example2_initializer_list() {
    std::cout << "\n=== Example 2: Initializer List ===\n";
    
    Set<int> s = {3, 1, 4, 1, 5, 9, 2, 6};
    
    std::cout << "Set (duplicates removed, sorted): ";
    for (int x : s) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

void example3_find_contains() {
    std::cout << "\n=== Example 3: Find and Contains ===\n";
    
    Set<int> s = {10, 20, 30, 40, 50};
    
    if (s.contains(30)) {
        std::cout << "30 is in the set\n";
    }
    
    auto it = s.find(40);
    if (it != s.end()) {
        std::cout << "Found: " << *it << "\n";
    }
    
    if (!s.contains(100)) {
        std::cout << "100 is not in the set\n";
    }
}

void example4_erase() {
    std::cout << "\n=== Example 4: Erase ===\n";
    
    Set<int> s = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    std::cout << "Before erase: ";
    for (int x : s) std::cout << x << " ";
    std::cout << "\n";
    
    s.erase(5);
    s.erase(10);
    s.erase(1);
    
    std::cout << "After erasing 5, 10, 1: ";
    for (int x : s) std::cout << x << " ";
    std::cout << "\n";
}

void example5_iteration() {
    std::cout << "\n=== Example 5: Iteration ===\n";
    
    Set<int> s = {50, 30, 70, 20, 40, 60, 80};
    
    std::cout << "Forward iteration: ";
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    std::cout << "Range-based for: ";
    for (int x : s) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

void example6_strings() {
    std::cout << "\n=== Example 6: Set of Strings ===\n";
    
    Set<std::string> words = {"apple", "banana", "cherry", "date", "elderberry"};
    
    std::cout << "Words (alphabetically): ";
    for (const auto& word : words) {
        std::cout << word << " ";
    }
    std::cout << "\n";
    
    words.insert("fig");
    words.insert("apple");  // Duplicate
    
    std::cout << "After adding 'fig' and duplicate 'apple': ";
    for (const auto& word : words) {
        std::cout << word << " ";
    }
    std::cout << "\n";
}

void example7_copy_move() {
    std::cout << "\n=== Example 7: Copy and Move ===\n";
    
    Set<int> s1 = {1, 2, 3, 4, 5};
    
    // Copy
    Set<int> s2 = s1;
    std::cout << "Copied set: ";
    for (int x : s2) std::cout << x << " ";
    std::cout << "\n";
    
    // Move
    Set<int> s3 = std::move(s1);
    std::cout << "Moved set: ";
    for (int x : s3) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "Original set size after move: " << s1.size() << "\n";
}

void example8_clear() {
    std::cout << "\n=== Example 8: Clear ===\n";
    
    Set<int> s = {1, 2, 3, 4, 5};
    std::cout << "Size before clear: " << s.size() << "\n";
    
    s.clear();
    std::cout << "Size after clear: " << s.size() << "\n";
    std::cout << "Empty: " << (s.empty() ? "yes" : "no") << "\n";
}

void example9_count() {
    std::cout << "\n=== Example 9: Count ===\n";
    
    Set<int> s = {10, 20, 30, 40};
    
    std::cout << "Count of 20: " << s.count(20) << "\n";
    std::cout << "Count of 99: " << s.count(99) << "\n";
}

void example10_unique_elements() {
    std::cout << "\n=== Example 10: Unique Elements from Array ===\n";
    
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    Set<int> unique_nums;
    
    for (int x : arr) {
        unique_nums.insert(x);
    }
    
    std::cout << "Original array: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n";
    
    std::cout << "Unique elements (sorted): ";
    for (int x : unique_nums) std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "          Set Class Examples                    \n";
    std::cout << "=================================================\n";
    
    try {
        example1_basic_operations();
        example2_initializer_list();
        example3_find_contains();
        example4_erase();
        example5_iteration();
        example6_strings();
        example7_copy_move();
        example8_clear();
        example9_count();
        example10_unique_elements();
        
        std::cout << "\n=================================================\n";
        std::cout << "   All examples completed successfully!        \n";
        std::cout << "=================================================\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

