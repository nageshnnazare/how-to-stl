#include "../bitset/bitset.hpp"
#include <iostream>

// Test framework macros
#define ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "Assertion failed at line " << __LINE__ << ": " << message << std::endl; \
        return false; \
    }

#define RUN_TEST(test_func) \
    std::cout << "Running " << #test_func << "... "; \
    if (test_func()) { \
        std::cout << "✅ PASSED\n"; \
        passed++; \
    } else { \
        std::cout << "❌ FAILED\n"; \
        failed++; \
    }

// Test 1: Default Construction
bool test_default_construction() {
    Bitset<8> bs;
    ASSERT(bs.size() == 8, "Size should be 8");
    ASSERT(bs.count() == 0, "Count should be 0");
    ASSERT(bs.none(), "Should be none");
    ASSERT(!bs.any(), "Should not be any");
    return true;
}

// Test 2: Value Construction
bool test_value_construction() {
    Bitset<8> bs(0b10101010);
    ASSERT(bs.count() == 4, "Should have 4 bits set");
    ASSERT(bs[1] == true, "Bit 1 should be set");
    ASSERT(bs[3] == true, "Bit 3 should be set");
    ASSERT(bs[0] == false, "Bit 0 should be clear");
    return true;
}

// Test 3: Set Operations
bool test_set() {
    Bitset<8> bs;
    bs.set(0);
    ASSERT(bs[0] == true, "Bit 0 should be set");
    ASSERT(bs.count() == 1, "Count should be 1");
    
    bs.set(7);
    ASSERT(bs[7] == true, "Bit 7 should be set");
    ASSERT(bs.count() == 2, "Count should be 2");
    
    bs.set(0, false);
    ASSERT(bs[0] == false, "Bit 0 should be clear");
    ASSERT(bs.count() == 1, "Count should be 1");
    
    return true;
}

// Test 4: Reset Operations
bool test_reset() {
    Bitset<8> bs(0xFF);  // All ones
    ASSERT(bs.count() == 8, "Should have 8 bits set");
    
    bs.reset(0);
    ASSERT(bs[0] == false, "Bit 0 should be clear");
    ASSERT(bs.count() == 7, "Count should be 7");
    
    bs.reset();  // Reset all
    ASSERT(bs.count() == 0, "All bits should be clear");
    ASSERT(bs.none(), "Should be none");
    
    return true;
}

// Test 5: Flip Operations
bool test_flip() {
    Bitset<8> bs;
    bs.flip(0);
    ASSERT(bs[0] == true, "Bit 0 should be set after flip");
    
    bs.flip(0);
    ASSERT(bs[0] == false, "Bit 0 should be clear after second flip");
    
    return true;
}

// Test 6: Test Method
bool test_test() {
    Bitset<8> bs(0b10101010);
    ASSERT(bs.test(1) == true, "Bit 1 should be set");
    ASSERT(bs.test(0) == false, "Bit 0 should be clear");
    ASSERT(bs.test(7) == true, "Bit 7 should be set");
    return true;
}

// Test 7: Count
bool test_count() {
    Bitset<8> bs1;
    ASSERT(bs1.count() == 0, "Empty should have count 0");
    
    Bitset<8> bs2(0xFF);
    ASSERT(bs2.count() == 8, "Full should have count 8");
    
    Bitset<8> bs3(0b00001111);
    ASSERT(bs3.count() == 4, "Should have count 4");
    
    return true;
}

// Test 8: all/any/none
bool test_all_any_none() {
    Bitset<8> empty;
    ASSERT(empty.none(), "Empty should be none");
    ASSERT(!empty.any(), "Empty should not be any");
    ASSERT(!empty.all(), "Empty should not be all");
    
    Bitset<8> full(0xFF);
    ASSERT(!full.none(), "Full should not be none");
    ASSERT(full.any(), "Full should be any");
    ASSERT(full.all(), "Full should be all");
    
    Bitset<8> partial(0b00001000);
    ASSERT(!partial.none(), "Partial should not be none");
    ASSERT(partial.any(), "Partial should be any");
    ASSERT(!partial.all(), "Partial should not be all");
    
    return true;
}

// Test 9: Bitwise AND
bool test_and() {
    Bitset<8> bs1(0b11110000);
    Bitset<8> bs2(0b10101010);
    auto result = bs1 & bs2;
    
    ASSERT(result.count() == 2, "AND should have 2 bits set");
    ASSERT(result[7] == true, "Bit 7 should be set");
    ASSERT(result[5] == true, "Bit 5 should be set");
    ASSERT(result[0] == false, "Bit 0 should be clear");
    
    return true;
}

// Test 10: Bitwise OR
bool test_or() {
    Bitset<8> bs1(0b11110000);
    Bitset<8> bs2(0b00001111);
    auto result = bs1 | bs2;
    
    ASSERT(result.count() == 8, "OR should have 8 bits set");
    ASSERT(result.all(), "Result should be all ones");
    
    return true;
}

// Test 11: Bitwise XOR
bool test_xor() {
    Bitset<8> bs1(0b11110000);
    Bitset<8> bs2(0b11001100);
    auto result = bs1 ^ bs2;
    
    // XOR: 11110000 ^ 11001100 = 00111100
    ASSERT(result.count() == 4, "XOR should have 4 bits set");
    ASSERT(result[2] == true, "Bit 2 should be set");
    ASSERT(result[3] == true, "Bit 3 should be set");
    ASSERT(result[4] == true, "Bit 4 should be set");
    ASSERT(result[5] == true, "Bit 5 should be set");
    
    return true;
}

// Test 12: Bitwise NOT
bool test_not() {
    Bitset<8> bs(0b00001111);
    auto result = ~bs;
    
    ASSERT(result.count() == 4, "NOT should have 4 bits set");
    ASSERT(result[0] == false, "Bit 0 should be clear");
    ASSERT(result[4] == true, "Bit 4 should be set");
    ASSERT(result[7] == true, "Bit 7 should be set");
    
    return true;
}

// Test 13: Chaining Operations
bool test_chaining() {
    Bitset<8> bs;
    bs.set(0).set(1).set(2);
    
    ASSERT(bs.count() == 3, "Should have 3 bits set");
    ASSERT(bs[0] && bs[1] && bs[2], "Bits 0, 1, 2 should be set");
    
    return true;
}

// Test 14: Large Bitset
bool test_large_bitset() {
    Bitset<64> bs;
    bs.set(0).set(31).set(63);
    
    ASSERT(bs.size() == 64, "Size should be 64");
    ASSERT(bs.count() == 3, "Should have 3 bits set");
    ASSERT(bs[0] && bs[31] && bs[63], "Bits 0, 31, 63 should be set");
    
    return true;
}

// Test 15: Operator []
bool test_operator_brackets() {
    Bitset<8> bs(0b10101010);
    
    ASSERT(bs[1] == true, "bs[1] should be true");
    ASSERT(bs[0] == false, "bs[0] should be false");
    ASSERT(bs[7] == true, "bs[7] should be true");
    
    // Out of bounds should return false
    ASSERT(bs[100] == false, "Out of bounds should be false");
    
    return true;
}

// Test 16: Complex Operations
bool test_complex_operations() {
    Bitset<8> a(0b11110000);
    Bitset<8> b(0b10101010);
    Bitset<8> c(0b00001111);
    
    // (a & b) | c
    auto result = (a & b) | c;
    
    // a & b = 10100000, (a & b) | c = 10101111
    ASSERT(result.count() == 6, "Should have 6 bits set");
    
    return true;
}

// Test 17: Zero-sized (edge case)
bool test_edge_cases() {
    Bitset<1> bs1;
    ASSERT(bs1.size() == 1, "Size should be 1");
    
    bs1.set(0);
    ASSERT(bs1.count() == 1, "Count should be 1");
    ASSERT(bs1.all(), "Single bit set should be all");
    
    return true;
}

int main() {
    int passed = 0;
    int failed = 0;
    
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║        Bitset Container Test Suite             ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    RUN_TEST(test_default_construction);
    RUN_TEST(test_value_construction);
    RUN_TEST(test_set);
    RUN_TEST(test_reset);
    RUN_TEST(test_flip);
    RUN_TEST(test_test);
    RUN_TEST(test_count);
    RUN_TEST(test_all_any_none);
    RUN_TEST(test_and);
    RUN_TEST(test_or);
    RUN_TEST(test_xor);
    RUN_TEST(test_not);
    RUN_TEST(test_chaining);
    RUN_TEST(test_large_bitset);
    RUN_TEST(test_operator_brackets);
    RUN_TEST(test_complex_operations);
    RUN_TEST(test_edge_cases);
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return failed == 0 ? 0 : 1;
}


/* ===== EXPECTED OUTPUT ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║        Bitset Container Test Suite             ║
╚════════════════════════════════════════════════╝

Running test_default_construction... ✅ PASSED
Running test_value_construction... ✅ PASSED
Running test_set... ✅ PASSED
Running test_reset... ✅ PASSED
Running test_flip... ✅ PASSED
Running test_test... ✅ PASSED
Running test_count... ✅ PASSED
Running test_all_any_none... ✅ PASSED
Running test_and... ✅ PASSED
Running test_or... ✅ PASSED
Running test_xor... ✅ PASSED
Running test_not... ✅ PASSED
Running test_chaining... ✅ PASSED
Running test_large_bitset... ✅ PASSED
Running test_operator_brackets... ✅ PASSED
Running test_complex_operations... ✅ PASSED
Running test_edge_cases... ✅ PASSED

════════════════════════════════════════════════
Test Results: 17 passed, 0 failed
════════════════════════════════════════════════
 * ============================================================================ */
