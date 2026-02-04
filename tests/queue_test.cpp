/**
 * @file queue_test.cpp
 * @brief Test suite for Queue implementation
 */

#include "queue/queue.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=================================================\n";
    std::cout << "       Queue Implementation Test Suite          \n";
    std::cout << "=================================================\n\n";
    
    int tests_passed = 0;
    int tests_total = 15;
    
    // Test 1: Basic push and front
    std::cout << "Test 1: Basic push and front... ";
    {
        Queue<int> q;
        q.push(10);
        q.push(20);
        if (q.front() != 10 || q.size() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 2: Pop operation
    std::cout << "Test 2: Pop operation... ";
    {
        Queue<int> q;
        q.push(10);
        q.push(20);
        q.pop();
        if (q.front() != 20 || q.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 3: Empty queue
    std::cout << "Test 3: Empty queue... ";
    {
        Queue<int> q;
        if (!q.empty() || q.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 4: FIFO order
    std::cout << "Test 4: FIFO order... ";
    {
        Queue<int> q;
        q.push(1);
        q.push(2);
        q.push(3);
        if (q.front() != 1) { std::cout << "FAILED\n"; return 1; }
        q.pop();
        if (q.front() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 5: Front and back
    std::cout << "Test 5: Front and back... ";
    {
        Queue<int> q;
        q.push(10);
        q.push(20);
        q.push(30);
        if (q.front() != 10 || q.back() != 30) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 6: Copy constructor
    std::cout << "Test 6: Copy constructor... ";
    {
        Queue<int> q1;
        q1.push(10);
        q1.push(20);
        Queue<int> q2 = q1;
        if (q2.front() != 10 || q2.size() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 7: Move constructor
    std::cout << "Test 7: Move constructor... ";
    {
        Queue<int> q1;
        q1.push(10);
        q1.push(20);
        Queue<int> q2 = std::move(q1);
        if (q2.front() != 10 || !q1.empty()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 8: With strings
    std::cout << "Test 8: With strings... ";
    {
        Queue<std::string> q;
        q.push("hello");
        q.push("world");
        if (q.front() != "hello" || q.back() != "world") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 9: Emplace
    std::cout << "Test 9: Emplace... ";
    {
        Queue<std::string> q;
        q.emplace("test");
        if (q.front() != "test") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 10: Swap
    std::cout << "Test 10: Swap... ";
    {
        Queue<int> q1, q2;
        q1.push(10);
        q2.push(20);
        q1.swap(q2);
        if (q1.front() != 20 || q2.front() != 10) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 11: Comparison
    std::cout << "Test 11: Comparison... ";
    {
        Queue<int> q1, q2;
        q1.push(10);
        q2.push(10);
        if (!(q1 == q2)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 12: Large queue
    std::cout << "Test 12: Large queue... ";
    {
        Queue<int> q;
        for (int i = 0; i < 100; ++i) q.push(i);
        if (q.size() != 100 || q.front() != 0 || q.back() != 99) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 13: Push move
    std::cout << "Test 13: Push move... ";
    {
        Queue<std::string> q;
        std::string str = "move_me";
        q.push(std::move(str));
        if (q.front() != "move_me") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 14: Multiple pops
    std::cout << "Test 14: Multiple pops... ";
    {
        Queue<int> q;
        q.push(1);
        q.push(2);
        q.push(3);
        q.pop();
        q.pop();
        q.pop();
        if (!q.empty()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 15: Assignment
    std::cout << "Test 15: Assignment... ";
    {
        Queue<int> q1, q2;
        q1.push(10);
        q2 = q1;
        if (q2.front() != 10) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    std::cout << "\n=================================================\n";
    std::cout << "  RESULTS: " << tests_passed << "/" << tests_total << " tests passed\n";
    std::cout << "=================================================\n";
    std::cout << "✓ All tests PASSED!\n";
    
    return 0;
}
