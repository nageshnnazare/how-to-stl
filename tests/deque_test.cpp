/**
 * @file deque_test.cpp
 * @brief Test suite for the Deque (chunked double-ended queue).
 */

#include "deque/deque.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

int main() {
    std::cout << "=================================================\n";
    std::cout << "      Deque Implementation Test Suite           \n";
    std::cout << "=================================================\n\n";

    int passed = 0, total = 14;

    // Test 1: push_back / push_front ordering
    std::cout << "Test 1: push both ends... ";
    {
        Deque<int> dq;
        dq.push_back(1);
        dq.push_front(0);
        dq.push_back(2);
        if (dq.size() != 3 || dq[0] != 0 || dq[1] != 1 || dq[2] != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 2: front / back
    std::cout << "Test 2: front/back... ";
    {
        Deque<int> dq = {10, 20, 30};
        if (dq.front() != 10 || dq.back() != 30) { std::cout << "FAILED\n"; return 1; }
        dq.front() = 99;
        dq.back() = 88;
        if (dq.front() != 99 || dq.back() != 88) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 3: pop_front / pop_back
    std::cout << "Test 3: pop both ends... ";
    {
        Deque<int> dq = {1, 2, 3, 4, 5};
        dq.pop_front();
        dq.pop_back();
        if (dq.size() != 3 || dq.front() != 2 || dq.back() != 4) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 4: random access
    std::cout << "Test 4: operator[]... ";
    {
        Deque<int> dq;
        for (int i = 0; i < 10; ++i) dq.push_back(i * 2);
        if (dq[0] != 0 || dq[5] != 10 || dq[9] != 18) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 5: at() bounds checking
    std::cout << "Test 5: at() throws... ";
    {
        Deque<int> dq = {1, 2, 3};
        if (dq.at(1) != 2) { std::cout << "FAILED\n"; return 1; }
        bool threw = false;
        try { (void)dq.at(99); } catch (const std::out_of_range&) { threw = true; }
        if (!threw) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 6: heavy push_back forces multiple chunks
    std::cout << "Test 6: 1000 push_back (chunk growth)... ";
    {
        Deque<int> dq;
        for (int i = 0; i < 1000; ++i) dq.push_back(i);
        if (dq.size() != 1000 || dq.front() != 0 || dq.back() != 999) { std::cout << "FAILED\n"; return 1; }
        bool ok = true;
        for (int i = 0; i < 1000; ++i) if (dq[i] != i) ok = false;
        if (!ok) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 7: heavy push_front keeps reverse order
    std::cout << "Test 7: 1000 push_front... ";
    {
        Deque<int> dq;
        for (int i = 0; i < 1000; ++i) dq.push_front(i);
        if (dq.size() != 1000 || dq.front() != 999 || dq.back() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 8: interleaved push front/back
    std::cout << "Test 8: interleaved push... ";
    {
        Deque<int> dq;
        for (int i = 0; i < 100; ++i) { dq.push_back(i); dq.push_front(-i); }
        if (dq.size() != 200 || dq.front() != -99 || dq.back() != 99) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 9: range-based iteration
    std::cout << "Test 9: iteration sum... ";
    {
        Deque<int> dq = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int x : dq) sum += x;
        if (sum != 15) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 10: clear / empty
    std::cout << "Test 10: clear/empty... ";
    {
        Deque<int> dq = {1, 2, 3};
        if (dq.empty()) { std::cout << "FAILED\n"; return 1; }
        dq.clear();
        if (!dq.empty() || dq.size() != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 11: resize grow and shrink
    std::cout << "Test 11: resize... ";
    {
        Deque<int> dq = {1, 2, 3};
        dq.resize(5, 7);
        if (dq.size() != 5 || dq[3] != 7 || dq[4] != 7) { std::cout << "FAILED\n"; return 1; }
        dq.resize(2);
        if (dq.size() != 2 || dq.back() != 2) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 12: swap
    std::cout << "Test 12: swap... ";
    {
        Deque<int> a = {1, 2};
        Deque<int> b = {9, 8, 7};
        a.swap(b);
        if (a.size() != 3 || b.size() != 2 || a[0] != 9 || b[0] != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 13: copy independence
    std::cout << "Test 13: copy independence... ";
    {
        Deque<int> a = {1, 2, 3};
        Deque<int> b = a;
        b.push_back(4);
        if (a.size() != 3 || b.size() != 4) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 14: emplace + string payloads
    std::cout << "Test 14: emplace/strings... ";
    {
        Deque<std::string> dq;
        dq.emplace_back("world");
        dq.emplace_front("hello");
        if (dq.front() != "hello" || dq.back() != "world") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    std::cout << "\n=================================================\n";
    std::cout << "  RESULTS: " << passed << "/" << total << " tests passed\n";
    std::cout << "=================================================\n";
    if (passed == total) { std::cout << "\xE2\x9C\x93 All tests PASSED!\n"; return 0; }
    std::cout << "\xE2\x9C\x97 Some tests FAILED\n";
    return 1;
}

/* ===== EXPECTED OUTPUT ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
=================================================
      Deque Implementation Test Suite           
=================================================

Test 1: push both ends... PASSED
Test 2: front/back... PASSED
Test 3: pop both ends... PASSED
Test 4: operator[]... PASSED
Test 5: at() throws... PASSED
Test 6: 1000 push_back (chunk growth)... PASSED
Test 7: 1000 push_front... PASSED
Test 8: interleaved push... PASSED
Test 9: iteration sum... PASSED
Test 10: clear/empty... PASSED
Test 11: resize... PASSED
Test 12: swap... PASSED
Test 13: copy independence... PASSED
Test 14: emplace/strings... PASSED

=================================================
  RESULTS: 14/14 tests passed
=================================================
✓ All tests PASSED!
 * ============================================================================ */
