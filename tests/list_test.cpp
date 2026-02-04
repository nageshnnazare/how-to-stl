/**
 * @file list_test.cpp
 * @brief Test suite for List implementation
 */

#include "list/list.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=================================================\n";
    std::cout << "       List Implementation Test Suite           \n";
    std::cout << "=================================================\n\n";
    
    int tests_passed = 0;
    int tests_total = 15;
    
    // Test 1: Push back
    std::cout << "Test 1: Push back... ";
    {
        List<int> l;
        l.push_back(10);
        l.push_back(20);
        if (l.size() != 2 || l.back() != 20) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 2: Push front
    std::cout << "Test 2: Push front... ";
    {
        List<int> l;
        l.push_back(10);
        l.push_front(5);
        if (l.front() != 5 || l.back() != 10) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 3: Pop front
    std::cout << "Test 3: Pop front... ";
    {
        List<int> l;
        l.push_back(1);
        l.push_back(2);
        l.pop_front();
        if (l.front() != 2 || l.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 4: Pop back
    std::cout << "Test 4: Pop back... ";
    {
        List<int> l;
        l.push_back(1);
        l.push_back(2);
        l.pop_back();
        if (l.back() != 1 || l.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 5: Empty list
    std::cout << "Test 5: Empty list... ";
    {
        List<int> l;
        if (!l.empty() || l.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 6: Initializer list
    std::cout << "Test 6: Initializer list... ";
    {
        List<int> l = {1, 2, 3, 4, 5};
        if (l.size() != 5 || l.front() != 1 || l.back() != 5) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 7: Copy constructor
    std::cout << "Test 7: Copy constructor... ";
    {
        List<int> l1 = {1, 2, 3};
        List<int> l2 = l1;
        if (l2.size() != 3 || l2.front() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 8: Move constructor
    std::cout << "Test 8: Move constructor... ";
    {
        List<int> l1 = {1, 2, 3};
        List<int> l2 = std::move(l1);
        if (l2.size() != 3 || l1.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 9: Clear
    std::cout << "Test 9: Clear... ";
    {
        List<int> l = {1, 2, 3};
        l.clear();
        if (!l.empty()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 10: Iteration
    std::cout << "Test 10: Iteration... ";
    {
        List<int> l = {1, 2, 3};
        int sum = 0;
        for (const auto& val : l) sum += val;
        if (sum != 6) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 11: With strings
    std::cout << "Test 11: With strings... ";
    {
        List<std::string> l;
        l.push_back("hello");
        l.push_back("world");
        if (l.front() != "hello" || l.back() != "world") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 12: Large list
    std::cout << "Test 12: Large list... ";
    {
        List<int> l;
        for (int i = 0; i < 100; ++i) l.push_back(i);
        if (l.size() != 100) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 13: Alternating push
    std::cout << "Test 13: Alternating push... ";
    {
        List<int> l;
        l.push_back(2);
        l.push_front(1);
        l.push_back(3);
        if (l.front() != 1 || l.back() != 3) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 14: Multiple pops
    std::cout << "Test 14: Multiple pops... ";
    {
        List<int> l = {1, 2, 3};
        l.pop_front();
        l.pop_back();
        if (l.size() != 1 || l.front() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 15: Assignment
    std::cout << "Test 15: Assignment... ";
    {
        List<int> l1 = {1, 2, 3};
        List<int> l2;
        l2 = l1;
        if (l2.size() != 3 || l2.front() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    std::cout << "\n=================================================\n";
    std::cout << "  RESULTS: " << tests_passed << "/" << tests_total << " tests passed\n";
    std::cout << "=================================================\n";
    std::cout << "✓ All tests PASSED!\n";
    
    return 0;
}
