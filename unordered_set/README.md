# UnorderedSet Implementation

Hash table based set with **O(1) average** operations.

## Features
- **Unordered**: No sorting
- **Unique**: No duplicates
- **O(1) average**: Insert, find, erase
- **Hash Table**: Separate chaining implementation
- **Fast**: Better than Set for pure membership testing

## Quick Example
```cpp
UnorderedSet<int> us = {3, 1, 4, 1, 5, 9};
// Order not guaranteed

bool has = us.contains(5);  // O(1) average
us.insert(7);               // O(1) average
us.erase(3);                // O(1) average
```

Use when you don't need ordering and want maximum speed.
