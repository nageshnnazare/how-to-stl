# Set Implementation

Red-Black Tree based ordered set of unique elements.

## Features
- **Ordered**: Elements sorted automatically
- **Unique**: No duplicates allowed
- **O(log n)**: Insert, find, erase operations
- **Bidirectional iterators**: Forward and backward traversal
- **Red-Black Tree**: Self-balancing for performance

## Quick Example
```cpp
Set<int> s = {5, 2, 8, 1, 9};
// Automatically sorted: 1, 2, 5, 8, 9

s.insert(3);           // Add element
bool has = s.contains(5);  // Check membership
s.erase(2);            // Remove element

for (int x : s) {      // Iterate in order
    std::cout << x << " ";
}
```

## Operations
- `insert(value)` - Add element (O(log n))
- `erase(value)` - Remove element (O(log n))
- `find(value)` - Find element (O(log n))
- `contains(value)` - Check if exists (O(log n))
- `count(value)` - Returns 0 or 1 (O(log n))
- `clear()` - Remove all elements
- `size()`, `empty()` - Capacity queries

See `set_example.cpp` for comprehensive examples.
