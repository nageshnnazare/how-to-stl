# Dynamic Array

A resizable array — the data structure behind `std::vector`. Elements live
contiguously in a single heap block; when it fills, we allocate a bigger block,
copy across, and free the old one.

## Picture

```
size=4, capacity=8
index:   0    1    2    3    4    5    6    7
       +----+----+----+----+----+----+----+----+
data_->| 10 | 20 | 30 | 40 |    |    |    |    |
       +----+----+----+----+----+----+----+----+
        \________ used ____/ \_____ spare ____/
```

When `size == capacity` and another element arrives, capacity **doubles**.
Doubling is what makes `push_back` *amortized* O(1): across N pushes the total
copy work is `N + N/2 + N/4 + ... < 2N`.

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `push_back` / `pop_back` | O(1) amortized | grows by doubling |
| `operator[]` / `at` | O(1) | `at` is bounds-checked (throws) |
| `insert(i, x)` / `erase(i)` | O(N) | shifts the tail |
| `find(x)` | O(N) | linear scan |
| `reserve(n)` | O(N) | pre-grow to avoid reallocations |

## Usage

```cpp
#include "dynamic_array.hpp"

ds::DynamicArray<int> a;
a.push_back(10);
a.push_back(20);
a.insert(1, 15);     // a = [10, 15, 20]
a.erase(0);          // a = [15, 20]
for (int x : a) std::cout << x << ' ';
```

## Build & run

```bash
make run-dynamic_array        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. dynamic_array.cpp -o demo && ./demo
```

## Things to watch

- `operator[]` is **unchecked** — use `at()` when an index might be invalid.
- `insert`/`erase` in the middle are O(N); if you do that a lot, a linked list
  or deque may fit better.
- After a growth, all pointers/references into the array are invalidated
  (the buffer moved).
