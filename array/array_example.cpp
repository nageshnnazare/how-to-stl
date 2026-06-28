/**
 * @file array_example.cpp
 * @brief Runnable tour of the fixed-size Array<T, N> wrapper.
 *
 * Demonstrates:
 *   - Aggregate initialization, element access (checked and unchecked)
 *   - Iterators, fill, STL algorithms (sort, find, count, accumulate)
 *   - Multi-dimensional Array<Array<T,M>, N>, zero-sized edge case
 */
#include "array/array.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Example 1: Basic Construction and Access
void example_basic() {
    print_divider("Basic Construction and Access");
    
    // C++11 aggregate initialization
    Array<int, 5> arr = {1, 2, 3, 4, 5};
    
    std::cout << "Array elements: ";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
    
    // Access elements
    std::cout << "First: " << arr.front() << "\n";
    std::cout << "Last: " << arr.back() << "\n";
    std::cout << "arr[2]: " << arr[2] << "\n";
    
    // Bounds-checked access
    try {
        std::cout << "arr.at(10): " << arr.at(10) << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

// Example 2: Iterators and Range-based for
void example_iterators() {
    print_divider("Iterators and Range-based for");
    
    Array<double, 4> temps = {98.6, 99.1, 97.8, 98.2};
    
    std::cout << "Temperatures (range-based for): ";
    for (const auto& temp : temps) {
        std::cout << temp << "°F ";
    }
    std::cout << "\n";
    
    // Using iterators
    std::cout << "Temperatures (iterators): ";
    for (auto it = temps.begin(); it != temps.end(); ++it) {
        std::cout << *it << "°F ";
    }
    std::cout << "\n";
}

// Example 3: Fill and Modify
void example_fill_modify() {
    print_divider("Fill and Modify");
    
    Array<int, 6> arr;
    arr.fill(42);
    
    std::cout << "After fill(42): ";
    for (const auto& val : arr) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    // Modify elements
    for (size_t i = 0; i < arr.size(); ++i) {
        arr[i] = i * 10;
    }
    
    std::cout << "After modification: ";
    for (const auto& val : arr) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

// Example 4: STL Algorithms
void example_stl_algorithms() {
    print_divider("STL Algorithms");
    
    Array<int, 8> arr = {5, 2, 8, 1, 9, 3, 7, 4};
    
    std::cout << "Original: ";
    for (const auto& val : arr) std::cout << val << " ";
    std::cout << "\n";
    
    // Sort
    std::sort(arr.begin(), arr.end());
    std::cout << "Sorted: ";
    for (const auto& val : arr) std::cout << val << " ";
    std::cout << "\n";
    
    // Find
    auto it = std::find(arr.begin(), arr.end(), 7);
    if (it != arr.end()) {
        std::cout << "Found 7 at position: " << (it - arr.begin()) << "\n";
    }
    
    // Count
    Array<int, 10> counts = {1, 2, 3, 2, 1, 2, 3, 2, 1, 2};
    int twos = std::count(counts.begin(), counts.end(), 2);
    std::cout << "Count of 2s: " << twos << "\n";
    
    // Accumulate
    int sum = std::accumulate(arr.begin(), arr.end(), 0);
    std::cout << "Sum of elements: " << sum << "\n";
}

// Example 5: Multi-dimensional Arrays
void example_multidimensional() {
    print_divider("Multi-dimensional Arrays");
    
    // 2D array (3x3 matrix)
    Array<Array<int, 3>, 3> matrix = {{
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    }};
    
    std::cout << "3x3 Matrix:\n";
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "\n";
    }
    
    // Calculate diagonal sum
    int diag_sum = 0;
    for (size_t i = 0; i < 3; ++i) {
        diag_sum += matrix[i][i];
    }
    std::cout << "Diagonal sum: " << diag_sum << "\n";
}

// Example 6: Zero-sized Array (edge case)
void example_zero_sized() {
    print_divider("Zero-sized Array");
    
    Array<int, 0> empty;
    std::cout << "Empty array size: " << empty.size() << "\n";
    std::cout << "Is empty: " << (empty.empty() ? "yes" : "no") << "\n";
    
    // Note: accessing elements is undefined behavior for zero-sized arrays
}

// Example 7: Aggregate Type (POD compatibility)
void example_aggregate_pod() {
    print_divider("Aggregate Type / POD Compatibility");
    
    // Array is an aggregate type, so it can be initialized with braces
    Array<int, 3> arr1 = {10, 20, 30};
    Array<int, 3> arr2 = {10, 20};  // Remaining elements zero-initialized
    
    std::cout << "arr1: ";
    for (const auto& val : arr1) std::cout << val << " ";
    std::cout << "\n";
    
    std::cout << "arr2 (partial init): ";
    for (const auto& val : arr2) std::cout << val << " ";
    std::cout << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Array Container Examples              ║\n";
    std::cout << "║   Fixed-size Array Wrapper (std::array)        ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_basic();
    example_iterators();
    example_fill_modify();
    example_stl_algorithms();
    example_multidimensional();
    example_zero_sized();
    example_aggregate_pod();
    
    std::cout << "\n✅ All Array examples completed successfully!\n";
    return 0;
}


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║          Array Container Examples              ║
║   Fixed-size Array Wrapper (std::array)        ║
╚════════════════════════════════════════════════╝

=== Basic Construction and Access ===
Array elements: 1 2 3 4 5 
First: 1
Last: 5
arr[2]: 3
arr.at(10): Caught exception: Array::at

=== Iterators and Range-based for ===
Temperatures (range-based for): 98.6°F 99.1°F 97.8°F 98.2°F 
Temperatures (iterators): 98.6°F 99.1°F 97.8°F 98.2°F 

=== Fill and Modify ===
After fill(42): 42 42 42 42 42 42 
After modification: 0 10 20 30 40 50 

=== STL Algorithms ===
Original: 5 2 8 1 9 3 7 4 
Sorted: 1 2 3 4 5 7 8 9 
Found 7 at position: 5
Count of 2s: 5
Sum of elements: 39

=== Multi-dimensional Arrays ===
3x3 Matrix:
1 2 3 
4 5 6 
7 8 9 
Diagonal sum: 15

=== Zero-sized Array ===
Empty array size: 0
Is empty: yes

=== Aggregate Type / POD Compatibility ===
arr1: 10 20 30 
arr2 (partial init): 10 20 0 

✅ All Array examples completed successfully!
 * ============================================================================ */
