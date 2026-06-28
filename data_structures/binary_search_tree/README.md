# Binary Search Tree

A rooted binary tree where every node satisfies the **ordering invariant**:
all values in the left subtree are less than the node, and all values in the
right subtree are greater. An in-order traversal therefore visits keys in
sorted order.

## Picture

```
              (8)
             /   \
          (3)     (10)
         /   \       \
      (1)   (6)     (14)
           /
        (4)

In-order: 1 3 4 6 8 10 14
```

Erase handles three cases: leaf (unlink), one child (splice), two children
(replace with in-order successor — the minimum of the right subtree).

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `insert` / `contains` / `erase` | O(h) | h = height; O(log N) if balanced |
| `min` / `max` | O(h) | walk to leftmost / rightmost |
| `in_order` | O(N) | sorted visit or `std::vector` collect |
| `height` | O(N) | recursive measure |
| `clear` | O(N) | post-order delete |

## Usage

```cpp
#include "binary_search_tree.hpp"

ds::BST<int> tree;
tree.insert(8);
tree.insert(3);
tree.insert(10);
for (int x : tree.in_order()) std::cout << x << ' ';  // 3 8 10
tree.erase(3);
```

## Build & run

```bash
make run-binary_search_tree        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. binary_search_tree.cpp -o demo && ./demo
```

## Things to watch

- Inserted order affects shape — sorted input builds a linked-list-like tree
  with O(N) height. Use an AVL or red-black tree when balance matters.
- Duplicates are ignored on insert (no change if the key is already present).
- `erase` with two children copies the successor's *value* into the node, then
  deletes the successor — pointers stay valid, no rotation needed.
