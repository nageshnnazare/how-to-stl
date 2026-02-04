/**
 * @file test_suite.cpp
 * @brief Comprehensive test suite for unique_ptr implementation
 * 
 * This file contains unit tests to verify correctness of the unique_ptr implementation.
 * Compile with: g++ -std=c++14 -Wall -Wextra -g test_suite.cpp -o test_suite
 * Run with: ./test_suite
 */

#include "unique_ptr/unique_ptr.hpp"
#include <iostream>
#include <cassert>
#include <string>

// Test counter to track object creation/destruction
static int g_object_count = 0;

// Test class that tracks construction/destruction
class TestObject {
private:
    int value_;
    std::string name_;

public:
    TestObject(int value = 0, const std::string& name = "default") 
        : value_(value), name_(name) {
        g_object_count++;
    }

    ~TestObject() {
        g_object_count--;
    }

    int getValue() const { return value_; }
    std::string getName() const { return name_; }
    void setValue(int v) { value_ = v; }
};

// Base class for polymorphism tests
class Base {
public:
    virtual ~Base() = default;
    virtual int getType() const { return 0; }
};

class Derived : public Base {
public:
    int getType() const override { return 1; }
};

// Custom deleter for testing
static int g_custom_deleter_calls = 0;

template<typename T>
struct CountingDeleter {
    void operator()(T* ptr) const {
        g_custom_deleter_calls++;
        delete ptr;
    }
};

// Test helper macros
#define TEST(name) \
    void test_##name(); \
    struct TestRegistrar_##name { \
        TestRegistrar_##name() { \
            std::cout << "Running test: " #name "..."; \
            test_##name(); \
            std::cout << " PASSED\n"; \
        } \
    } registrar_##name; \
    void test_##name()

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { \
        std::cerr << "\n  Assertion failed at line " << __LINE__ << ": " #cond "\n"; \
        std::abort(); \
    } } while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))
#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))

// ============================================================================
// CONSTRUCTION TESTS
// ============================================================================

TEST(default_construction) {
    UniquePtr<TestObject> ptr;
    ASSERT_TRUE(ptr.get() == nullptr);
    ASSERT_FALSE(ptr);
}

TEST(nullptr_construction) {
    UniquePtr<TestObject> ptr(nullptr);
    ASSERT_TRUE(ptr.get() == nullptr);
    ASSERT_FALSE(ptr);
}

TEST(pointer_construction) {
    int initial_count = g_object_count;
    {
        UniquePtr<TestObject> ptr(new TestObject(42, "test"));
        ASSERT_TRUE(ptr.get() != nullptr);
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->getValue(), 42);
        ASSERT_EQ(g_object_count, initial_count + 1);
    }
    ASSERT_EQ(g_object_count, initial_count);
}

TEST(make_unique) {
    int initial_count = g_object_count;
    {
        auto ptr = makeUnique<TestObject>(100, "made");
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->getValue(), 100);
        ASSERT_EQ(ptr->getName(), "made");
        ASSERT_EQ(g_object_count, initial_count + 1);
    }
    ASSERT_EQ(g_object_count, initial_count);
}

TEST(move_construction) {
    auto ptr1 = makeUnique<TestObject>(50, "move_test");
    TestObject* raw = ptr1.get();
    
    UniquePtr<TestObject> ptr2(std::move(ptr1));
    
    ASSERT_FALSE(ptr1);
    ASSERT_TRUE(ptr2);
    ASSERT_EQ(ptr2.get(), raw);
    ASSERT_EQ(ptr2->getValue(), 50);
}

// ============================================================================
// ASSIGNMENT TESTS
// ============================================================================

TEST(move_assignment) {
    auto ptr1 = makeUnique<TestObject>(10, "first");
    auto ptr2 = makeUnique<TestObject>(20, "second");
    
    TestObject* raw1 = ptr1.get();
    int count_before = g_object_count;
    
    ptr2 = std::move(ptr1);
    
    ASSERT_FALSE(ptr1);
    ASSERT_TRUE(ptr2);
    ASSERT_EQ(ptr2.get(), raw1);
    ASSERT_EQ(ptr2->getValue(), 10);
    ASSERT_EQ(g_object_count, count_before - 1); // ptr2's original object deleted
}

TEST(nullptr_assignment) {
    auto ptr = makeUnique<TestObject>(30);
    int count_before = g_object_count;
    
    ptr = nullptr;
    
    ASSERT_FALSE(ptr);
    ASSERT_EQ(g_object_count, count_before - 1);
}

TEST(self_move_assignment) {
    auto ptr = makeUnique<TestObject>(40);
    TestObject* raw = ptr.get();
    
    ptr = std::move(ptr);  // Self-assignment
    
    // Object should still be valid
    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr.get(), raw);
}

// ============================================================================
// MODIFIER TESTS
// ============================================================================

TEST(reset_empty) {
    auto ptr = makeUnique<TestObject>(50);
    int count_before = g_object_count;
    
    ptr.reset();
    
    ASSERT_FALSE(ptr);
    ASSERT_EQ(g_object_count, count_before - 1);
}

TEST(reset_with_pointer) {
    auto ptr = makeUnique<TestObject>(60);
    int count_before = g_object_count;
    TestObject* new_obj = new TestObject(70);
    int count_after_new = g_object_count;
    
    ptr.reset(new_obj);
    
    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr.get(), new_obj);
    ASSERT_EQ(ptr->getValue(), 70);
    ASSERT_EQ(g_object_count, count_before); // Old deleted, new was already added
    ASSERT_EQ(g_object_count, count_after_new - 1); // Verify old was deleted
}

TEST(release) {
    auto ptr = makeUnique<TestObject>(80);
    int count_before = g_object_count;
    TestObject* raw = ptr.get();
    
    TestObject* released = ptr.release();
    
    ASSERT_FALSE(ptr);
    ASSERT_EQ(released, raw);
    ASSERT_EQ(g_object_count, count_before); // Object not deleted
    
    delete released; // Manual cleanup
    ASSERT_EQ(g_object_count, count_before - 1);
}

TEST(swap) {
    auto ptr1 = makeUnique<TestObject>(90);
    auto ptr2 = makeUnique<TestObject>(95);
    
    TestObject* raw1 = ptr1.get();
    TestObject* raw2 = ptr2.get();
    
    ptr1.swap(ptr2);
    
    ASSERT_EQ(ptr1.get(), raw2);
    ASSERT_EQ(ptr2.get(), raw1);
    ASSERT_EQ(ptr1->getValue(), 95);
    ASSERT_EQ(ptr2->getValue(), 90);
}

// ============================================================================
// OBSERVER TESTS
// ============================================================================

TEST(get) {
    TestObject* raw = new TestObject(100);
    UniquePtr<TestObject> ptr(raw);
    
    ASSERT_EQ(ptr.get(), raw);
}

TEST(dereference_operator) {
    auto ptr = makeUnique<TestObject>(110);
    
    TestObject& ref = *ptr;
    ASSERT_EQ(ref.getValue(), 110);
    
    ref.setValue(120);
    ASSERT_EQ(ptr->getValue(), 120);
}

TEST(arrow_operator) {
    auto ptr = makeUnique<TestObject>(130);
    
    ASSERT_EQ(ptr->getValue(), 130);
    ptr->setValue(140);
    ASSERT_EQ(ptr->getValue(), 140);
}

TEST(bool_conversion) {
    UniquePtr<TestObject> empty;
    auto full = makeUnique<TestObject>();
    
    ASSERT_FALSE(static_cast<bool>(empty));
    ASSERT_TRUE(static_cast<bool>(full));
    
    if (full) {
        // Should execute
    } else {
        ASSERT_TRUE(false); // Should not reach
    }
}

// ============================================================================
// ARRAY TESTS
// ============================================================================

TEST(array_construction) {
    int initial_count = g_object_count;
    {
        UniquePtr<TestObject[]> arr(new TestObject[3]);
        ASSERT_TRUE(arr);
        ASSERT_EQ(g_object_count, initial_count + 3);
    }
    ASSERT_EQ(g_object_count, initial_count);
}

TEST(make_unique_array) {
    auto arr = makeUniqueArray<int>(5);
    ASSERT_TRUE(arr);
    
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }
    
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(arr[i], i * 10);
    }
}

TEST(array_subscript) {
    UniquePtr<int[]> arr(new int[3]{10, 20, 30});
    
    ASSERT_EQ(arr[0], 10);
    ASSERT_EQ(arr[1], 20);
    ASSERT_EQ(arr[2], 30);
    
    arr[1] = 25;
    ASSERT_EQ(arr[1], 25);
}

// ============================================================================
// CUSTOM DELETER TESTS
// ============================================================================

TEST(custom_deleter) {
    g_custom_deleter_calls = 0;
    {
        UniquePtr<TestObject, CountingDeleter<TestObject>> ptr(
            new TestObject(),
            CountingDeleter<TestObject>()
        );
        ASSERT_TRUE(ptr);
    }
    ASSERT_EQ(g_custom_deleter_calls, 1);
}

// ============================================================================
// POLYMORPHISM TESTS
// ============================================================================

TEST(polymorphic_deletion) {
    UniquePtr<Base> ptr = makeUnique<Derived>();
    ASSERT_EQ(ptr->getType(), 1); // Derived type
}

TEST(derived_to_base_conversion) {
    UniquePtr<Derived> derived = makeUnique<Derived>();
    ASSERT_EQ(derived->getType(), 1);
    
    UniquePtr<Base> base = std::move(derived);
    ASSERT_FALSE(derived);
    ASSERT_TRUE(base);
    ASSERT_EQ(base->getType(), 1);
}

// ============================================================================
// COMPARISON TESTS
// ============================================================================

TEST(nullptr_comparison) {
    UniquePtr<TestObject> empty;
    auto full = makeUnique<TestObject>();
    
    ASSERT_TRUE(empty == nullptr);
    ASSERT_TRUE(nullptr == empty);
    ASSERT_FALSE(full == nullptr);
    ASSERT_FALSE(nullptr == full);
    
    ASSERT_FALSE(empty != nullptr);
    ASSERT_TRUE(full != nullptr);
}

TEST(pointer_comparison) {
    auto ptr1 = makeUnique<TestObject>();
    auto ptr2 = makeUnique<TestObject>();
    UniquePtr<TestObject> empty;
    
    ASSERT_TRUE(ptr1 != ptr2);
    ASSERT_FALSE(ptr1 == ptr2);
    ASSERT_TRUE(empty == empty);
}

// ============================================================================
// FUNCTION PARAMETER TESTS
// ============================================================================

void take_ownership(UniquePtr<TestObject> ptr) {
    ASSERT_TRUE(ptr);
    // ptr deleted when function returns
}

void borrow_pointer(const UniquePtr<TestObject>& ptr) {
    ASSERT_TRUE(ptr);
}

void modify_pointer(UniquePtr<TestObject>& ptr) {
    ptr.reset(new TestObject(999));
}

TEST(function_by_value) {
    auto ptr = makeUnique<TestObject>(200);
    int count_before = g_object_count;
    
    take_ownership(std::move(ptr));
    
    ASSERT_FALSE(ptr);
    ASSERT_EQ(g_object_count, count_before - 1);
}

TEST(function_by_reference) {
    auto ptr = makeUnique<TestObject>(210);
    
    borrow_pointer(ptr);
    
    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr->getValue(), 210);
}

TEST(function_modify) {
    auto ptr = makeUnique<TestObject>(220);
    
    modify_pointer(ptr);
    
    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr->getValue(), 999);
}

// ============================================================================
// RETURN VALUE TESTS
// ============================================================================

UniquePtr<TestObject> return_by_value(int value) {
    return makeUnique<TestObject>(value);
}

TEST(function_return) {
    auto ptr = return_by_value(300);
    
    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr->getValue(), 300);
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST(multiple_reset) {
    auto ptr = makeUnique<TestObject>(400);
    
    ptr.reset(new TestObject(401));
    ptr.reset(new TestObject(402));
    ptr.reset(new TestObject(403));
    
    ASSERT_EQ(ptr->getValue(), 403);
}

TEST(reset_with_get) {
    auto ptr = makeUnique<TestObject>(500);
    // Note: Resetting with the same pointer causes undefined behavior
    // This test is commented out as it's an anti-pattern
    // In real code, never do: ptr.reset(ptr.get());
    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr->getValue(), 500);
}

TEST(move_from_empty) {
    UniquePtr<TestObject> empty1;
    UniquePtr<TestObject> empty2(std::move(empty1));
    
    ASSERT_FALSE(empty1);
    ASSERT_FALSE(empty2);
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  unique_ptr Test Suite                \n";
    std::cout << "========================================\n\n";
    
    // Tests run automatically via static initialization
    
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  All tests passed!                    \n";
    std::cout << "========================================\n";
    std::cout << "\nFinal object count: " << g_object_count << " (should be 0)\n";
    
    if (g_object_count != 0) {
        std::cerr << "ERROR: Memory leak detected!\n";
        return 1;
    }
    
    return 0;
}

