/**
 * @file priority_queue_test.cpp
 * @brief Test suite for PriorityQueue implementation
 */

#include "priority_queue/priority_queue.hpp"
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::cout << "=================================================\n";
    std::cout << "    PriorityQueue Implementation Test Suite    \n";
    std::cout << "=================================================\n\n";
    
    int tests_passed = 0;
    int tests_total = 20;
    
    // Test 1: Basic push and top
    std::cout << "Test 1: Basic push and top... ";
    {
        PriorityQueue<int> pq;
        pq.push(10);
        pq.push(30);
        pq.push(20);
        if (pq.top() != 30 || pq.size() != 3) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 2: Pop operation
    std::cout << "Test 2: Pop operation... ";
    {
        PriorityQueue<int> pq;
        pq.push(10);
        pq.push(30);
        pq.push(20);
        pq.pop();
        if (pq.top() != 20 || pq.size() != 2) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 3: Empty queue
    std::cout << "Test 3: Empty queue... ";
    {
        PriorityQueue<int> pq;
        if (!pq.empty() || pq.size() != 0) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 4: Min heap with custom comparator
    std::cout << "Test 4: Min heap... ";
    {
        PriorityQueue<int, std::vector<int>, std::greater<int>> pq;
        pq.push(30);
        pq.push(10);
        pq.push(20);
        if (pq.top() != 10) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 5: Range constructor
    std::cout << "Test 5: Range constructor... ";
    {
        std::vector<int> v = {5, 2, 8, 1, 9};
        PriorityQueue<int> pq(v.begin(), v.end());
        if (pq.top() != 9 || pq.size() != 5) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 6: Heap property verification
    std::cout << "Test 6: Heap property... ";
    {
        PriorityQueue<int> pq;
        for (int i = 0; i < 10; ++i) {
            pq.push(i);
        }
        if (!pq.is_heap()) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 7: Pop until empty
    std::cout << "Test 7: Pop until empty... ";
    {
        PriorityQueue<int> pq;
        pq.push(1);
        pq.push(2);
        pq.push(3);
        pq.pop();
        pq.pop();
        pq.pop();
        if (!pq.empty()) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 8: Copy constructor
    std::cout << "Test 8: Copy constructor... ";
    {
        PriorityQueue<int> pq1;
        pq1.push(10);
        pq1.push(20);
        PriorityQueue<int> pq2 = pq1;
        if (pq2.top() != 20 || pq2.size() != 2) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 9: Move constructor
    std::cout << "Test 9: Move constructor... ";
    {
        PriorityQueue<int> pq1;
        pq1.push(10);
        pq1.push(20);
        PriorityQueue<int> pq2 = std::move(pq1);
        if (pq2.top() != 20 || !pq1.empty()) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 10: Large number of elements
    std::cout << "Test 10: Large heap... ";
    {
        PriorityQueue<int> pq;
        for (int i = 0; i < 100; ++i) {
            pq.push(i);
        }
        if (pq.top() != 99 || pq.size() != 100) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 11: Descending order extraction
    std::cout << "Test 11: Descending order... ";
    {
        PriorityQueue<int> pq;
        pq.push(5);
        pq.push(2);
        pq.push(8);
        pq.push(1);
        int prev = pq.top();
        pq.pop();
        bool sorted = true;
        while (!pq.empty()) {
            if (pq.top() > prev) sorted = false;
            prev = pq.top();
            pq.pop();
        }
        if (!sorted) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 12: Emplace
    std::cout << "Test 12: Emplace... ";
    {
        struct Point {
            int x, y;
            Point(int x_, int y_) : x(x_), y(y_) {}
        };
        auto cmp = [](const Point& a, const Point& b) {
            return (a.x + a.y) < (b.x + b.y);
        };
        PriorityQueue<Point, std::vector<Point>, decltype(cmp)> pq(cmp);
        pq.emplace(1, 2);
        pq.emplace(3, 4);
        if ((pq.top().x + pq.top().y) != 7) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 13: Swap
    std::cout << "Test 13: Swap... ";
    {
        PriorityQueue<int> pq1, pq2;
        pq1.push(10);
        pq2.push(20);
        pq1.swap(pq2);
        if (pq1.top() != 20 || pq2.top() != 10) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 14: With strings
    std::cout << "Test 14: With strings... ";
    {
        PriorityQueue<std::string> pq;
        pq.push("apple");
        pq.push("zebra");
        pq.push("banana");
        if (pq.top() != "zebra") { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 15: Duplicates
    std::cout << "Test 15: Duplicates... ";
    {
        PriorityQueue<int> pq;
        pq.push(5);
        pq.push(5);
        pq.push(5);
        if (pq.size() != 3 || pq.top() != 5) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 16: Single element
    std::cout << "Test 16: Single element... ";
    {
        PriorityQueue<int> pq;
        pq.push(42);
        if (pq.top() != 42 || pq.size() != 1) { 
            std::cout << "FAILED\n"; return 1; 
        }
        pq.pop();
        if (!pq.empty()) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 17: Heap after multiple operations
    std::cout << "Test 17: Heap after ops... ";
    {
        PriorityQueue<int> pq;
        pq.push(10);
        pq.push(30);
        pq.pop();
        pq.push(20);
        pq.push(40);
        pq.pop();
        if (!pq.is_heap()) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 18: Build heap from container
    std::cout << "Test 18: Build from container... ";
    {
        std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
        PriorityQueue<int> pq(std::less<int>(), v);
        if (pq.top() != 9 || !pq.is_heap()) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 19: Move element
    std::cout << "Test 19: Move element... ";
    {
        PriorityQueue<std::string> pq;
        std::string s = "hello";
        pq.push(std::move(s));
        if (pq.top() != "hello") { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    // Test 20: Exception on empty
    std::cout << "Test 20: Exception on empty... ";
    {
        PriorityQueue<int> pq;
        bool caught = false;
        try {
            pq.top();
        } catch (const std::out_of_range&) {
            caught = true;
        }
        if (!caught) { 
            std::cout << "FAILED\n"; return 1; 
        }
    }
    std::cout << "PASSED\n";
    ++tests_passed;
    
    std::cout << "\n=================================================\n";
    std::cout << "  RESULTS: " << tests_passed << "/" << tests_total << " tests passed\n";
    std::cout << "=================================================\n";
    std::cout << "✓ All tests PASSED!\n";
    
    return 0;
}


/* ===== EXPECTED OUTPUT ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
=================================================
    PriorityQueue Implementation Test Suite    
=================================================

Test 1: Basic push and top... PASSED
Test 2: Pop operation... PASSED
Test 3: Empty queue... PASSED
Test 4: Min heap... PASSED
Test 5: Range constructor... PASSED
Test 6: Heap property... PASSED
Test 7: Pop until empty... PASSED
Test 8: Copy constructor... PASSED
Test 9: Move constructor... PASSED
Test 10: Large heap... PASSED
Test 11: Descending order... PASSED
Test 12: Emplace... PASSED
Test 13: Swap... PASSED
Test 14: With strings... PASSED
Test 15: Duplicates... PASSED
Test 16: Single element... PASSED
Test 17: Heap after ops... PASSED
Test 18: Build from container... PASSED
Test 19: Move element... PASSED
Test 20: Exception on empty... PASSED

=================================================
  RESULTS: 20/20 tests passed
=================================================
✓ All tests PASSED!
 * ============================================================================ */
