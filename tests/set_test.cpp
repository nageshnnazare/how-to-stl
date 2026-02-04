/**
 * @file set_test.cpp
 * @brief Test suite for Set implementation
 */

#include "set/set.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=================================================\n";
    std::cout << "        Set Implementation Test Suite           \n";
    std::cout << "=================================================\n\n";
    
    int tests_passed = 0;
    int tests_total = 15;
    
    // Test 1: Insert and size
    std::cout << "Test 1: Insert and size... ";
    {
        Set<int> s;
        s.insert(5);
        s.insert(3);
        s.insert(7);
        if (s.size() != 3) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 2: Duplicate insert
    std::cout << "Test 2: Duplicate insert... ";
    {
        Set<int> s;
        auto r1 = s.insert(5);
        auto r2 = s.insert(5);
        if (!r1.second || r2.second || s.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 3: Ordered iteration
    std::cout << "Test 3: Ordered iteration... ";
    {
        Set<int> s = {5, 2, 8, 1, 9};
        int prev = -1;
        bool sorted = true;
        for (int x : s) {
            if (x <= prev) sorted = false;
            prev = x;
        }
        if (!sorted) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 4: Find
    std::cout << "Test 4: Find... ";
    {
        Set<int> s = {10, 20, 30};
        auto it1 = s.find(20);
        auto it2 = s.find(99);
        if (it1 == s.end() || *it1 != 20 || it2 != s.end()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 5: Contains
    std::cout << "Test 5: Contains... ";
    {
        Set<int> s = {1, 2, 3};
        if (!s.contains(2) || s.contains(99)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 6: Erase
    std::cout << "Test 6: Erase... ";
    {
        Set<int> s = {1, 2, 3, 4, 5};
        size_t n = s.erase(3);
        if (n != 1 || s.size() != 4 || s.contains(3)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 7: Clear
    std::cout << "Test 7: Clear... ";
    {
        Set<int> s = {1, 2, 3};
        s.clear();
        if (s.size() != 0 || !s.empty()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 8: Copy constructor
    std::cout << "Test 8: Copy constructor... ";
    {
        Set<int> s1 = {1, 2, 3};
        Set<int> s2 = s1;
        if (s2.size() != 3 || !s2.contains(2)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 9: Move constructor
    std::cout << "Test 9: Move constructor... ";
    {
        Set<int> s1 = {1, 2, 3};
        Set<int> s2 = std::move(s1);
        if (s2.size() != 3 || s1.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 10: Empty set
    std::cout << "Test 10: Empty set... ";
    {
        Set<int> s;
        if (!s.empty() || s.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 11: Count
    std::cout << "Test 11: Count... ";
    {
        Set<int> s = {10, 20, 30};
        if (s.count(20) != 1 || s.count(99) != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 12: With strings
    std::cout << "Test 12: With strings... ";
    {
        Set<std::string> s = {"apple", "banana", "cherry"};
        if (s.size() != 3 || !s.contains("banana")) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 13: Large set
    std::cout << "Test 13: Large set... ";
    {
        Set<int> s;
        for (int i = 0; i < 100; ++i) s.insert(i);
        if (s.size() != 100) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 14: Erase non-existent
    std::cout << "Test 14: Erase non-existent... ";
    {
        Set<int> s = {1, 2, 3};
        size_t n = s.erase(99);
        if (n != 0 || s.size() != 3) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 15: Iterator increment
    std::cout << "Test 15: Iterator increment... ";
    {
        Set<int> s = {1, 2, 3, 4, 5};
        auto it = s.begin();
        if (*it != 1) { std::cout << "FAILED\n"; return 1; }
        ++it;
        if (*it != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    std::cout << "\n=================================================\n";
    std::cout << "  RESULTS: " << tests_passed << "/" << tests_total << " tests passed\n";
    std::cout << "=================================================\n";
    std::cout << "✓ All tests PASSED!\n";
    
    return 0;
}

