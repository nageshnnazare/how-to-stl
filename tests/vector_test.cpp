/**
 * @file vector_test.cpp
 * @brief Comprehensive test suite for Vector implementation
 */

#include "vector/vector.hpp"
#include <iostream>
#include <string>
#include <cassert>

// Test counter
static int g_tests_passed = 0;
static int g_tests_total = 0;

// Object counter for memory leak detection
static int g_object_count = 0;

// Test object class
class TestObject {
public:
    int value;
    
    TestObject() : value(0) {
        ++g_object_count;
    }
    
    explicit TestObject(int v) : value(v) {
        ++g_object_count;
    }
    
    TestObject(const TestObject& other) : value(other.value) {
        ++g_object_count;
    }
    
    TestObject(TestObject&& other) noexcept : value(other.value) {
        other.value = -1;
        ++g_object_count;
    }
    
    TestObject& operator=(const TestObject& other) {
        value = other.value;
        return *this;
    }
    
    TestObject& operator=(TestObject&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }
    
    ~TestObject() {
        --g_object_count;
    }
    
    bool operator==(const TestObject& other) const {
        return value == other.value;
    }
    
    bool operator<(const TestObject& other) const {
        return value < other.value;
    }
};

// Macro for running tests
#define RUN_TEST(test_func) do { \
    std::cout << "Running " << #test_func << "... "; \
    ++g_tests_total; \
    test_func(); \
    ++g_tests_passed; \
    std::cout << "PASSED\n"; \
} while(0)

// ============================================================================
// CONSTRUCTION TESTS
// ============================================================================

void test_default_constructor() {
    Vector<int> vec;
    assert(vec.size() == 0);
    assert(vec.capacity() == 0);
    assert(vec.empty());
}

void test_size_constructor() {
    Vector<int> vec(5);
    assert(vec.size() == 5);
    assert(vec.capacity() == 5);
    assert(!vec.empty());
    
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == 0);
    }
}

void test_size_value_constructor() {
    Vector<int> vec(5, 42);
    assert(vec.size() == 5);
    assert(vec.capacity() == 5);
    
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == 42);
    }
}

void test_initializer_list_constructor() {
    Vector<int> vec = {1, 2, 3, 4, 5};
    assert(vec.size() == 5);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 3);
    assert(vec[3] == 4);
    assert(vec[4] == 5);
}

void test_copy_constructor() {
    Vector<int> vec1 = {1, 2, 3};
    Vector<int> vec2(vec1);
    
    assert(vec2.size() == 3);
    assert(vec2[0] == 1);
    assert(vec2[1] == 2);
    assert(vec2[2] == 3);
    
    // Modify vec2 and ensure vec1 is unchanged
    vec2[0] = 100;
    assert(vec1[0] == 1);
}

void test_move_constructor() {
    Vector<int> vec1 = {1, 2, 3, 4, 5};
    size_t old_cap = vec1.capacity();
    
    Vector<int> vec2(std::move(vec1));
    
    assert(vec2.size() == 5);
    assert(vec2.capacity() == old_cap);
    assert(vec2[0] == 1);
    
    assert(vec1.size() == 0);
    assert(vec1.capacity() == 0);
}

// ============================================================================
// ASSIGNMENT TESTS
// ============================================================================

void test_copy_assignment() {
    Vector<int> vec1 = {1, 2, 3};
    Vector<int> vec2;
    
    vec2 = vec1;
    
    assert(vec2.size() == 3);
    assert(vec2[0] == 1);
    assert(vec2[1] == 2);
    assert(vec2[2] == 3);
    
    vec2[0] = 100;
    assert(vec1[0] == 1);
}

void test_move_assignment() {
    Vector<int> vec1 = {1, 2, 3, 4, 5};
    Vector<int> vec2;
    
    vec2 = std::move(vec1);
    
    assert(vec2.size() == 5);
    assert(vec2[0] == 1);
    
    assert(vec1.size() == 0);
}

void test_self_assignment() {
    Vector<int> vec = {1, 2, 3};
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wself-assign-overloaded"
    vec = vec;  // Self-assignment
    #pragma GCC diagnostic pop
    
    assert(vec.size() == 3);
    assert(vec[0] == 1);
}

void test_initializer_list_assignment() {
    Vector<int> vec;
    vec = {10, 20, 30};
    
    assert(vec.size() == 3);
    assert(vec[0] == 10);
    assert(vec[1] == 20);
    assert(vec[2] == 30);
}

// ============================================================================
// ELEMENT ACCESS TESTS
// ============================================================================

void test_subscript_operator() {
    Vector<int> vec = {1, 2, 3, 4, 5};
    
    assert(vec[0] == 1);
    assert(vec[2] == 3);
    assert(vec[4] == 5);
    
    vec[2] = 100;
    assert(vec[2] == 100);
}

void test_at_method() {
    Vector<int> vec = {1, 2, 3};
    
    assert(vec.at(0) == 1);
    assert(vec.at(2) == 3);
    
    vec.at(1) = 200;
    assert(vec.at(1) == 200);
    
    // Test out of range
    bool caught = false;
    try {
        int x = vec.at(100);
        (void)x;
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);
}

void test_front_back() {
    Vector<int> vec = {1, 2, 3, 4, 5};
    
    assert(vec.front() == 1);
    assert(vec.back() == 5);
    
    vec.front() = 100;
    vec.back() = 500;
    
    assert(vec.front() == 100);
    assert(vec.back() == 500);
}

void test_data_method() {
    Vector<int> vec = {1, 2, 3};
    
    int* ptr = vec.data();
    assert(ptr != nullptr);
    assert(ptr[0] == 1);
    assert(ptr[1] == 2);
    assert(ptr[2] == 3);
    
    ptr[1] = 200;
    assert(vec[1] == 200);
}

// ============================================================================
// ITERATOR TESTS
// ============================================================================

void test_iterators() {
    Vector<int> vec = {1, 2, 3, 4, 5};
    
    // Forward iteration
    int expected = 1;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        assert(*it == expected);
        ++expected;
    }
    
    // Const iterators
    const Vector<int>& cvec = vec;
    expected = 1;
    for (auto it = cvec.cbegin(); it != cvec.cend(); ++it) {
        assert(*it == expected);
        ++expected;
    }
}

void test_range_based_for() {
    Vector<int> vec = {1, 2, 3, 4, 5};
    
    int sum = 0;
    for (int x : vec) {
        sum += x;
    }
    assert(sum == 15);
    
    // Modify via reference
    for (int& x : vec) {
        x *= 2;
    }
    
    assert(vec[0] == 2);
    assert(vec[4] == 10);
}

// ============================================================================
// CAPACITY TESTS
// ============================================================================

void test_empty_size() {
    Vector<int> vec;
    assert(vec.empty());
    assert(vec.size() == 0);
    
    vec.push_back(1);
    assert(!vec.empty());
    assert(vec.size() == 1);
}

void test_reserve() {
    Vector<int> vec;
    assert(vec.capacity() == 0);
    
    vec.reserve(10);
    assert(vec.capacity() == 10);
    assert(vec.size() == 0);
    
    // Reserve smaller - should not change
    vec.reserve(5);
    assert(vec.capacity() == 10);
    
    // Add elements - should not reallocate
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    assert(vec.capacity() == 10);
}

void test_shrink_to_fit() {
    Vector<int> vec;
    vec.reserve(100);
    assert(vec.capacity() == 100);
    
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    vec.shrink_to_fit();
    assert(vec.capacity() == 5);
    assert(vec.size() == 5);
}

// ============================================================================
// MODIFIER TESTS
// ============================================================================

void test_clear() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        for (int i = 0; i < 5; ++i) {
            vec.push_back(TestObject(i));
        }
        
        assert(vec.size() == 5);
        size_t cap = vec.capacity();
        
        vec.clear();
        
        assert(vec.size() == 0);
        assert(vec.capacity() == cap);  // Capacity unchanged
        assert(vec.empty());
    }
    
    assert(g_object_count == count_before);
}

void test_push_back() {
    Vector<int> vec;
    
    vec.push_back(1);
    assert(vec.size() == 1);
    assert(vec[0] == 1);
    
    vec.push_back(2);
    vec.push_back(3);
    assert(vec.size() == 3);
    assert(vec[2] == 3);
}

void test_push_back_move() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        
        TestObject obj(42);
        vec.push_back(std::move(obj));
        
        assert(vec.size() == 1);
        assert(vec[0].value == 42);
        assert(obj.value == -1);  // Moved from
    }
    
    assert(g_object_count == count_before);
}

void test_emplace_back() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        
        // Construct in place
        vec.emplace_back(123);
        
        assert(vec.size() == 1);
        assert(vec[0].value == 123);
    }
    
    assert(g_object_count == count_before);
}

void test_pop_back() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        vec.push_back(TestObject(1));
        vec.push_back(TestObject(2));
        vec.push_back(TestObject(3));
        
        assert(vec.size() == 3);
        
        vec.pop_back();
        assert(vec.size() == 2);
        assert(vec[1].value == 2);
        
        vec.pop_back();
        vec.pop_back();
        assert(vec.size() == 0);
        assert(vec.empty());
    }
    
    assert(g_object_count == count_before);
}

void test_insert_single() {
    Vector<int> vec = {1, 2, 4, 5};
    
    // Insert in middle
    vec.insert(vec.begin() + 2, 3);
    assert(vec.size() == 5);
    assert(vec[2] == 3);
    
    // Insert at beginning
    vec.insert(vec.begin(), 0);
    assert(vec.size() == 6);
    assert(vec[0] == 0);
    
    // Insert at end
    vec.insert(vec.end(), 6);
    assert(vec.size() == 7);
    assert(vec[6] == 6);
}

void test_insert_move() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        vec.push_back(TestObject(1));
        vec.push_back(TestObject(3));
        
        TestObject obj(2);
        vec.insert(vec.begin() + 1, std::move(obj));
        
        assert(vec.size() == 3);
        assert(vec[1].value == 2);
        assert(obj.value == -1);
    }
    
    assert(g_object_count == count_before);
}

void test_erase_single() {
    Vector<int> vec = {0, 1, 2, 3, 4, 5};
    
    // Erase from middle
    vec.erase(vec.begin() + 2);
    assert(vec.size() == 5);
    assert(vec[2] == 3);
    
    // Erase from beginning
    vec.erase(vec.begin());
    assert(vec.size() == 4);
    assert(vec[0] == 1);
}

void test_erase_range() {
    Vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    // Erase range
    vec.erase(vec.begin() + 2, vec.begin() + 5);
    assert(vec.size() == 7);
    assert(vec[2] == 5);
    assert(vec[3] == 6);
    
    // Erase empty range (no-op)
    size_t old_size = vec.size();
    vec.erase(vec.begin() + 2, vec.begin() + 2);
    assert(vec.size() == old_size);
}

void test_resize_grow() {
    Vector<int> vec = {1, 2, 3};
    
    vec.resize(6);
    assert(vec.size() == 6);
    assert(vec[0] == 1);
    assert(vec[2] == 3);
    assert(vec[3] == 0);  // Default constructed
    assert(vec[5] == 0);
}

void test_resize_grow_with_value() {
    Vector<int> vec = {1, 2, 3};
    
    vec.resize(6, 99);
    assert(vec.size() == 6);
    assert(vec[0] == 1);
    assert(vec[3] == 99);
    assert(vec[5] == 99);
}

void test_resize_shrink() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        for (int i = 0; i < 10; ++i) {
            vec.push_back(TestObject(i));
        }
        
        vec.resize(5);
        assert(vec.size() == 5);
        assert(vec[4].value == 4);
    }
    
    assert(g_object_count == count_before);
}

void test_swap() {
    Vector<int> vec1 = {1, 2, 3};
    Vector<int> vec2 = {10, 20, 30, 40, 50};
    
    size_t cap1 = vec1.capacity();
    size_t cap2 = vec2.capacity();
    
    vec1.swap(vec2);
    
    assert(vec1.size() == 5);
    assert(vec1[0] == 10);
    assert(vec1.capacity() == cap2);
    
    assert(vec2.size() == 3);
    assert(vec2[0] == 1);
    assert(vec2.capacity() == cap1);
}

// ============================================================================
// COMPARISON TESTS
// ============================================================================

void test_equality() {
    Vector<int> vec1 = {1, 2, 3};
    Vector<int> vec2 = {1, 2, 3};
    Vector<int> vec3 = {1, 2, 4};
    Vector<int> vec4 = {1, 2};
    
    assert(vec1 == vec2);
    assert(!(vec1 == vec3));
    assert(!(vec1 == vec4));
    
    assert(!(vec1 != vec2));
    assert(vec1 != vec3);
    assert(vec1 != vec4);
}

void test_relational() {
    Vector<int> vec1 = {1, 2, 3};
    Vector<int> vec2 = {1, 2, 4};
    Vector<int> vec3 = {1, 2};
    
    assert(vec1 < vec2);
    assert(!(vec2 < vec1));
    assert(vec3 < vec1);
    
    assert(vec2 > vec1);
    assert(!(vec1 > vec2));
    assert(vec1 > vec3);
    
    assert(vec1 <= vec2);
    assert(vec1 <= vec1);
    
    assert(vec2 >= vec1);
    assert(vec1 >= vec1);
}

// ============================================================================
// GROWTH PATTERN TESTS
// ============================================================================

void test_growth_pattern() {
    Vector<int> vec;
    
    size_t last_cap = 0;
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
        
        if (vec.capacity() != last_cap) {
            // Capacity doubled (or started at 1)
            if (last_cap > 0) {
                assert(vec.capacity() == last_cap * 2);
            }
            last_cap = vec.capacity();
        }
    }
}

// ============================================================================
// EDGE CASES
// ============================================================================

void test_empty_operations() {
    Vector<int> vec;
    
    vec.clear();  // Should be safe
    assert(vec.empty());
    
    vec.shrink_to_fit();  // Should be safe
    assert(vec.capacity() == 0);
}

void test_large_vector() {
    Vector<int> vec;
    
    for (int i = 0; i < 10000; ++i) {
        vec.push_back(i);
    }
    
    assert(vec.size() == 10000);
    assert(vec[9999] == 9999);
}

void test_with_strings() {
    Vector<std::string> vec;
    vec.push_back("Hello");
    vec.push_back("World");
    vec.push_back("!");
    
    assert(vec.size() == 3);
    assert(vec[0] == "Hello");
    assert(vec[1] == "World");
    assert(vec[2] == "!");
    
    vec.erase(vec.begin() + 1);
    assert(vec.size() == 2);
    assert(vec[1] == "!");
}

// ============================================================================
// MEMORY LEAK TESTS
// ============================================================================

void test_no_memory_leaks() {
    int count_before = g_object_count;
    
    {
        Vector<TestObject> vec;
        
        for (int i = 0; i < 100; ++i) {
            vec.push_back(TestObject(i));
        }
        
        vec.clear();
        
        for (int i = 0; i < 50; ++i) {
            vec.push_back(TestObject(i * 2));
        }
        
        vec.resize(25);
        vec.reserve(200);
        vec.shrink_to_fit();
    }
    
    assert(g_object_count == count_before);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    std::cout << "=================================================\n";
    std::cout << "     Vector Implementation Test Suite           \n";
    std::cout << "=================================================\n\n";
    
    try {
        // Construction tests
        RUN_TEST(test_default_constructor);
        RUN_TEST(test_size_constructor);
        RUN_TEST(test_size_value_constructor);
        RUN_TEST(test_initializer_list_constructor);
        RUN_TEST(test_copy_constructor);
        RUN_TEST(test_move_constructor);
        
        // Assignment tests
        RUN_TEST(test_copy_assignment);
        RUN_TEST(test_move_assignment);
        RUN_TEST(test_self_assignment);
        RUN_TEST(test_initializer_list_assignment);
        
        // Element access tests
        RUN_TEST(test_subscript_operator);
        RUN_TEST(test_at_method);
        RUN_TEST(test_front_back);
        RUN_TEST(test_data_method);
        
        // Iterator tests
        RUN_TEST(test_iterators);
        RUN_TEST(test_range_based_for);
        
        // Capacity tests
        RUN_TEST(test_empty_size);
        RUN_TEST(test_reserve);
        RUN_TEST(test_shrink_to_fit);
        
        // Modifier tests
        RUN_TEST(test_clear);
        RUN_TEST(test_push_back);
        RUN_TEST(test_push_back_move);
        RUN_TEST(test_emplace_back);
        RUN_TEST(test_pop_back);
        RUN_TEST(test_insert_single);
        RUN_TEST(test_insert_move);
        RUN_TEST(test_erase_single);
        RUN_TEST(test_erase_range);
        RUN_TEST(test_resize_grow);
        RUN_TEST(test_resize_grow_with_value);
        RUN_TEST(test_resize_shrink);
        RUN_TEST(test_swap);
        
        // Comparison tests
        RUN_TEST(test_equality);
        RUN_TEST(test_relational);
        
        // Growth pattern
        RUN_TEST(test_growth_pattern);
        
        // Edge cases
        RUN_TEST(test_empty_operations);
        RUN_TEST(test_large_vector);
        RUN_TEST(test_with_strings);
        
        // Memory leak test
        RUN_TEST(test_no_memory_leaks);
        
        std::cout << "\n=================================================\n";
        std::cout << "  RESULTS: " << g_tests_passed << "/" << g_tests_total << " tests passed\n";
        std::cout << "=================================================\n";
        
        if (g_tests_passed == g_tests_total) {
            std::cout << "✓ All tests PASSED!\n";
            return 0;
        } else {
            std::cout << "✗ Some tests FAILED!\n";
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\nException caught: " << e.what() << "\n";
        return 1;
    }
}

