/**
 * @file map_test.cpp
 * @brief Test suite for the Map (Red-Black tree of key->value pairs).
 */

#include "map/map.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

int main() {
    std::cout << "=================================================\n";
    std::cout << "        Map Implementation Test Suite           \n";
    std::cout << "=================================================\n\n";

    int passed = 0, total = 13;

    // Test 1: operator[] insert and read
    std::cout << "Test 1: operator[] insert/read... ";
    {
        Map<int, std::string> m;
        m[1] = "one";
        m[2] = "two";
        if (m.size() != 2 || m[1] != "one") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 2: operator[] default-inserts missing keys
    std::cout << "Test 2: operator[] default-insert... ";
    {
        Map<std::string, int> m;
        if (m["ghost"] != 0 || m.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 3: operator[] updates without adding
    std::cout << "Test 3: operator[] update... ";
    {
        Map<int, std::string> m;
        m[1] = "one";
        m[1] = "ONE";
        if (m[1] != "ONE" || m.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 4: insert returns {it, true} for new, {it, false} for dup
    std::cout << "Test 4: insert return pair... ";
    {
        Map<int, std::string> m;
        auto r1 = m.insert({1, "one"});
        auto r2 = m.insert({1, "uno"});
        if (!r1.second || r2.second) { std::cout << "FAILED\n"; return 1; }
        if (m[1] != "one" || m.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 5: at() returns / throws
    std::cout << "Test 5: at() + throw... ";
    {
        Map<int, std::string> m;
        m[7] = "seven";
        if (m.at(7) != "seven") { std::cout << "FAILED\n"; return 1; }
        bool threw = false;
        try { (void)m.at(99); } catch (const std::out_of_range&) { threw = true; }
        if (!threw) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 6: keys iterate in sorted order
    std::cout << "Test 6: ordered iteration... ";
    {
        Map<int, int> m;
        m[5] = 0; m[1] = 0; m[3] = 0; m[2] = 0; m[4] = 0;
        int prev = -1; bool sorted = true;
        for (const auto& kv : m) { if (kv.first <= prev) sorted = false; prev = kv.first; }
        if (!sorted) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 7: find
    std::cout << "Test 7: find()... ";
    {
        Map<int, std::string> m = {{1, "a"}, {2, "b"}};
        auto it = m.find(2);
        if (it == m.end() || it->second != "b") { std::cout << "FAILED\n"; return 1; }
        if (m.find(99) != m.end()) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 8: erase
    std::cout << "Test 8: erase... ";
    {
        Map<int, int> m = {{1, 1}, {2, 2}, {3, 3}};
        if (m.erase(2) != 1 || m.contains(2) || m.size() != 2) { std::cout << "FAILED\n"; return 1; }
        if (m.erase(99) != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 9: contains / count
    std::cout << "Test 9: contains/count... ";
    {
        Map<int, int> m = {{1, 1}, {2, 2}};
        if (!m.contains(1) || m.contains(3)) { std::cout << "FAILED\n"; return 1; }
        if (m.count(2) != 1 || m.count(3) != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 10: empty / clear
    std::cout << "Test 10: empty/clear... ";
    {
        Map<int, int> m = {{1, 1}};
        if (m.empty()) { std::cout << "FAILED\n"; return 1; }
        m.clear();
        if (!m.empty() || m.size() != 0 || m.contains(1)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 11: copy is independent
    std::cout << "Test 11: copy independence... ";
    {
        Map<int, int> a = {{1, 1}, {2, 2}};
        Map<int, int> b = a;
        b[1] = 100;
        if (a[1] != 1 || b[1] != 100) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 12: stress insert keeps tree valid (sorted + correct size)
    std::cout << "Test 12: 500-key stress... ";
    {
        Map<int, int> m;
        for (int i = 500; i > 0; --i) m[i] = i;   // reverse insert exercises rotations
        if (m.size() != 500) { std::cout << "FAILED\n"; return 1; }
        int prev = 0; bool sorted = true;
        for (const auto& kv : m) { if (kv.first <= prev) sorted = false; prev = kv.first; }
        if (!sorted || m.at(250) != 250) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 13: string keys sort lexicographically
    std::cout << "Test 13: string keys... ";
    {
        Map<std::string, int> m;
        m["charlie"] = 3; m["alice"] = 1; m["bob"] = 2;
        std::string prev; bool sorted = true;
        for (const auto& kv : m) { if (kv.first < prev) sorted = false; prev = kv.first; }
        if (!sorted || m["alice"] != 1) { std::cout << "FAILED\n"; return 1; }
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
        Map Implementation Test Suite           
=================================================

Test 1: operator[] insert/read... PASSED
Test 2: operator[] default-insert... PASSED
Test 3: operator[] update... PASSED
Test 4: insert return pair... PASSED
Test 5: at() + throw... PASSED
Test 6: ordered iteration... PASSED
Test 7: find()... PASSED
Test 8: erase... PASSED
Test 9: contains/count... PASSED
Test 10: empty/clear... PASSED
Test 11: copy independence... PASSED
Test 12: 500-key stress... PASSED
Test 13: string keys... PASSED

=================================================
  RESULTS: 13/13 tests passed
=================================================
✓ All tests PASSED!
 * ============================================================================ */
