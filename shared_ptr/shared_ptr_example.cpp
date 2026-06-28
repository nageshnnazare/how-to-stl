/**
 * @file shared_ptr_example.cpp
 * @brief Runnable tour of SharedPtr / WeakPtr — reference-counted shared ownership
 *
 * Sections demonstrate control-block semantics from shared_ptr/README.md:
 *   1. Copy increases shared_count; inner scope decrement
 *   2. makeShared factory
 *   3. Move transfers one handle without changing total count
 *   4. Vectors of SharedPtr (shared ownership across containers)
 *   5. WeakPtr — no strong bump; lock() promotes; expired after last SharedPtr
 *   6. Cyclic graph broken by WeakPtr parent edge
 *   7. Polymorphic SharedPtr<Base>
 *   8. dynamic_pointer_cast
 *   9. By-value copy vs const-ref borrow in parameters
 *  10. Return by value
 *  11. Equality compares raw addresses
 *  12. reset / unique()
 *  13. Cache pattern — WeakPtr entries expire when objects die
 *
 * Build from repo root:
 *   g++ -std=c++14 -Wall -Wextra -Wpedantic -I. shared_ptr/shared_ptr_example.cpp -o /tmp/x_shared_ptr
 */

#include "shared_ptr/shared_ptr.hpp"
#include <iostream>
#include <string>
#include <vector>

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

// Base and derived classes for polymorphism
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

// Node graph for cycle demo: strong child_, weak parent_ breaks SharedPtr cycles
class Node;

class Node {
private:
    std::string name_;
    SharedPtr<Node> next_;      // Strong reference - would cause circular ref
    WeakPtr<Node> parent_;      // Weak reference - breaks circular ref

public:
    Node(const std::string& name) : name_(name) {
        std::cout << "Node '" << name_ << "' created\n";
    }

    ~Node() {
        std::cout << "Node '" << name_ << "' destroyed\n";
    }

    void setNext(const SharedPtr<Node>& next) {
        next_ = next;
    }

    void setParent(const SharedPtr<Node>& parent) {
        parent_ = parent;
    }

    std::string getName() const { return name_; }

    SharedPtr<Node> getNext() const { return next_; }

    SharedPtr<Node> getParent() const {
        return parent_.lock();  // Convert weak to shared
    }
};

// ============================================================================
// EXAMPLE FUNCTIONS
// ============================================================================

/**
 * Example 1: Copy shares cb_; shared_count tracks live SharedPtr handles.
 */
void example1_shared_ownership() {
    std::cout << "\n=== Example 1: Shared Ownership ===\n";
    
    SharedPtr<Resource> ptr1(new Resource("SharedData", 1));
    std::cout << "ptr1 use_count: " << ptr1.use_count() << "\n";
    
    {
        SharedPtr<Resource> ptr2 = ptr1;  // Share ownership
        std::cout << "After copy:\n";
        std::cout << "  ptr1 use_count: " << ptr1.use_count() << "\n";
        std::cout << "  ptr2 use_count: " << ptr2.use_count() << "\n";
        
        SharedPtr<Resource> ptr3 = ptr2;  // Another copy
        std::cout << "After another copy:\n";
        std::cout << "  ptr1 use_count: " << ptr1.use_count() << "\n";
        std::cout << "  ptr2 use_count: " << ptr2.use_count() << "\n";
        std::cout << "  ptr3 use_count: " << ptr3.use_count() << "\n";
    }
    
    std::cout << "After inner scope:\n";
    std::cout << "  ptr1 use_count: " << ptr1.use_count() << "\n";
    std::cout << "Resource still alive!\n";
}

/**
 * Example 2: makeShared — two allocations (object + control block) in this impl.
 */
void example2_make_shared() {
    std::cout << "\n=== Example 2: Using makeShared ===\n";
    
    auto ptr = makeShared<Resource>("MadeResource", 2);
    std::cout << "use_count: " << ptr.use_count() << "\n";
    ptr->use();
}

/**
 * Example 3: Move — ptr1 emptied; total use_count unchanged (seat moved, not copied).
 */
void example3_move_semantics() {
    std::cout << "\n=== Example 3: Move Semantics ===\n";
    
    auto ptr1 = makeShared<Resource>("Moveable", 3);
    std::cout << "ptr1 use_count: " << ptr1.use_count() << "\n";
    
    auto ptr2 = std::move(ptr1);
    std::cout << "After move:\n";
    std::cout << "  ptr1 use_count: " << ptr1.use_count() << "\n";
    std::cout << "  ptr2 use_count: " << ptr2.use_count() << "\n";
    
    // Unlike unique_ptr, moving a shared_ptr doesn't change the total ref count
    // It just transfers one reference
}

/**
 * Example 4: Container copy duplicates handles → same object, higher use_count.
 */
void example4_containers() {
    std::cout << "\n=== Example 4: Containers ===\n";
    
    std::vector<SharedPtr<Resource>> resources;
    
    resources.push_back(makeShared<Resource>("Resource1", 4));
    resources.push_back(makeShared<Resource>("Resource2", 5));
    resources.push_back(makeShared<Resource>("Resource3", 6));
    
    std::cout << "Vector contains " << resources.size() << " resources\n";
    
    // Share ownership with another container
    std::vector<SharedPtr<Resource>> shared_resources = resources;
    
    std::cout << "After copying vector:\n";
    std::cout << "  First resource use_count: " << resources[0].use_count() << "\n";
    
    std::cout << "Clearing original vector...\n";
    resources.clear();
    
    std::cout << "After clearing original:\n";
    std::cout << "  First resource use_count: " << shared_resources[0].use_count() << "\n";
    std::cout << "  Resources still alive in second vector!\n";
}

/**
 * Example 5: WeakPtr observes via cb_; lock() CAS-increments shared_count if alive.
 */
void example5_weak_ptr() {
    std::cout << "\n=== Example 5: WeakPtr Basics ===\n";
    
    WeakPtr<Resource> weak;
    
    {
        auto shared = makeShared<Resource>("Temporary", 7);
        std::cout << "shared use_count: " << shared.use_count() << "\n";
        
        weak = shared;  // Create weak reference
        std::cout << "After creating weak_ptr:\n";
        std::cout << "  shared use_count: " << shared.use_count() << " (unchanged!)\n";
        std::cout << "  weak use_count: " << weak.use_count() << "\n";
        std::cout << "  weak expired: " << (weak.expired() ? "yes" : "no") << "\n";
        
        // Lock weak_ptr to get shared_ptr
        SharedPtr<Resource> locked = weak.lock();
        if (locked) {
            std::cout << "Locked weak_ptr successfully\n";
            std::cout << "  locked use_count: " << locked.use_count() << "\n";
            locked->use();
        }
    }
    
    std::cout << "After shared_ptr destroyed:\n";
    std::cout << "  weak expired: " << (weak.expired() ? "yes" : "no") << "\n";
    
    SharedPtr<Resource> locked = weak.lock();
    if (!locked) {
        std::cout << "  Cannot lock - object was deleted\n";
    }
}

/**
 * Example 6: Parent→child strong, child→parent weak — no reference cycle leak.
 */
void example6_circular_reference() {
    std::cout << "\n=== Example 6: Breaking Circular References ===\n";
    
    auto parent = makeShared<Node>("Parent");
    auto child = makeShared<Node>("Child");
    
    // Set up parent-child relationship
    parent->setNext(child);      // Parent -> Child (strong)
    child->setParent(parent);    // Child -> Parent (weak!)
    
    std::cout << "Parent use_count: " << parent.use_count() << "\n";
    std::cout << "Child use_count: " << child.use_count() << "\n";
    
    // Access parent through child's weak_ptr
    auto parent_from_child = child->getParent();
    if (parent_from_child) {
        std::cout << "Child's parent: " << parent_from_child->getName() << "\n";
    }
    
    std::cout << "Leaving scope - both nodes will be destroyed\n";
    std::cout << "(No memory leak because we used weak_ptr!)\n";
}

/**
 * Example 7: SharedPtr<Base> from makeShared<Derived> — virtual dispatch preserved.
 */
void example7_polymorphism() {
    std::cout << "\n=== Example 7: Polymorphism ===\n";
    
    SharedPtr<Base> ptr1 = makeShared<Derived>();
    ptr1->speak();  // Calls Derived::speak()
    
    std::cout << "use_count: " << ptr1.use_count() << "\n";
    
    // Can copy and share polymorphic pointers
    SharedPtr<Base> ptr2 = ptr1;
    std::cout << "After copy, use_count: " << ptr2.use_count() << "\n";
    ptr2->speak();
}

/**
 * Example 8: dynamic_pointer_cast shares cb_; empty SharedPtr on failed cast.
 */
void example8_dynamic_cast() {
    std::cout << "\n=== Example 8: Dynamic Cast ===\n";
    
    SharedPtr<Base> base = makeShared<Derived>();
    std::cout << "base use_count: " << base.use_count() << "\n";
    
    // Try to cast to Derived
    SharedPtr<Derived> derived = dynamic_pointer_cast<Derived>(base);
    if (derived) {
        std::cout << "Cast successful!\n";
        std::cout << "  base use_count: " << base.use_count() << "\n";
        std::cout << "  derived use_count: " << derived.use_count() << "\n";
        derived->speak();
    } else {
        std::cout << "Cast failed\n";
    }
    
    // Try invalid cast
    SharedPtr<Base> base2 = makeShared<Base>();
    SharedPtr<Derived> derived2 = dynamic_pointer_cast<Derived>(base2);
    if (!derived2) {
        std::cout << "Cannot cast Base to Derived (as expected)\n";
    }
}

/**
 * Example 9: Pass-by-value copies handle (+1 count); const ref borrows (no bump).
 */
void processShared(SharedPtr<Resource> ptr) {
    // Takes shared ownership
    std::cout << "In processShared, use_count: " << ptr.use_count() << "\n";
    ptr->use();
}

void borrowShared(const SharedPtr<Resource>& ptr) {
    // Borrows without affecting ref count
    std::cout << "In borrowShared, use_count: " << ptr.use_count() << "\n";
    ptr->use();
}

void example9_function_parameters() {
    std::cout << "\n=== Example 9: Function Parameters ===\n";
    
    auto ptr = makeShared<Resource>("FunctionParam", 9);
    std::cout << "Initial use_count: " << ptr.use_count() << "\n";
    
    std::cout << "\nBorrowing (const ref):\n";
    borrowShared(ptr);
    std::cout << "After borrow, use_count: " << ptr.use_count() << "\n";
    
    std::cout << "\nSharing (by value):\n";
    processShared(ptr);
    std::cout << "After process, use_count: " << ptr.use_count() << "\n";
}

/**
 * Example 10: Return SharedPtr — ownership shared with caller (count ≥ 1).
 */
SharedPtr<Resource> createResource(const std::string& name, int id) {
    return makeShared<Resource>(name, id);
}

void example10_return_values() {
    std::cout << "\n=== Example 10: Return Values ===\n";
    
    auto ptr = createResource("ReturnedResource", 10);
    std::cout << "use_count: " << ptr.use_count() << "\n";
    ptr->use();
}

/**
 * Example 11: operator== compares get() addresses, not use_count.
 */
void example11_comparisons() {
    std::cout << "\n=== Example 11: Comparisons ===\n";
    
    auto ptr1 = makeShared<Resource>("Resource1", 11);
    auto ptr2 = ptr1;  // Share ownership
    SharedPtr<Resource> ptr3 = makeShared<Resource>("Resource2", 12);
    SharedPtr<Resource> empty;
    
    std::cout << "ptr1 == ptr2: " << (ptr1 == ptr2 ? "true" : "false") << "\n";
    std::cout << "ptr1 == ptr3: " << (ptr1 == ptr3 ? "true" : "false") << "\n";
    std::cout << "empty == nullptr: " << (empty == nullptr ? "true" : "false") << "\n";
    
    if (ptr1) {
        std::cout << "ptr1 is valid\n";
    }
    
    if (!empty) {
        std::cout << "empty is null\n";
    }
}

/**
 * Example 12: reset drops this handle; unique() true only when use_count()==1.
 */
void example12_reset_unique() {
    std::cout << "\n=== Example 12: Reset and Unique ===\n";
    
    auto ptr1 = makeShared<Resource>("Original", 13);
    std::cout << "ptr1 unique: " << (ptr1.unique() ? "yes" : "no") << "\n";
    
    auto ptr2 = ptr1;
    std::cout << "After copy:\n";
    std::cout << "  ptr1 unique: " << (ptr1.unique() ? "yes" : "no") << "\n";
    std::cout << "  ptr1 use_count: " << ptr1.use_count() << "\n";
    
    ptr2.reset();
    std::cout << "After ptr2.reset():\n";
    std::cout << "  ptr1 unique: " << (ptr1.unique() ? "yes" : "no") << "\n";
    std::cout << "  ptr1 use_count: " << ptr1.use_count() << "\n";
    
    ptr1.reset(new Resource("NewResource", 14));
    std::cout << "After ptr1.reset(new):\n";
    ptr1->use();
}

/**
 * Example 13: Cache holds WeakPtr — entries expire when last SharedPtr dies.
 */
void example13_cache_pattern() {
    std::cout << "\n=== Example 13: Cache Pattern with WeakPtr ===\n";
    
    std::vector<WeakPtr<Resource>> cache;
    
    {
        auto res1 = makeShared<Resource>("Cached1", 15);
        auto res2 = makeShared<Resource>("Cached2", 16);
        
        cache.push_back(res1);
        cache.push_back(res2);
        
        std::cout << "Cache contains " << cache.size() << " entries\n";
        
        // Try to access cached items
        for (size_t i = 0; i < cache.size(); ++i) {
            auto locked = cache[i].lock();
            if (locked) {
                std::cout << "  Cache hit: ";
                locked->use();
            }
        }
        
        // Keep res2 alive for demonstration
        (void)res2;
    }
    
    std::cout << "\nAfter resources destroyed:\n";
    for (size_t i = 0; i < cache.size(); ++i) {
        auto locked = cache[i].lock();
        if (locked) {
            std::cout << "  Cache hit: ";
            locked->use();
        } else {
            std::cout << "  Cache miss (object deleted)\n";
        }
    }
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=================================================\n";
    std::cout << "   Custom shared_ptr Implementation Examples   \n";
    std::cout << "=================================================\n";
    
    try {
        example1_shared_ownership();
        example2_make_shared();
        example3_move_semantics();
        example4_containers();
        example5_weak_ptr();
        example6_circular_reference();
        example7_polymorphism();
        example8_dynamic_cast();
        example9_function_parameters();
        example10_return_values();
        example11_comparisons();
        example12_reset_unique();
        example13_cache_pattern();
        
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
 * ----------------------------------------------------------------------------

 * ============================================================================ */
