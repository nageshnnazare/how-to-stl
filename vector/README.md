# Vector Class Implementation

A complete implementation of a dynamic array container similar to `std::vector`.

## Overview

The `Vector` class template provides a dynamic array with automatic memory management, efficient element access, and a rich set of operations.

### Key Features

- **Dynamic Memory**: Automatically grows and shrinks as needed
- **Geometric Growth**: Capacity doubles when full (amortized O(1) push_back)
- **RAII**: Automatic cleanup, no memory leaks
- **Move Semantics**: Efficient transfers of ownership
- **Exception Safety**: Strong exception guarantees
- **Random Access**: O(1) element access by index
- **Rich API**: Compatible with std::vector interface

## Quick Start

```cpp
#include "vector/vector.hpp"

// Create and populate vector
Vector<int> numbers = {1, 2, 3, 4, 5};

// Add elements
numbers.push_back(6);
numbers.emplace_back(7);

// Access elements
int first = numbers[0];
int last = numbers.back();

// Iterate
for (int n : numbers) {
    std::cout << n << " ";
}

// Modify
numbers.insert(numbers.begin() + 2, 99);
numbers.erase(numbers.begin());

// Resize
numbers.resize(10, 0);
```

## Construction

### Default Constructor
```cpp
Vector<int> vec;  // Empty vector
```
- Size: 0
- Capacity: 0
- No memory allocated

### Size Constructor
```cpp
Vector<int> vec(10);  // 10 default-constructed elements
```
- Creates n default-constructed elements
- Size equals capacity

### Size + Value Constructor
```cpp
Vector<int> vec(5, 42);  // {42, 42, 42, 42, 42}
```
- Creates n copies of value
- Useful for initialization

### Initializer List
```cpp
Vector<int> vec = {1, 2, 3, 4, 5};
```
- Convenient syntax for literals
- Size equals capacity initially

### Copy Constructor
```cpp
Vector<int> vec2(vec1);  // Deep copy
```
- Creates independent copy
- O(n) time complexity

### Move Constructor
```cpp
Vector<int> vec2(std::move(vec1));  // Steals resources
```
- O(1) time complexity
- Original vector becomes empty

## Element Access

### Subscript Operator []
```cpp
int x = vec[2];     // No bounds checking
vec[0] = 100;       // Direct access
```
- **Fast**: No overhead
- **Unsafe**: No bounds checking
- Use when index is guaranteed valid

### at() Method
```cpp
try {
    int x = vec.at(100);  // Throws if out of range
} catch (const std::out_of_range& e) {
    // Handle error
}
```
- **Safe**: Throws on invalid index
- **Slower**: Has bounds check
- Use when safety matters

### Front and Back
```cpp
int first = vec.front();
int last = vec.back();

vec.front() = 10;
vec.back() = 20;
```
- Access first/last element
- Undefined if vector is empty

### data()
```cpp
int* ptr = vec.data();
// ptr points to internal array
```
- Returns pointer to underlying array
- Use for C API interop

## Capacity Management

### Size vs Capacity

```cpp
Vector<int> vec;
vec.reserve(10);    // capacity = 10, size = 0
vec.push_back(1);   // capacity = 10, size = 1
```

- **Size**: Number of elements
- **Capacity**: Allocated space
- Capacity ≥ size always

### reserve()
```cpp
vec.reserve(1000);  // Pre-allocate space
```
- Ensures capacity ≥ n
- Avoids reallocations
- Use when size is known upfront

### shrink_to_fit()
```cpp
vec.shrink_to_fit();  // capacity = size
```
- Reduces capacity to size
- Frees unused memory
- Use after removing many elements

### Growth Pattern
```cpp
// Capacity growth demonstration:
Initial: capacity = 0
After 1st push: capacity = 1
After 2nd push: capacity = 2
After 3rd push: capacity = 4
After 5th push: capacity = 8
After 9th push: capacity = 16
```
- Geometric growth (doubling)
- Amortized O(1) push_back
- Log(n) reallocations for n elements

## Modifiers

### push_back()
```cpp
vec.push_back(42);              // Copy
vec.push_back(std::move(obj));  // Move
```
- Adds element at end
- Reallocates if capacity full
- O(1) amortized

### emplace_back()
```cpp
struct Point { int x, y; };
vec.emplace_back(10, 20);  // Constructs in-place
```
- Constructs element directly
- Avoids temporary objects
- More efficient than push_back

### pop_back()
```cpp
vec.pop_back();  // Removes last element
```
- Destroys last element
- Does not change capacity
- O(1)

### insert()
```cpp
vec.insert(vec.begin() + 2, 99);     // Insert 99 at position 2
vec.insert(vec.end(), 100);          // Append
vec.insert(vec.begin(), value);      // Prepend
```
- Inserts element at position
- Shifts elements right
- O(n) worst case

### erase()
```cpp
vec.erase(vec.begin() + 2);              // Erase single element
vec.erase(vec.begin(), vec.begin() + 3); // Erase range
```
- Removes element(s)
- Shifts elements left
- O(n) worst case

### resize()
```cpp
vec.resize(10);      // Resize to 10, default-construct new elements
vec.resize(20, 99);  // Resize to 20, fill new elements with 99
```
- Changes size
- Constructs or destroys elements
- May reallocate

### clear()
```cpp
vec.clear();  // Remove all elements
```
- Destroys all elements
- Size becomes 0
- Capacity unchanged

### swap()
```cpp
vec1.swap(vec2);  // Swap contents
```
- Exchanges contents
- O(1) - just swaps pointers
- No element copies

## Iterators

### Basic Iteration
```cpp
// Forward iteration
for (auto it = vec.begin(); it != vec.end(); ++it) {
    std::cout << *it << " ";
}

// Range-based for
for (int x : vec) {
    std::cout << x << " ";
}

// Modify via iterator
for (int& x : vec) {
    x *= 2;
}
```

### Const Iterators
```cpp
for (auto it = vec.cbegin(); it != vec.cend(); ++it) {
    // *it is const, cannot modify
}
```

### Iterator Arithmetic
```cpp
auto it = vec.begin();
it += 5;                  // Move forward 5 positions
auto mid = vec.begin() + vec.size() / 2;  // Middle element
```

## Comparisons

### Equality
```cpp
Vector<int> v1 = {1, 2, 3};
Vector<int> v2 = {1, 2, 3};
Vector<int> v3 = {1, 2, 4};

bool b1 = (v1 == v2);  // true
bool b2 = (v1 != v3);  // true
```
- Element-wise comparison
- Must have same size and elements

### Relational
```cpp
bool b1 = (v1 < v3);   // Lexicographic comparison
bool b2 = (v1 <= v2);
bool b3 = (v3 > v1);
bool b4 = (v2 >= v1);
```
- Lexicographic ordering
- Like string comparison

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Construction | O(n) | Where n is initial size |
| Element access [] | O(1) | No bounds check |
| Element access at() | O(1) | With bounds check |
| push_back | O(1)* | Amortized |
| pop_back | O(1) | |
| insert at end | O(1)* | Amortized |
| insert at position | O(n) | Must shift elements |
| erase at end | O(1) | |
| erase at position | O(n) | Must shift elements |
| resize | O(n) | If growing |
| reserve | O(n) | If growing |
| clear | O(n) | Must destroy elements |
| swap | O(1) | Just swaps pointers |

\* Amortized constant time due to geometric growth

## Memory Layout

```
Stack:                          Heap:
┌──────────────┐               ┌─────┬─────┬─────┬─────┬─────┐
│ data_    ────┼──────────────→│  0  │  1  │  2  │  3  │  4  │
│ size_ = 3    │               └─────┴─────┴─────┴─────┴─────┘
│ capacity_= 5 │                 Used          Unused
└──────────────┘
```

- **data_**: Pointer to heap-allocated array
- **size_**: Current number of elements
- **capacity_**: Total allocated space

## Common Patterns

### Pre-allocate for Known Size
```cpp
Vector<int> vec;
vec.reserve(1000);  // Avoid reallocations
for (int i = 0; i < 1000; ++i) {
    vec.push_back(i);
}
```

### Build and Shrink
```cpp
Vector<int> vec;
// ... add many elements ...
vec.erase(/* remove some */);
vec.shrink_to_fit();  // Free unused memory
```

### 2D Array
```cpp
Vector<Vector<int>> matrix(rows);
for (auto& row : matrix) {
    row.resize(cols);
}
```

### Move to Function
```cpp
void process(Vector<int> vec) {  // Takes ownership
    // ...
}

Vector<int> data = {1, 2, 3};
process(std::move(data));  // Efficient transfer
```

## Exception Safety

### Strong Guarantee
Most operations provide strong exception guarantee:
- If exception thrown, vector unchanged
- Examples: insert, push_back, resize

### Basic Guarantee
Some operations provide basic guarantee:
- Vector valid but state unspecified
- Example: some move operations

### No-throw Guarantee
Some operations never throw:
- swap, move constructor, move assignment (if T's move doesn't throw)

## Comparison with std::vector

### Similarities
- Same interface (mostly)
- Same performance characteristics
- Same memory layout
- Same exception guarantees

### Differences
- No allocator support
- No reverse iterators
- No emplace (only emplace_back)
- Simpler implementation

## Best Practices

### ✅ Do
- Use reserve() when final size is known
- Prefer emplace_back() for efficiency
- Use move semantics for large objects
- Pass by const reference when not transferring ownership
- Use range-based for loops

### ❌ Don't
- Don't store pointers to elements (may be invalidated)
- Don't manually manage memory (RAII handles it)
- Don't use [] without bounds checking in untrusted code
- Don't resize() in loops (expensive)

## Thread Safety

### ❌ Not Thread-Safe
- Concurrent modifications are unsafe
- Concurrent reads are safe
- Use external synchronization

```cpp
// UNSAFE
thread1: vec.push_back(1);
thread2: vec.push_back(2);

// SAFE
thread1: int x = vec[0];
thread2: int y = vec[1];
```

## Memory Management

### No Leaks
```cpp
{
    Vector<std::string> vec;
    vec.push_back("Hello");
    // Automatically cleaned up
}  // Destructor frees memory
```

### Move Efficiency
```cpp
Vector<HugeObject> v1;
// ... populate v1 ...

Vector<HugeObject> v2 = std::move(v1);  // O(1), no copies
```

## Example: Building a Dynamic 2D Grid

```cpp
// Create 10x10 grid
Vector<Vector<int>> grid(10);
for (auto& row : grid) {
    row.resize(10, 0);
}

// Set values
grid[5][5] = 42;

// Iterate
for (size_t i = 0; i < grid.size(); ++i) {
    for (size_t j = 0; j < grid[i].size(); ++j) {
        std::cout << grid[i][j] << " ";
    }
    std::cout << "\n";
}
```

## Files

- **vector.hpp**: Implementation (header-only)
- **vector_example.cpp**: Comprehensive examples
- **vector_test.cpp**: 39 unit tests

## See Also

- **unique_ptr**: For single object ownership
- **shared_ptr**: For shared ownership
- **String**: For string handling

## Build and Run

```bash
# Build examples
make run-vector

# Run tests
make test-vector

# Build everything
make all
```

---

**Implementation Status**: ✅ Complete with full std::vector API

