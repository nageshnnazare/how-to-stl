/**
 * @file deque_example.cpp
 * @brief Runnable tour of the hand-rolled Deque (chunk map, both-end growth).
 *
 * Demonstrates:
 *   - Construction, double-ended push/pop, random access
 *   - Iterators, move semantics, comparisons, resize, emplace
 *   - Queue (FIFO), stack (LIFO), sliding-window, and alternating growth
 */

#include "deque/deque.hpp"
#include <iostream>
#include <string>

// Helper function to print deque
template<typename T>
void print_deque(const Deque<T>& dq, const char* name) {
    std::cout << name << ": [";
    for (size_t i = 0; i < dq.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << dq[i];
    }
    std::cout << "] (size=" << dq.size() << ")\n";
}

// ============================================================================
// EXAMPLE FUNCTIONS
// ============================================================================

/**
 * Example 1: Basic construction
 */
void example1_construction() {
    std::cout << "\n=== Example 1: Construction ===\n";
    
    // Default constructor
    Deque<int> dq1;
    print_deque(dq1, "Empty deque");
    
    // Construct with size
    Deque<int> dq2(5);
    print_deque(dq2, "Deque(5)");
    
    // Construct with size and value
    Deque<int> dq3(5, 42);
    print_deque(dq3, "Deque(5, 42)");
    
    // Initializer list
    Deque<int> dq4 = {1, 2, 3, 4, 5};
    print_deque(dq4, "Deque{1,2,3,4,5}");
    
    // Copy constructor
    Deque<int> dq5(dq4);
    print_deque(dq5, "Copy of dq4");
}

/**
 * Example 2: Double-ended operations
 */
void example2_double_ended() {
    std::cout << "\n=== Example 2: Double-Ended Operations ===\n";
    
    Deque<int> dq;
    
    // Add to back
    std::cout << "Adding to back: 1, 2, 3\n";
    dq.push_back(1);
    dq.push_back(2);
    dq.push_back(3);
    print_deque(dq, "After push_back");
    
    // Add to front
    std::cout << "\nAdding to front: -1, -2, -3\n";
    dq.push_front(-1);
    dq.push_front(-2);
    dq.push_front(-3);
    print_deque(dq, "After push_front");
    
    // Remove from front
    std::cout << "\nRemoving from front twice\n";
    dq.pop_front();
    dq.pop_front();
    print_deque(dq, "After pop_front");
    
    // Remove from back
    std::cout << "\nRemoving from back once\n";
    dq.pop_back();
    print_deque(dq, "After pop_back");
}

/**
 * Example 3: Element access
 */
void example3_element_access() {
    std::cout << "\n=== Example 3: Element Access ===\n";
    
    Deque<int> dq = {10, 20, 30, 40, 50};
    
    // Subscript operator
    std::cout << "dq[0] = " << dq[0] << "\n";
    std::cout << "dq[2] = " << dq[2] << "\n";
    
    // at() with bounds checking
    try {
        std::cout << "dq.at(1) = " << dq.at(1) << "\n";
        std::cout << "Trying dq.at(100)...\n";
        int x = dq.at(100);
        (void)x;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    // front() and back()
    std::cout << "dq.front() = " << dq.front() << "\n";
    std::cout << "dq.back() = " << dq.back() << "\n";
    
    // Modify elements
    dq[0] = 100;
    dq.back() = 500;
    print_deque(dq, "After modification");
}

/**
 * Example 4: Iterators
 */
void example4_iterators() {
    std::cout << "\n=== Example 4: Iterators ===\n";
    
    Deque<int> dq = {1, 2, 3, 4, 5};
    
    // Forward iteration
    std::cout << "Forward: ";
    for (auto it = dq.begin(); it != dq.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    // Range-based for loop
    std::cout << "Range-based: ";
    for (int x : dq) {
        std::cout << x << " ";
    }
    std::cout << "\n";
    
    // Modify via iterator
    for (int& x : dq) {
        x *= 2;
    }
    print_deque(dq, "After multiplying by 2");
}

/**
 * Example 5: Move semantics
 */
void example5_move_semantics() {
    std::cout << "\n=== Example 5: Move Semantics ===\n";
    
    Deque<int> dq1 = {1, 2, 3, 4, 5};
    print_deque(dq1, "dq1 before move");
    
    // Move constructor
    Deque<int> dq2(std::move(dq1));
    print_deque(dq1, "dq1 after move");
    print_deque(dq2, "dq2 after move construction");
    
    // Move assignment
    Deque<int> dq3;
    dq3 = std::move(dq2);
    print_deque(dq2, "dq2 after move assignment");
    print_deque(dq3, "dq3 after move assignment");
}

/**
 * Example 6: Comparison operators
 */
void example6_comparisons() {
    std::cout << "\n=== Example 6: Comparisons ===\n";
    
    Deque<int> dq1 = {1, 2, 3};
    Deque<int> dq2 = {1, 2, 3};
    Deque<int> dq3 = {1, 2, 4};
    Deque<int> dq4 = {1, 2};
    
    std::cout << "dq1 = "; print_deque(dq1, "");
    std::cout << "dq2 = "; print_deque(dq2, "");
    std::cout << "dq3 = "; print_deque(dq3, "");
    std::cout << "dq4 = "; print_deque(dq4, "");
    
    std::cout << "\ndq1 == dq2: " << (dq1 == dq2 ? "true" : "false") << "\n";
    std::cout << "dq1 != dq3: " << (dq1 != dq3 ? "true" : "false") << "\n";
    std::cout << "dq1 < dq3: " << (dq1 < dq3 ? "true" : "false") << "\n";
    std::cout << "dq4 < dq1: " << (dq4 < dq1 ? "true" : "false") << "\n";
}

/**
 * Example 7: Resize operations
 */
void example7_resize() {
    std::cout << "\n=== Example 7: Resize ===\n";
    
    Deque<int> dq = {1, 2, 3};
    print_deque(dq, "Initial");
    
    // Grow
    dq.resize(6);
    print_deque(dq, "After resize(6)");
    
    // Grow with value
    dq.resize(10, 99);
    print_deque(dq, "After resize(10, 99)");
    
    // Shrink
    dq.resize(4);
    print_deque(dq, "After resize(4)");
}

/**
 * Example 8: Working with strings
 */
void example8_strings() {
    std::cout << "\n=== Example 8: Deque of Strings ===\n";
    
    Deque<std::string> words;
    words.push_back("World");
    words.push_front("Hello");
    words.push_back("!");
    
    std::cout << "Words: [";
    for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "\"" << words[i] << "\"";
    }
    std::cout << "]\n";
}

/**
 * Example 9: emplace operations
 */
void example9_emplace() {
    std::cout << "\n=== Example 9: Emplace ===\n";
    
    // Custom class
    struct Point {
        int x, y;
        Point(int x_, int y_) : x(x_), y(y_) {
            std::cout << "  Point(" << x << ", " << y << ") constructed\n";
        }
    };
    
    Deque<Point> points;
    
    std::cout << "Using emplace_back:\n";
    points.emplace_back(1, 2);
    
    std::cout << "\nUsing emplace_front:\n";
    points.emplace_front(0, 0);
    
    std::cout << "\nPoints: ";
    for (size_t i = 0; i < points.size(); ++i) {
        std::cout << "(" << points[i].x << "," << points[i].y << ") ";
    }
    std::cout << "\n";
}

/**
 * Example 10: Clear and swap
 */
void example10_clear_swap() {
    std::cout << "\n=== Example 10: Clear and Swap ===\n";
    
    Deque<int> dq1 = {1, 2, 3, 4, 5};
    Deque<int> dq2 = {10, 20, 30};
    
    print_deque(dq1, "dq1 before");
    print_deque(dq2, "dq2 before");
    
    // Swap
    dq1.swap(dq2);
    std::cout << "\nAfter swap:\n";
    print_deque(dq1, "dq1");
    print_deque(dq2, "dq2");
    
    // Clear
    dq1.clear();
    std::cout << "\nAfter dq1.clear():\n";
    print_deque(dq1, "dq1");
}

/**
 * Example 11: Alternating push operations
 */
void example11_alternating() {
    std::cout << "\n=== Example 11: Alternating Operations ===\n";
    
    Deque<int> dq;
    
    std::cout << "Building deque with alternating push_front/push_back:\n";
    for (int i = 1; i <= 5; ++i) {
        dq.push_back(i);
        dq.push_front(-i);
        print_deque(dq, "  Step");
    }
}

/**
 * Example 12: Queue simulation (FIFO)
 */
void example12_queue_simulation() {
    std::cout << "\n=== Example 12: Queue Simulation (FIFO) ===\n";
    
    Deque<std::string> queue;
    
    // Enqueue (add to back)
    std::cout << "Enqueuing: Alice, Bob, Charlie\n";
    queue.push_back("Alice");
    queue.push_back("Bob");
    queue.push_back("Charlie");
    print_deque(queue, "Queue");
    
    // Dequeue (remove from front)
    std::cout << "\nDequeuing: " << queue.front() << "\n";
    queue.pop_front();
    print_deque(queue, "Queue after dequeue");
    
    std::cout << "\nDequeuing: " << queue.front() << "\n";
    queue.pop_front();
    print_deque(queue, "Queue after dequeue");
}

/**
 * Example 13: Stack simulation (LIFO)
 */
void example13_stack_simulation() {
    std::cout << "\n=== Example 13: Stack Simulation (LIFO) ===\n";
    
    Deque<int> stack;
    
    // Push (add to back)
    std::cout << "Pushing: 10, 20, 30\n";
    stack.push_back(10);
    stack.push_back(20);
    stack.push_back(30);
    print_deque(stack, "Stack");
    
    // Pop (remove from back)
    std::cout << "\nPopping: " << stack.back() << "\n";
    stack.pop_back();
    print_deque(stack, "Stack after pop");
    
    std::cout << "\nPopping: " << stack.back() << "\n";
    stack.pop_back();
    print_deque(stack, "Stack after pop");
}

/**
 * Example 14: Sliding window pattern
 */
void example14_sliding_window() {
    std::cout << "\n=== Example 14: Sliding Window ===\n";
    
    Deque<int> window;
    int data[] = {1, 3, 5, 7, 9, 11, 13, 15};
    const int window_size = 3;
    
    std::cout << "Data: [1, 3, 5, 7, 9, 11, 13, 15]\n";
    std::cout << "Window size: " << window_size << "\n\n";
    
    for (int i = 0; i < 8; ++i) {
        window.push_back(data[i]);
        
        if (window.size() > static_cast<size_t>(window_size)) {
            window.pop_front();
        }
        
        std::cout << "After adding " << data[i] << ": ";
        print_deque(window, "");
    }
}

/**
 * Example 15: Performance demonstration
 */
void example15_performance() {
    std::cout << "\n=== Example 15: Performance Demo ===\n";
    
    Deque<int> dq;
    
    std::cout << "Adding 100 elements alternating front/back:\n";
    for (int i = 0; i < 50; ++i) {
        dq.push_front(i);
        dq.push_back(-i);
    }
    
    std::cout << "Final size: " << dq.size() << "\n";
    std::cout << "First 10: [";
    for (int i = 0; i < 10 && i < static_cast<int>(dq.size()); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << dq[i];
    }
    std::cout << "]\n";
    
    std::cout << "Last 10: [";
    for (int i = static_cast<int>(dq.size()) - 10; i < static_cast<int>(dq.size()); ++i) {
        if (i > static_cast<int>(dq.size()) - 10) std::cout << ", ";
        std::cout << dq[i];
    }
    std::cout << "]\n";
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=================================================\n";
    std::cout << "     Custom Deque Class Examples                \n";
    std::cout << "=================================================\n";
    
    try {
        example1_construction();
        example2_double_ended();
        example3_element_access();
        example4_iterators();
        example5_move_semantics();
        example6_comparisons();
        example7_resize();
        example8_strings();
        example9_emplace();
        example10_clear_swap();
        example11_alternating();
        example12_queue_simulation();
        example13_stack_simulation();
        example14_sliding_window();
        example15_performance();
        
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
=================================================
     Custom Deque Class Examples                
=================================================

=== Example 1: Construction ===
Empty deque: [] (size=0)
Deque(5): [0, 0, 0, 0, 0] (size=5)
Deque(5, 42): [42, 42, 42, 42, 42] (size=5)
Deque{1,2,3,4,5}: [1, 2, 3, 4, 5] (size=5)
Copy of dq4: [1, 2, 3, 4, 5] (size=5)

=== Example 2: Double-Ended Operations ===
Adding to back: 1, 2, 3
After push_back: [1, 2, 3] (size=3)

Adding to front: -1, -2, -3
After push_front: [-3, -2, -1, 1, 2, 3] (size=6)

Removing from front twice
After pop_front: [-1, 1, 2, 3] (size=4)

Removing from back once
After pop_back: [-1, 1, 2] (size=3)

=== Example 3: Element Access ===
dq[0] = 10
dq[2] = 30
dq.at(1) = 20
Trying dq.at(100)...
Caught: Deque::at: position out of range
dq.front() = 10
dq.back() = 50
After modification: [100, 20, 30, 40, 500] (size=5)

=== Example 4: Iterators ===
Forward: 1 2 3 4 5 
Range-based: 1 2 3 4 5 
After multiplying by 2: [2, 4, 6, 8, 10] (size=5)

=== Example 5: Move Semantics ===
dq1 before move: [1, 2, 3, 4, 5] (size=5)
dq1 after move: [] (size=0)
dq2 after move construction: [1, 2, 3, 4, 5] (size=5)
dq2 after move assignment: [] (size=0)
dq3 after move assignment: [1, 2, 3, 4, 5] (size=5)

=== Example 6: Comparisons ===
dq1 = : [1, 2, 3] (size=3)
dq2 = : [1, 2, 3] (size=3)
dq3 = : [1, 2, 4] (size=3)
dq4 = : [1, 2] (size=2)

dq1 == dq2: true
dq1 != dq3: true
dq1 < dq3: true
dq4 < dq1: true

=== Example 7: Resize ===
Initial: [1, 2, 3] (size=3)
After resize(6): [1, 2, 3, 0, 0, 0] (size=6)
After resize(10, 99): [1, 2, 3, 0, 0, 0, 99, 99, 99, 99] (size=10)
After resize(4): [1, 2, 3, 0] (size=4)

=== Example 8: Deque of Strings ===
Words: ["Hello", "World", "!"]

=== Example 9: Emplace ===
Using emplace_back:
  Point(1, 2) constructed

Using emplace_front:
  Point(0, 0) constructed

Points: (0,0) (1,2) 

=== Example 10: Clear and Swap ===
dq1 before: [1, 2, 3, 4, 5] (size=5)
dq2 before: [10, 20, 30] (size=3)

After swap:
dq1: [10, 20, 30] (size=3)
dq2: [1, 2, 3, 4, 5] (size=5)

After dq1.clear():
dq1: [] (size=0)

=== Example 11: Alternating Operations ===
Building deque with alternating push_front/push_back:
  Step: [-1, 1] (size=2)
  Step: [-2, -1, 1, 2] (size=4)
  Step: [-3, -2, -1, 1, 2, 3] (size=6)
  Step: [-4, -3, -2, -1, 1, 2, 3, 4] (size=8)
  Step: [-5, -4, -3, -2, -1, 1, 2, 3, 4, 5] (size=10)

=== Example 12: Queue Simulation (FIFO) ===
Enqueuing: Alice, Bob, Charlie
Queue: [Alice, Bob, Charlie] (size=3)

Dequeuing: Alice
Queue after dequeue: [Bob, Charlie] (size=2)

Dequeuing: Bob
Queue after dequeue: [Charlie] (size=1)

=== Example 13: Stack Simulation (LIFO) ===
Pushing: 10, 20, 30
Stack: [10, 20, 30] (size=3)

Popping: 30
Stack after pop: [10, 20] (size=2)

Popping: 20
Stack after pop: [10] (size=1)

=== Example 14: Sliding Window ===
Data: [1, 3, 5, 7, 9, 11, 13, 15]
Window size: 3

After adding 1: : [1] (size=1)
After adding 3: : [1, 3] (size=2)
After adding 5: : [1, 3, 5] (size=3)
After adding 7: : [3, 5, 7] (size=3)
After adding 9: : [5, 7, 9] (size=3)
After adding 11: : [7, 9, 11] (size=3)
After adding 13: : [9, 11, 13] (size=3)
After adding 15: : [11, 13, 15] (size=3)

=== Example 15: Performance Demo ===
Adding 100 elements alternating front/back:
Final size: 100
First 10: [49, 48, 47, 46, 45, 44, 43, 42, 41, 40]
Last 10: [-40, -41, -42, -43, -44, -45, -46, -47, -48, -49]

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
