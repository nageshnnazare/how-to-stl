/**
 * @file unique_ptr_example.cpp
 * @brief Runnable tour of UniquePtr — exclusive ownership, move-only RAII
 *
 * Sections walk through the behaviors documented in unique_ptr/README.md:
 *   1. RAII destructor at scope exit
 *   2. makeUnique (exception-safe factory)
 *   3. Move semantics (source becomes empty)
 *   4. reset / release (replace vs relinquish without delete)
 *   5. UniquePtr<T[]> and makeUniqueArray
 *   6. Custom deleter (LoggingDeleter)
 *   7. Polymorphic UniquePtr<Base> from Derived
 *   8. Sink (by value) vs borrow (const ref) parameters
 *   9. Return by value (NRVO / implicit move)
 *  10. nullptr comparisons and operator bool
 *
 * Build from repo root:
 *   g++ -std=c++14 -Wall -Wextra -Wpedantic -I. unique_ptr/unique_ptr_example.cpp -o /tmp/x_unique_ptr
 */

#include "unique_ptr/unique_ptr.hpp"
#include <iostream>
#include <string>

// ============================================================================
// EXAMPLE CLASSES — destructors print so RAII / move timing is visible
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
// CUSTOM DELETERS — stateless callables; pair with UniquePtr<T, Deleter>
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
 * Example 1: RAII — destructor runs deleter when ptr leaves scope.
 *
 *     { UniquePtr owns [Database] }  →  scope end  →  ~Resource printed
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
 * Example 2: makeUnique — preferred factory; allocation wrapped immediately.
 */
void example2_make_unique() {
    std::cout << "\n=== Example 2: Using makeUnique ===\n";
    
    auto ptr = makeUnique<Resource>("Network", 2);
    ptr->use();
    
    std::cout << "Name: " << ptr->getName() << "\n";
    std::cout << "ID: " << ptr->getId() << "\n";
}

/**
 * Example 3: Move — exclusive ownership transfers; ptr1 becomes empty.
 *
 *     BEFORE: ptr1 ──> [Cache]   ptr2 ──> nullptr
 *     AFTER:  ptr1 ──> nullptr   ptr2 ──> [Cache]
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
 * Example 4: reset replaces managed object; release gives up ownership without delete.
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
 * Example 5: Array specialization — UniquePtr<T[]>, delete[], operator[].
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
 * Example 6: Custom deleter invoked from ~UniquePtr instead of plain delete.
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
 * Example 7: UniquePtr<Derived> → UniquePtr<Base> via converting move constructor.
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
 * Example 8: Sink takes ownership (std::move); borrow keeps caller as sole owner.
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
 * Example 9: Returning UniquePtr by value — compiler elides or moves out.
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
 * Example 10: Compare to nullptr; explicit operator bool for truthiness.
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


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: pointer addresses below are from one run and will differ on yours.
 * ----------------------------------------------------------------------------
=================================================
   Custom unique_ptr Implementation Examples   
=================================================

=== Example 1: Basic Usage ===
Resource 'Database' (ID: 1) created
Using resource: Database (ID: 1)
Resource 'Database' (ID: 1) destroyed
After scope - resource has been cleaned up

=== Example 2: Using makeUnique ===
Resource 'Network' (ID: 2) created
Using resource: Network (ID: 2)
Name: Network
ID: 2
Resource 'Network' (ID: 2) destroyed

=== Example 3: Move Semantics ===
Resource 'Cache' (ID: 3) created
ptr1 owns resource: yes
After move:
  ptr1 owns resource: no
  ptr2 owns resource: yes
Using resource: Cache (ID: 3)
Resource 'Cache' (ID: 3) destroyed

=== Example 4: Reset and Release ===
Resource 'Session' (ID: 4) created
Resetting to new resource...
Resource 'NewSession' (ID: 5) created
Resource 'Session' (ID: 4) destroyed
Releasing ownership...
ptr owns resource: no
Manually deleting...
Resource 'NewSession' (ID: 5) destroyed

=== Example 5: Array Support ===
Array elements: 0 10 20 30 40 

=== Example 6: Custom Deleter ===
Resource 'LoggedResource' (ID: 6) created
Using resource: LoggedResource (ID: 6)
LoggingDeleter: Deleting object at 0xb00c00940
Resource 'LoggedResource' (ID: 6) destroyed

=== Example 7: Polymorphism ===
I am Derived
I am Derived
Derived destructor
Base destructor
Derived destructor
Base destructor

=== Example 8: Function Parameters ===
Resource 'SharedData' (ID: 8) created
Borrowing resource:
Using resource: SharedData (ID: 8)
Still own resource: yes

Transferring ownership:
Using resource: SharedData (ID: 8)
Processing complete
Resource 'SharedData' (ID: 8) destroyed
Still own resource: no

=== Example 9: Return Values ===
Resource 'ReturnedResource' (ID: 9) created
Using resource: ReturnedResource (ID: 9)
Resource 'TemporaryResource' (ID: 10) created
Using resource: TemporaryResource (ID: 10)
Resource 'TemporaryResource' (ID: 10) destroyed
(Temporary resource destroyed immediately)
Resource 'ReturnedResource' (ID: 9) destroyed

=== Example 10: Comparisons ===
Resource 'Resource1' (ID: 11) created
ptr1 == nullptr: false
ptr2 == nullptr: true
ptr1 != ptr2: true
ptr1 is valid
ptr2 is null
Resource 'Resource1' (ID: 11) destroyed

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
