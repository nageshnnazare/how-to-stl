/**
 * @file example.cpp
 * @brief Comprehensive examples demonstrating the unique_ptr implementation
 * 
 * This file shows various use cases and features of the custom unique_ptr
 */

#include "unique_ptr/unique_ptr.hpp"
#include <iostream>
#include <string>

// ============================================================================
// EXAMPLE CLASSES
// ============================================================================

class Resource {
private:
    std::string name_;
    int id_;

public:
    Resource(const std::string& name, int id) : name_(name), id_(id) {
        std::cout << "Resource '" << name_ << "' (ID: " << id_ << ") created\n";
    }

    ~Resource() {
        std::cout << "Resource '" << name_ << "' (ID: " << id_ << ") destroyed\n";
    }

    void use() const {
        std::cout << "Using resource: " << name_ << " (ID: " << id_ << ")\n";
    }

    std::string getName() const { return name_; }
    int getId() const { return id_; }
};

// Base and derived classes for demonstrating polymorphism
class Base {
public:
    virtual ~Base() {
        std::cout << "Base destructor\n";
    }
    
    virtual void speak() const {
        std::cout << "I am Base\n";
    }
};

class Derived : public Base {
public:
    ~Derived() override {
        std::cout << "Derived destructor\n";
    }
    
    void speak() const override {
        std::cout << "I am Derived\n";
    }
};

// ============================================================================
// CUSTOM DELETER EXAMPLES
// ============================================================================

// Custom deleter that logs deletion
template<typename T>
struct LoggingDeleter {
    void operator()(T* ptr) const {
        std::cout << "LoggingDeleter: Deleting object at " << ptr << "\n";
        delete ptr;
    }
};

// Custom deleter for C-style file handles (demonstration only)
struct FileDeleter {
    void operator()(FILE* file) const {
        if (file) {
            std::cout << "Closing file\n";
            fclose(file);
        }
    }
};

// ============================================================================
// EXAMPLE FUNCTIONS
// ============================================================================

/**
 * Example 1: Basic usage with automatic cleanup
 */
void example1_basic_usage() {
    std::cout << "\n=== Example 1: Basic Usage ===\n";
    
    {
        UniquePtr<Resource> ptr(new Resource("Database", 1));
        ptr->use();
        // Resource automatically deleted when ptr goes out of scope
    }
    
    std::cout << "After scope - resource has been cleaned up\n";
}

/**
 * Example 2: Using makeUnique (preferred way)
 */
void example2_make_unique() {
    std::cout << "\n=== Example 2: Using makeUnique ===\n";
    
    auto ptr = makeUnique<Resource>("Network", 2);
    ptr->use();
    
    std::cout << "Name: " << ptr->getName() << "\n";
    std::cout << "ID: " << ptr->getId() << "\n";
}

/**
 * Example 3: Move semantics (transferring ownership)
 */
void example3_move_semantics() {
    std::cout << "\n=== Example 3: Move Semantics ===\n";
    
    UniquePtr<Resource> ptr1(new Resource("Cache", 3));
    std::cout << "ptr1 owns resource: " << (ptr1 ? "yes" : "no") << "\n";
    
    // Transfer ownership via move
    UniquePtr<Resource> ptr2 = std::move(ptr1);
    std::cout << "After move:\n";
    std::cout << "  ptr1 owns resource: " << (ptr1 ? "yes" : "no") << "\n";
    std::cout << "  ptr2 owns resource: " << (ptr2 ? "yes" : "no") << "\n";
    
    ptr2->use();
}

/**
 * Example 4: Reset and release
 */
void example4_reset_release() {
    std::cout << "\n=== Example 4: Reset and Release ===\n";
    
    UniquePtr<Resource> ptr(new Resource("Session", 4));
    
    // Reset with a new resource
    std::cout << "Resetting to new resource...\n";
    ptr.reset(new Resource("NewSession", 5));
    
    // Release ownership (caller becomes responsible for deletion)
    std::cout << "Releasing ownership...\n";
    Resource* raw_ptr = ptr.release();
    std::cout << "ptr owns resource: " << (ptr ? "yes" : "no") << "\n";
    
    // Must manually delete since we released ownership
    std::cout << "Manually deleting...\n";
    delete raw_ptr;
}

/**
 * Example 5: Array support
 */
void example5_arrays() {
    std::cout << "\n=== Example 5: Array Support ===\n";
    
    // Create array of integers
    UniquePtr<int[]> arr = makeUniqueArray<int>(5);
    
    // Initialize array
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }
    
    // Access array elements
    std::cout << "Array elements: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
    
    // Array automatically deleted with delete[]
}

/**
 * Example 6: Custom deleters
 */
void example6_custom_deleter() {
    std::cout << "\n=== Example 6: Custom Deleter ===\n";
    
    UniquePtr<Resource, LoggingDeleter<Resource>> ptr(
        new Resource("LoggedResource", 6),
        LoggingDeleter<Resource>()
    );
    
    ptr->use();
    // LoggingDeleter will be called when ptr goes out of scope
}

/**
 * Example 7: Polymorphism
 */
void example7_polymorphism() {
    std::cout << "\n=== Example 7: Polymorphism ===\n";
    
    UniquePtr<Base> ptr1 = makeUnique<Derived>();
    ptr1->speak();  // Calls Derived::speak() due to virtual function
    
    // Can move Derived* to Base*
    UniquePtr<Derived> derived = makeUnique<Derived>();
    UniquePtr<Base> base = std::move(derived);
    base->speak();
}

/**
 * Example 8: Passing unique_ptr to functions
 */
void processResource(UniquePtr<Resource> ptr) {
    // Takes ownership by value (must be moved in)
    ptr->use();
    std::cout << "Processing complete\n";
    // Resource deleted when function returns
}

void borrowResource(const UniquePtr<Resource>& ptr) {
    // Borrows the resource (doesn't take ownership)
    if (ptr) {
        ptr->use();
    }
}

void example8_function_parameters() {
    std::cout << "\n=== Example 8: Function Parameters ===\n";
    
    auto ptr = makeUnique<Resource>("SharedData", 8);
    
    // Borrow the resource (ownership not transferred)
    std::cout << "Borrowing resource:\n";
    borrowResource(ptr);
    std::cout << "Still own resource: " << (ptr ? "yes" : "no") << "\n";
    
    // Transfer ownership to function
    std::cout << "\nTransferring ownership:\n";
    processResource(std::move(ptr));
    std::cout << "Still own resource: " << (ptr ? "yes" : "no") << "\n";
}

/**
 * Example 9: Returning unique_ptr from functions
 */
UniquePtr<Resource> createResource(const std::string& name, int id) {
    // Create and return - ownership transferred to caller
    return makeUnique<Resource>(name, id);
}

void example9_return_values() {
    std::cout << "\n=== Example 9: Return Values ===\n";
    
    auto ptr = createResource("ReturnedResource", 9);
    ptr->use();
    
    // Can also chain operations
    createResource("TemporaryResource", 10)->use();
    std::cout << "(Temporary resource destroyed immediately)\n";
}

/**
 * Example 10: Comparison operations
 */
void example10_comparisons() {
    std::cout << "\n=== Example 10: Comparisons ===\n";
    
    UniquePtr<Resource> ptr1(new Resource("Resource1", 11));
    UniquePtr<Resource> ptr2;  // Empty
    
    std::cout << "ptr1 == nullptr: " << (ptr1 == nullptr ? "true" : "false") << "\n";
    std::cout << "ptr2 == nullptr: " << (ptr2 == nullptr ? "true" : "false") << "\n";
    std::cout << "ptr1 != ptr2: " << (ptr1 != ptr2 ? "true" : "false") << "\n";
    
    // Can use in boolean context
    if (ptr1) {
        std::cout << "ptr1 is valid\n";
    }
    
    if (!ptr2) {
        std::cout << "ptr2 is null\n";
    }
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=================================================\n";
    std::cout << "   Custom unique_ptr Implementation Examples   \n";
    std::cout << "=================================================\n";
    
    try {
        example1_basic_usage();
        example2_make_unique();
        example3_move_semantics();
        example4_reset_release();
        example5_arrays();
        example6_custom_deleter();
        example7_polymorphism();
        example8_function_parameters();
        example9_return_values();
        example10_comparisons();
        
        std::cout << "\n=================================================\n";
        std::cout << "   All examples completed successfully!        \n";
        std::cout << "=================================================\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

