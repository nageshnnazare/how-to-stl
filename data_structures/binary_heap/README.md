# Binary Heap

A nearly-complete binary tree stored in a `std::vector`. With the default
`Compare = std::less<T>` this is a **max-heap**: the largest element is always
at index 0 (`top()`). Use `std::greater<T>` for a min-heap.

## Picture

```
Array index:  0   1   2   3   4   5   6
             [90][80][70][30][50][60][40]

Tree:              (90)
               /        \
            (80)        (70)
           /   \       /
        (30) (50)  (60)

parent(i)      = (i - 1) / 2
left_child(i)  = 2*i + 1
right_child(i) = 2*i + 2
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `push` / `pop` | O(log N) | sift_up / sift_down |
| `top` | O(1) | root of the heap |
| `build` / `build_heap` | O(N) | Floyd heapify |
| `empty` / `size` | O(1) | vector metadata |

## Usage

```cpp
#include "binary_heap.hpp"

ds::BinaryHeap<int> heap;          // max-heap (default)
heap.push(4);
heap.push(9);
std::cout << heap.top();           // 9
heap.pop();

auto h = ds::BinaryHeap<int>::build({3, 1, 4, 1, 5});
```

## Build & run

```bash
make run-binary_heap        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. binary_heap.cpp -o demo && ./demo
```

## Things to watch

- Default `std::less` → **max-heap** (parent is not less than children).
- `pop` moves the last array slot to the root, then sifts down — O(log N).
- Repeated `pop` from a max-heap is **heapsort** (descending pops); reverse or
  use a min-heap for ascending order.
