# Multiset Implementation

Ordered set that **allows duplicates**.

## Features
- **Ordered**: Elements sorted
- **Allows Duplicates**: Multiple copies of same value
- **Count**: Query how many of each element
- **Sorted vector** implementation (simple & effective)

## Quick Example
```cpp
Multiset<int> ms = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
// Stores all: 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9

int n = ms.count(5);  // Returns 3
ms.erase(5);          // Removes ALL 5s
```

Use when you need to track frequencies of elements in sorted order.
