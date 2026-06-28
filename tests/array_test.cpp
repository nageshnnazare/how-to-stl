#include "../array/array.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

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

// Test 1: Construction and Size
bool test_construction_and_size() {
    Array<int, 5> arr = {1, 2, 3, 4, 5};
    ASSERT(arr.size() == 5, "Size should be 5");
    ASSERT(!arr.empty(), "Array should not be empty");
    
    Array<int, 0> empty;
    ASSERT(empty.size() == 0, "Zero-sized array should have size 0");
    ASSERT(empty.empty(), "Zero-sized array should be empty");
    
    return true;
}

// Test 2: Element Access
bool test_element_access() {
    Array<int, 5> arr = {10, 20, 30, 40, 50};
    
    ASSERT(arr[0] == 10, "arr[0] should be 10");
    ASSERT(arr[2] == 30, "arr[2] should be 30");
    ASSERT(arr[4] == 50, "arr[4] should be 50");
    
    ASSERT(arr.front() == 10, "front() should be 10");
    ASSERT(arr.back() == 50, "back() should be 50");
    
    return true;
}

// Test 3: Bounds-checked Access
bool test_at_bounds_checking() {
    Array<int, 3> arr = {1, 2, 3};
    
    bool caught = false;
    try {
        arr.at(5);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    ASSERT(caught, "at() should throw out_of_range for invalid index");
    
    ASSERT(arr.at(0) == 1, "at(0) should return 1");
    ASSERT(arr.at(2) == 3, "at(2) should return 3");
    
    return true;
}

// Test 4: Modification
bool test_modification() {
    Array<int, 4> arr = {0, 0, 0, 0};
    
    arr[0] = 100;
    arr[1] = 200;
    arr[2] = 300;
    arr[3] = 400;
    
    ASSERT(arr[0] == 100, "arr[0] should be 100");
    ASSERT(arr[1] == 200, "arr[1] should be 200");
    ASSERT(arr[2] == 300, "arr[2] should be 300");
    ASSERT(arr[3] == 400, "arr[3] should be 400");
    
    arr.front() = 999;
    arr.back() = 888;
    
    ASSERT(arr.front() == 999, "front() should be 999");
    ASSERT(arr.back() == 888, "back() should be 888");
    
    return true;
}

// Test 5: Fill
bool test_fill() {
    Array<int, 6> arr;
    arr.fill(42);
    
    for (size_t i = 0; i < arr.size(); ++i) {
        ASSERT(arr[i] == 42, "All elements should be 42");
    }
    
    arr.fill(0);
    for (size_t i = 0; i < arr.size(); ++i) {
        ASSERT(arr[i] == 0, "All elements should be 0");
    }
    
    return true;
}

// Test 6: Iterators
bool test_iterators() {
    Array<int, 5> arr = {1, 2, 3, 4, 5};
    
    int sum = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        sum += *it;
    }
    ASSERT(sum == 15, "Sum should be 15");
    
    // Modify through iterator
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        *it *= 2;
    }
    
    ASSERT(arr[0] == 2, "arr[0] should be 2");
    ASSERT(arr[4] == 10, "arr[4] should be 10");
    
    return true;
}

// Test 7: Range-based for
bool test_range_based_for() {
    Array<int, 4> arr = {5, 10, 15, 20};
    
    int sum = 0;
    for (const auto& val : arr) {
        sum += val;
    }
    ASSERT(sum == 50, "Sum should be 50");
    
    // Modify through range-based for
    for (auto& val : arr) {
        val += 1;
    }
    
    ASSERT(arr[0] == 6, "arr[0] should be 6");
    ASSERT(arr[3] == 21, "arr[3] should be 21");
    
    return true;
}

// Test 8: STL Algorithm Compatibility
bool test_stl_algorithms() {
    Array<int, 6> arr = {3, 1, 4, 1, 5, 9};
    
    // Sort
    std::sort(arr.begin(), arr.end());
    ASSERT(arr[0] == 1, "First element should be 1 after sort");
    ASSERT(arr[5] == 9, "Last element should be 9 after sort");
    
    // Find
    auto it = std::find(arr.begin(), arr.end(), 5);
    ASSERT(it != arr.end(), "Should find 5");
    ASSERT(*it == 5, "Found element should be 5");
    
    // Count
    int ones = std::count(arr.begin(), arr.end(), 1);
    ASSERT(ones == 2, "Should find two 1s");
    
    return true;
}

// Test 9: Aggregate Initialization
bool test_aggregate_initialization() {
    // Full initialization
    Array<int, 3> arr1 = {1, 2, 3};
    ASSERT(arr1[0] == 1 && arr1[1] == 2 && arr1[2] == 3, "Full init should work");
    
    // Partial initialization (remaining zero-initialized)
    Array<int, 5> arr2 = {10, 20};
    ASSERT(arr2[0] == 10, "arr2[0] should be 10");
    ASSERT(arr2[1] == 20, "arr2[1] should be 20");
    ASSERT(arr2[2] == 0, "arr2[2] should be 0");
    ASSERT(arr2[4] == 0, "arr2[4] should be 0");
    
    return true;
}

// Test 10: Multidimensional Array
bool test_multidimensional() {
    Array<Array<int, 3>, 2> matrix = {{
        {1, 2, 3},
        {4, 5, 6}
    }};
    
    ASSERT(matrix[0][0] == 1, "matrix[0][0] should be 1");
    ASSERT(matrix[0][2] == 3, "matrix[0][2] should be 3");
    ASSERT(matrix[1][0] == 4, "matrix[1][0] should be 4");
    ASSERT(matrix[1][2] == 6, "matrix[1][2] should be 6");
    
    matrix[1][1] = 999;
    ASSERT(matrix[1][1] == 999, "matrix[1][1] should be 999");
    
    return true;
}

// Test 11: Const Correctness
bool test_const_correctness() {
    const Array<int, 3> arr = {10, 20, 30};
    
    ASSERT(arr[0] == 10, "const arr[0] should be 10");
    ASSERT(arr.front() == 10, "const front() should work");
    ASSERT(arr.back() == 30, "const back() should work");
    ASSERT(arr.size() == 3, "const size() should work");
    
    int sum = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        sum += *it;
    }
    ASSERT(sum == 60, "const iterators should work");
    
    return true;
}

// Test 12: Different Types
bool test_different_types() {
    // Double
    Array<double, 3> darr = {1.1, 2.2, 3.3};
    ASSERT(darr[0] > 1.0 && darr[0] < 1.2, "Double array should work");
    
    // String (POD-like)
    Array<const char*, 3> sarr = {"hello", "world", "test"};
    ASSERT(strcmp(sarr[0], "hello") == 0, "String array should work");
    
    return true;
}

int main() {
    int passed = 0;
    int failed = 0;
    
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Array Container Test Suite            ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    RUN_TEST(test_construction_and_size);
    RUN_TEST(test_element_access);
    RUN_TEST(test_at_bounds_checking);
    RUN_TEST(test_modification);
    RUN_TEST(test_fill);
    RUN_TEST(test_iterators);
    RUN_TEST(test_range_based_for);
    RUN_TEST(test_stl_algorithms);
    RUN_TEST(test_aggregate_initialization);
    RUN_TEST(test_multidimensional);
    RUN_TEST(test_const_correctness);
    RUN_TEST(test_different_types);
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return failed == 0 ? 0 : 1;
}


/* ===== EXPECTED OUTPUT ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║          Array Container Test Suite            ║
╚════════════════════════════════════════════════╝

Running test_construction_and_size... ✅ PASSED
Running test_element_access... ✅ PASSED
Running test_at_bounds_checking... ✅ PASSED
Running test_modification... ✅ PASSED
Running test_fill... ✅ PASSED
Running test_iterators... ✅ PASSED
Running test_range_based_for... ✅ PASSED
Running test_stl_algorithms... ✅ PASSED
Running test_aggregate_initialization... ✅ PASSED
Running test_multidimensional... ✅ PASSED
Running test_const_correctness... ✅ PASSED
Running test_different_types... ✅ PASSED

════════════════════════════════════════════════
Test Results: 12 passed, 0 failed
════════════════════════════════════════════════
 * ============================================================================ */
