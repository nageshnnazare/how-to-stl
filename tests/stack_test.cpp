/**
 * @file stack_test.cpp
 * @brief Test suite for Stack implementation
 */

#include "stack/stack.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=================================================\n";
    std::cout << "       Stack Implementation Test Suite          \n";
    std::cout << "=================================================\n\n";
    
    int tests_passed = 0;
    int tests_total = 15;
    
    // Test 1: Basic push and top
    std::cout << "Test 1: Basic push and top... ";
    {
        Stack<int> s;
        s.push(10);
        s.push(20);
        if (s.top() != 20 || s.size() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 2: Pop operation
    std::cout << "Test 2: Pop operation... ";
    {
        Stack<int> s;
        s.push(10);
        s.push(20);
        s.pop();
        if (s.top() != 10 || s.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 3: Empty stack
    std::cout << "Test 3: Empty stack... ";
    {
        Stack<int> s;
        if (!s.empty() || s.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 4: LIFO order
    std::cout << "Test 4: LIFO order... ";
    {
        Stack<int> s;
        s.push(1);
        s.push(2);
        s.push(3);
        if (s.top() != 3) { std::cout << "FAILED\n"; return 1; }
        s.pop();
        if (s.top() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 5: Copy constructor
    std::cout << "Test 5: Copy constructor... ";
    {
        Stack<int> s1;
        s1.push(10);
        s1.push(20);
        Stack<int> s2 = s1;
        if (s2.top() != 20 || s2.size() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 6: Move constructor
    std::cout << "Test 6: Move constructor... ";
    {
        Stack<int> s1;
        s1.push(10);
        s1.push(20);
        Stack<int> s2 = std::move(s1);
        if (s2.top() != 20 || !s1.empty()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 7: With strings
    std::cout << "Test 7: With strings... ";
    {
        Stack<std::string> s;
        s.push("hello");
        s.push("world");
        if (s.top() != "world") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 8: Emplace
    std::cout << "Test 8: Emplace... ";
    {
        Stack<std::string> s;
        s.emplace("test");
        if (s.top() != "test") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 9: Swap
    std::cout << "Test 9: Swap... ";
    {
        Stack<int> s1, s2;
        s1.push(10);
        s2.push(20);
        s1.swap(s2);
        if (s1.top() != 20 || s2.top() != 10) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 10: Comparison
    std::cout << "Test 10: Comparison... ";
    {
        Stack<int> s1, s2;
        s1.push(10);
        s2.push(10);
        if (!(s1 == s2)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 11: Large stack
    std::cout << "Test 11: Large stack... ";
    {
        Stack<int> s;
        for (int i = 0; i < 100; ++i) s.push(i);
        if (s.size() != 100 || s.top() != 99) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 12: Push move
    std::cout << "Test 12: Push move... ";
    {
        Stack<std::string> s;
        std::string str = "move_me";
        s.push(std::move(str));
        if (s.top() != "move_me") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 13: Multiple pops
    std::cout << "Test 13: Multiple pops... ";
    {
        Stack<int> s;
        s.push(1);
        s.push(2);
        s.push(3);
        s.pop();
        s.pop();
        s.pop();
        if (!s.empty()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 14: Assignment
    std::cout << "Test 14: Assignment... ";
    {
        Stack<int> s1, s2;
        s1.push(10);
        s2 = s1;
        if (s2.top() != 10) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 15: Self assignment
    std::cout << "Test 15: Self assignment... ";
    {
        Stack<int> s;
        s.push(10);
        s = s;
        if (s.top() != 10) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    std::cout << "\n=================================================\n";
    std::cout << "  RESULTS: " << tests_passed << "/" << tests_total << " tests passed\n";
    std::cout << "=================================================\n";
    std::cout << "✓ All tests PASSED!\n";
    
    return 0;
}
