/**
 * @file unordered_map_test.cpp
 * @brief Test suite for the UnorderedMap (hash table, separate chaining).
 */

#include "unordered_map/unordered_map.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

int main() {
    std::cout << "=================================================\n";
    std::cout << "    UnorderedMap Implementation Test Suite      \n";
    std::cout << "=================================================\n\n";

    int passed = 0, total = 11;

    // Test 1: operator[] inserts then reads
    std::cout << "Test 1: operator[] insert/read... ";
    {
        UnorderedMap<int, std::string> m;
        m[1] = "one";
        m[2] = "two";
        if (m.size() != 2 || m[1] != "one" || m[2] != "two") { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 2: operator[] on missing key default-constructs
    std::cout << "Test 2: operator[] default-insert... ";
    {
        UnorderedMap<int, int> m;
        if (m[42] != 0) { std::cout << "FAILED\n"; return 1; }   // value-initialized int
        if (m.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 3: operator[] updates existing value (no new entry)
    std::cout << "Test 3: operator[] update... ";
    {
        UnorderedMap<int, std::string> m;
        m[1] = "one";
        m[1] = "ONE";
        if (m[1] != "ONE" || m.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 4: insert returns false for duplicate key
    std::cout << "Test 4: insert dup returns false... ";
    {
        UnorderedMap<int, std::string> m;
        if (!m.insert({1, "one"})) { std::cout << "FAILED\n"; return 1; }
        if (m.insert({1, "uno"})) { std::cout << "FAILED\n"; return 1; }  // dup key
        if (m[1] != "one" || m.size() != 1) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 5: at() returns value / throws on missing
    std::cout << "Test 5: at() + throw... ";
    {
        UnorderedMap<int, std::string> m;
        m[5] = "five";
        if (m.at(5) != "five") { std::cout << "FAILED\n"; return 1; }
        bool threw = false;
        try { (void)m.at(99); } catch (const std::out_of_range&) { threw = true; }
        if (!threw) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 6: contains / count
    std::cout << "Test 6: contains/count... ";
    {
        UnorderedMap<int, int> m = {{1, 10}, {2, 20}};
        if (!m.contains(1) || m.contains(3)) { std::cout << "FAILED\n"; return 1; }
        if (m.count(2) != 1 || m.count(3) != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 7: erase
    std::cout << "Test 7: erase... ";
    {
        UnorderedMap<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
        if (m.erase(2) != 1 || m.contains(2) || m.size() != 2) { std::cout << "FAILED\n"; return 1; }
        if (m.erase(99) != 0) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 8: empty / clear
    std::cout << "Test 8: empty/clear... ";
    {
        UnorderedMap<int, int> m = {{1, 1}};
        if (m.empty()) { std::cout << "FAILED\n"; return 1; }
        m.clear();
        if (!m.empty() || m.size() != 0 || m.contains(1)) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 9: rehash correctness under heavy load
    std::cout << "Test 9: rehash with 1000 keys... ";
    {
        UnorderedMap<int, int> m;
        for (int i = 0; i < 1000; ++i) m[i] = i * i;
        if (m.size() != 1000) { std::cout << "FAILED\n"; return 1; }
        bool all = true;
        for (int i = 0; i < 1000; ++i) if (m[i] != i * i) all = false;
        if (!all) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 10: string keys
    std::cout << "Test 10: string keys... ";
    {
        UnorderedMap<std::string, int> ages;
        ages["alice"] = 30;
        ages["bob"] = 25;
        if (ages.at("alice") != 30 || ages["bob"] != 25) { std::cout << "FAILED\n"; return 1; }
    }
    std::cout << "PASSED\n"; ++passed;

    // Test 11: init-list construction
    std::cout << "Test 11: init-list construction... ";
    {
        UnorderedMap<int, std::string> m = {{1, "a"}, {2, "b"}, {3, "c"}};
        if (m.size() != 3 || m.at(3) != "c") { std::cout << "FAILED\n"; return 1; }
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
    UnorderedMap Implementation Test Suite      
=================================================

Test 1: operator[] insert/read... PASSED
Test 2: operator[] default-insert... PASSED
Test 3: operator[] update... PASSED
Test 4: insert dup returns false... PASSED
Test 5: at() + throw... PASSED
Test 6: contains/count... PASSED
Test 7: erase... PASSED
Test 8: empty/clear... PASSED
Test 9: rehash with 1000 keys... PASSED
Test 10: string keys... PASSED
Test 11: init-list construction... PASSED

=================================================
  RESULTS: 11/11 tests passed
=================================================
✓ All tests PASSED!
 * ============================================================================ */
