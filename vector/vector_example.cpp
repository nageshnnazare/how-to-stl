/**
 * @file vector_example.cpp
 * @brief Comprehensive examples demonstrating the Vector class implementation
 * 
 * This file shows various use cases and features of the custom Vector class
 */

#include "vector/vector.hpp"
#include <iostream>
#include <string>

// Helper function to print vector
template<typename T>
void print_vector(const Vector<T>& vec, const char* name) {
    std::cout << name << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << vec[i];
    }
    std::cout << "] (size=" << vec.size() << ", capacity=" << vec.capacity() << ")\n";
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
    Vector<int> v1;
    print_vector(v1, "Empty vector");
    
    // Construct with size
    Vector<int> v2(5);
    print_vector(v2, "Vector(5)");
    
    // Construct with size and value
    Vector<int> v3(5, 42);
    print_vector(v3, "Vector(5, 42)");
    
    // Initializer list
    Vector<int> v4 = {1, 2, 3, 4, 5};
    print_vector(v4, "Vector{1,2,3,4,5}");
    
    // Copy constructor
    Vector<int> v5(v4);
    print_vector(v5, "Copy of v4");
}

/**
 * Example 2: Element access
 */
void example2_element_access() {
    std::cout << "\n=== Example 2: Element Access ===\n";
    
    Vector<int> vec = {10, 20, 30, 40, 50};
    
    // Subscript operator
    std::cout << "vec[0] = " << vec[0] << "\n";
    std::cout << "vec[2] = " << vec[2] << "\n";
    
    // at() with bounds checking
    try {
        std::cout << "vec.at(1) = " << vec.at(1) << "\n";
        std::cout << "Trying vec.at(100)...\n";
        int x = vec.at(100);
        (void)x;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    // front() and back()
    std::cout << "vec.front() = " << vec.front() << "\n";
    std::cout << "vec.back() = " << vec.back() << "\n";
    
    // Modify elements
    vec[0] = 100;
    vec.back() = 500;
    print_vector(vec, "After modification");
}

/**
 * Example 3: Capacity management
 */
void example3_capacity() {
    std::cout << "\n=== Example 3: Capacity ===\n";
    
    Vector<int> vec;
    std::cout << "Empty vector:\n";
    std::cout << "  size: " << vec.size() << "\n";
    std::cout << "  capacity: " << vec.capacity() << "\n";
    std::cout << "  empty: " << (vec.empty() ? "yes" : "no") << "\n";
    
    // Reserve capacity
    vec.reserve(10);
    std::cout << "\nAfter reserve(10):\n";
    std::cout << "  capacity: " << vec.capacity() << "\n";
    std::cout << "  size: " << vec.size() << " (unchanged)\n";
    
    // Add elements
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    print_vector(vec, "After adding 5 elements");
    
    // Shrink to fit
    vec.shrink_to_fit();
    std::cout << "After shrink_to_fit(): capacity=" << vec.capacity() << "\n";
}

/**
 * Example 4: push_back and pop_back
 */
void example4_push_pop() {
    std::cout << "\n=== Example 4: Push and Pop ===\n";
    
    Vector<int> vec;
    
    // Push elements
    std::cout << "Pushing elements 1-5:\n";
    for (int i = 1; i <= 5; ++i) {
        vec.push_back(i);
        print_vector(vec, "  After push");
    }
    
    // Pop elements
    std::cout << "\nPopping 2 elements:\n";
    vec.pop_back();
    print_vector(vec, "  After first pop");
    vec.pop_back();
    print_vector(vec, "  After second pop");
}

/**
 * Example 5: Insert operations
 */
void example5_insert() {
    std::cout << "\n=== Example 5: Insert ===\n";
    
    Vector<int> vec = {1, 2, 4, 5};
    print_vector(vec, "Initial");
    
    // Insert at position
    vec.insert(vec.begin() + 2, 3);
    print_vector(vec, "After insert(begin()+2, 3)");
    
    // Insert at beginning
    vec.insert(vec.begin(), 0);
    print_vector(vec, "After insert(begin(), 0)");
    
    // Insert at end
    vec.insert(vec.end(), 6);
    print_vector(vec, "After insert(end(), 6)");
}

/**
 * Example 6: Erase operations
 */
void example6_erase() {
    std::cout << "\n=== Example 6: Erase ===\n";
    
    Vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    print_vector(vec, "Initial");
    
    // Erase single element
    vec.erase(vec.begin() + 5);
    print_vector(vec, "After erase(begin()+5)");
    
    // Erase range
    vec.erase(vec.begin() + 2, vec.begin() + 5);
    print_vector(vec, "After erase(begin()+2, begin()+5)");
}

/**
 * Example 7: Resize
 */
void example7_resize() {
    std::cout << "\n=== Example 7: Resize ===\n";
    
    Vector<int> vec = {1, 2, 3};
    print_vector(vec, "Initial");
    
    // Grow
    vec.resize(6);
    print_vector(vec, "After resize(6)");
    
    // Grow with value
    vec.resize(10, 99);
    print_vector(vec, "After resize(10, 99)");
    
    // Shrink
    vec.resize(4);
    print_vector(vec, "After resize(4)");
}

/**
 * Example 8: Iterators
 */
void example8_iterators() {
    std::cout << "\n=== Example 8: Iterators ===\n";
    
    Vector<int> vec = {1, 2, 3, 4, 5};
    
    // Forward iteration
    std::cout << "Forward: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    // Range-based for loop
    std::cout << "Range-based: ";
    for (int x : vec) {
        std::cout << x << " ";
    }
    std::cout << "\n";
    
    // Modify via iterator
    for (int& x : vec) {
        x *= 2;
    }
    print_vector(vec, "After multiplying by 2");
}

/**
 * Example 9: Move semantics
 */
void example9_move_semantics() {
    std::cout << "\n=== Example 9: Move Semantics ===\n";
    
    Vector<int> v1 = {1, 2, 3, 4, 5};
    print_vector(v1, "v1 before move");
    
    // Move constructor
    Vector<int> v2(std::move(v1));
    print_vector(v1, "v1 after move");
    print_vector(v2, "v2 after move construction");
    
    // Move assignment
    Vector<int> v3;
    v3 = std::move(v2);
    print_vector(v2, "v2 after move assignment");
    print_vector(v3, "v3 after move assignment");
}

/**
 * Example 10: Comparison operators
 */
void example10_comparisons() {
    std::cout << "\n=== Example 10: Comparisons ===\n";
    
    Vector<int> v1 = {1, 2, 3};
    Vector<int> v2 = {1, 2, 3};
    Vector<int> v3 = {1, 2, 4};
    Vector<int> v4 = {1, 2};
    
    std::cout << "v1 = "; print_vector(v1, "");
    std::cout << "v2 = "; print_vector(v2, "");
    std::cout << "v3 = "; print_vector(v3, "");
    std::cout << "v4 = "; print_vector(v4, "");
    
    std::cout << "\nv1 == v2: " << (v1 == v2 ? "true" : "false") << "\n";
    std::cout << "v1 != v3: " << (v1 != v3 ? "true" : "false") << "\n";
    std::cout << "v1 < v3: " << (v1 < v3 ? "true" : "false") << "\n";
    std::cout << "v4 < v1: " << (v4 < v1 ? "true" : "false") << "\n";
}

/**
 * Example 11: Working with strings
 */
void example11_strings() {
    std::cout << "\n=== Example 11: Vector of Strings ===\n";
    
    Vector<std::string> words;
    words.push_back("Hello");
    words.push_back("World");
    words.push_back("from");
    words.push_back("C++");
    
    std::cout << "Words: [";
    for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "\"" << words[i] << "\"";
    }
    std::cout << "]\n";
    
    // Insert
    words.insert(words.begin() + 2, "the");
    std::cout << "After inserting 'the': [";
    for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "\"" << words[i] << "\"";
    }
    std::cout << "]\n";
}

/**
 * Example 12: emplace_back
 */
void example12_emplace() {
    std::cout << "\n=== Example 12: Emplace ===\n";
    
    // Custom class
    struct Point {
        int x, y;
        Point(int x_, int y_) : x(x_), y(y_) {
            std::cout << "  Point(" << x << ", " << y << ") constructed\n";
        }
    };
    
    Vector<Point> points;
    
    std::cout << "Using push_back (creates temporary):\n";
    points.push_back(Point(1, 2));
    
    std::cout << "\nUsing emplace_back (constructs in-place):\n";
    points.emplace_back(3, 4);
    
    std::cout << "\nPoints: ";
    for (size_t i = 0; i < points.size(); ++i) {
        std::cout << "(" << points[i].x << "," << points[i].y << ") ";
    }
    std::cout << "\n";
}

/**
 * Example 13: Clear and swap
 */
void example13_clear_swap() {
    std::cout << "\n=== Example 13: Clear and Swap ===\n";
    
    Vector<int> v1 = {1, 2, 3, 4, 5};
    Vector<int> v2 = {10, 20, 30};
    
    print_vector(v1, "v1 before");
    print_vector(v2, "v2 before");
    
    // Swap
    v1.swap(v2);
    std::cout << "\nAfter swap:\n";
    print_vector(v1, "v1");
    print_vector(v2, "v2");
    
    // Clear
    v1.clear();
    std::cout << "\nAfter v1.clear():\n";
    print_vector(v1, "v1");
}

/**
 * Example 14: Nested vectors (2D array)
 */
void example14_nested() {
    std::cout << "\n=== Example 14: Nested Vectors (2D Array) ===\n";
    
    Vector<Vector<int>> matrix;
    
    // Create 3x3 matrix
    for (int i = 0; i < 3; ++i) {
        Vector<int> row;
        for (int j = 0; j < 3; ++j) {
            row.push_back(i * 3 + j);
        }
        matrix.push_back(std::move(row));
    }
    
    // Print matrix
    std::cout << "3x3 Matrix:\n";
    for (size_t i = 0; i < matrix.size(); ++i) {
        std::cout << "  [";
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << matrix[i][j];
        }
        std::cout << "]\n";
    }
}

/**
 * Example 15: Growth pattern
 */
void example15_growth() {
    std::cout << "\n=== Example 15: Growth Pattern ===\n";
    
    Vector<int> vec;
    std::cout << "Observing capacity growth:\n";
    
    size_t last_capacity = 0;
    for (int i = 0; i < 20; ++i) {
        vec.push_back(i);
        if (vec.capacity() != last_capacity) {
            std::cout << "  After push " << i+1 << ": capacity=" << vec.capacity() << "\n";
            last_capacity = vec.capacity();
        }
    }
    
    std::cout << "\nGeometric growth (doubling) reduces reallocations!\n";
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=================================================\n";
    std::cout << "     Custom Vector Class Examples               \n";
    std::cout << "=================================================\n";
    
    try {
        example1_construction();
        example2_element_access();
        example3_capacity();
        example4_push_pop();
        example5_insert();
        example6_erase();
        example7_resize();
        example8_iterators();
        example9_move_semantics();
        example10_comparisons();
        example11_strings();
        example12_emplace();
        example13_clear_swap();
        example14_nested();
        example15_growth();
        
        std::cout << "\n=================================================\n";
        std::cout << "   All examples completed successfully!        \n";
        std::cout << "=================================================\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

