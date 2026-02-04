#include "../optional/optional.hpp"
#include <iostream>
#include <string>

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
    Optional<int> opt;
    ASSERT(!opt.has_value(), "Default optional should be empty");
    ASSERT(!opt, "Default optional should convert to false");
    return true;
}

// Test 2: Value Construction
bool test_value_construction() {
    Optional<int> opt(42);
    ASSERT(opt.has_value(), "Optional should have value");
    ASSERT(opt, "Optional should convert to true");
    ASSERT(opt.value() == 42, "Value should be 42");
    ASSERT(*opt == 42, "Dereferenced value should be 42");
    return true;
}

// Test 3: value_or
bool test_value_or() {
    Optional<int> opt1(100);
    Optional<int> opt2;
    
    ASSERT(opt1.value_or(999) == 100, "Should return actual value");
    ASSERT(opt2.value_or(999) == 999, "Should return default value");
    return true;
}

// Test 4: Exception on Empty Access
bool test_exception_on_empty() {
    Optional<int> opt;
    
    bool caught = false;
    try {
        int val = opt.value();
        (void)val;
    } catch (const std::runtime_error&) {
        caught = true;
    }
    
    ASSERT(caught, "Should throw when accessing empty optional");
    return true;
}

// Test 5: Reset
bool test_reset() {
    Optional<int> opt(123);
    ASSERT(opt.has_value(), "Should have value initially");
    
    opt.reset();
    ASSERT(!opt.has_value(), "Should be empty after reset");
    
    // Reset empty optional (should be safe)
    opt.reset();
    ASSERT(!opt.has_value(), "Should still be empty");
    
    return true;
}

// Test 6: Copy Construction
bool test_copy_construction() {
    Optional<int> opt1(55);
    Optional<int> opt2(opt1);
    
    ASSERT(opt2.has_value(), "Copy should have value");
    ASSERT(*opt2 == 55, "Copy should have same value");
    ASSERT(*opt1 == 55, "Original should be unchanged");
    
    // Copy empty optional
    Optional<int> opt3;
    Optional<int> opt4(opt3);
    ASSERT(!opt4.has_value(), "Copy of empty should be empty");
    
    return true;
}

// Test 7: Move Construction
bool test_move_construction() {
    Optional<int> opt1(77);
    Optional<int> opt2(std::move(opt1));
    
    ASSERT(opt2.has_value(), "Moved-to should have value");
    ASSERT(*opt2 == 77, "Moved-to should have correct value");
    ASSERT(!opt1.has_value(), "Moved-from should be empty");
    
    return true;
}

// Test 8: Copy Assignment
bool test_copy_assignment() {
    Optional<int> opt1(88);
    Optional<int> opt2;
    
    opt2 = opt1;
    ASSERT(opt2.has_value(), "Assigned optional should have value");
    ASSERT(*opt2 == 88, "Assigned value should be correct");
    
    // Self-assignment
    opt1 = opt1;
    ASSERT(*opt1 == 88, "Self-assignment should preserve value");
    
    return true;
}

// Test 9: Arrow Operator
struct Point {
    int x, y;
    Point(int x_, int y_) : x(x_), y(y_) {}
};

bool test_arrow_operator() {
    Optional<Point> opt(Point(10, 20));
    
    ASSERT(opt->x == 10, "Arrow operator x should be 10");
    ASSERT(opt->y == 20, "Arrow operator y should be 20");
    
    // Modify through arrow
    opt->x = 30;
    ASSERT(opt->x == 30, "Modified x should be 30");
    
    return true;
}

// Test 10: Reassignment
bool test_reassignment() {
    Optional<int> opt(100);
    ASSERT(*opt == 100, "Initial value should be 100");
    
    opt = Optional<int>(200);
    ASSERT(*opt == 200, "Reassigned value should be 200");
    
    opt = Optional<int>();
    ASSERT(!opt.has_value(), "Assigned empty should be empty");
    
    return true;
}

// Test 11: String Type
bool test_string_type() {
    Optional<std::string> opt1("hello");
    ASSERT(opt1.has_value(), "String optional should have value");
    ASSERT(*opt1 == "hello", "String value should be correct");
    
    Optional<std::string> opt2;
    ASSERT(opt2.value_or("default") == "default", "Should use default");
    
    opt2 = Optional<std::string>("world");
    ASSERT(*opt2 == "world", "Assigned string should be correct");
    
    return true;
}

// Test 12: Resource Management
static int construct_count = 0;
static int destruct_count = 0;

struct Resource {
    Resource() { construct_count++; }
    ~Resource() { destruct_count++; }
    Resource(const Resource&) { construct_count++; }
};

bool test_resource_management() {
    construct_count = 0;
    destruct_count = 0;
    
    {
        Optional<Resource> opt1;
        ASSERT(construct_count == 0, "No construction for empty optional");
        
        opt1 = Optional<Resource>(Resource());
        ASSERT(construct_count >= 1, "Should construct resource");
        
        Optional<Resource> opt2(opt1);
        ASSERT(construct_count >= 2, "Should construct copy");
    }
    
    ASSERT(destruct_count == construct_count, "All resources should be destroyed");
    
    return true;
}

// Test 13: Bool Conversion
bool test_bool_conversion() {
    Optional<int> opt1(0);  // Value is 0, but optional is not empty!
    Optional<int> opt2;
    
    ASSERT(opt1, "Optional with value 0 should be true");
    ASSERT(!opt2, "Empty optional should be false");
    
    if (opt1) {
        ASSERT(*opt1 == 0, "Value should be 0");
    } else {
        ASSERT(false, "Should not reach here");
    }
    
    return true;
}

int main() {
    int passed = 0;
    int failed = 0;
    
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║        Optional Container Test Suite           ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    RUN_TEST(test_default_construction);
    RUN_TEST(test_value_construction);
    RUN_TEST(test_value_or);
    RUN_TEST(test_exception_on_empty);
    RUN_TEST(test_reset);
    RUN_TEST(test_copy_construction);
    RUN_TEST(test_move_construction);
    RUN_TEST(test_copy_assignment);
    RUN_TEST(test_arrow_operator);
    RUN_TEST(test_reassignment);
    RUN_TEST(test_string_type);
    RUN_TEST(test_resource_management);
    RUN_TEST(test_bool_conversion);
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return failed == 0 ? 0 : 1;
}

