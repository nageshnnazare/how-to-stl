# AVL Tree

A self-balancing binary search tree. Every node stores subtree height; the
**balance factor** `height(left) - height(right)` must stay in `{-1, 0, 1}`.
After insert (or erase), rotations restore balance in O(log N).

## Picture

```
Balance factor at each node = height(left) - height(right)  in {-1,0,1}

LL fix (right rotation):          RR fix (left rotation):

    z                                  z
   /                                    \
  y          -->        y              y          <--        z
 /                     /   \                          /       /
x                    x     z                        y       x
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `insert` / `contains` / `erase` | O(log N) | rebalance with rotations |
| `in_order` | O(N) | sorted traversal |
| `height` | O(1) | cached at root |
| `size` | O(1) | tracked on insert/erase |

## Usage

```cpp
#include "avl_tree.hpp"

ds::AVLTree<int> tree;
for (int i = 1; i <= 100; ++i) tree.insert(i);  // stays balanced
for (int x : tree.in_order()) std::cout << x << ' ';
```

## Build & run

```bash
make run-avl_tree        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. avl_tree.cpp -o demo && ./demo
```

## Things to watch

- Rotations preserve the BST ordering invariant — only pointers rearrange.
- Four cases: LL, RR, LR, RL (double rotation when the new leaf is on the
  "inside" of the heavy side).
- AVL is stricter than red-black trees (faster lookups, slightly more rotations
  on insert).
