/**
 * @file deque_test.cpp
 * @brief Test suite for Deque implementation
 */

#include "deque/deque.hpp"
#include <iostream>

int main() {
    std::cout << "=================================================\n";
    std::cout << "     Deque Implementation Test Suite            \n";
    std::cout << "=================================================\n\n";
    
    std::cout << "Running deque tests...\n";
    std::cout << "(Basic operations validated through examples)\n\n";
    
    // Basic smoke tests
    Deque<int> dq;
    dq.push_back(1);
    dq.push_front(0);
    dq.push_back(2);
    
    if (dq.size() != 3) {
        std::cout << "✗ Test FAILED\n";
        return 1;
    }
    
    if (dq[0] != 0 || dq[1] != 1 || dq[2] != 2) {
        std::cout << "✗ Test FAILED\n";
        return 1;
    }
    
    dq.pop_front();
    dq.pop_back();
    
    if (dq.size() != 1 || dq[0] != 1) {
        std::cout << "✗ Test FAILED\n";
        return 1;
    }
    
    std::cout << "=================================================\n";
    std::cout << "  RESULTS: Basic tests passed                    \n";
    std::cout << "  (See deque_example for comprehensive demos)   \n";
    std::cout << "=================================================\n";
    std::cout << "✓ All tests PASSED!\n";
    
    return 0;
}
