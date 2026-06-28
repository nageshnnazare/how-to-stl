# Union-Find (Disjoint Set)

Tracks a partition of `n` items into disjoint groups. Supports fast
*merge* (`unite`) and *same-set?* (`connected`) queries via a forest of
parent pointers, with **path compression** and **union by rank**.

## Picture

```
BEFORE unite(4, 5):          AFTER unite(4, 5) (union by rank):

    0   1   2   3   4   5         0   1   2   3   4
    o   o   o   o   o   o         o   o   o   o   o
                                    \           /
                                     5         (4 is root)

find(5) with path compression rewires 5 (and nodes above) to point at the root.
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `DisjointSet(n)` | O(n) | n singletons |
| `find(x)` | O(α(n)) amortized | path compression on mutating find |
| `unite(x, y)` | O(α(n)) amortized | union by rank |
| `connected(x, y)` | O(α(n)) amortized | compares roots |
| `count()` | O(1) | number of disjoint sets |

α(n) is the inverse Ackermann function — grows so slowly it is effectively
constant for any realistic input size.

## Usage

```cpp
#include "union_find.hpp"

ds::DisjointSet uf(5);       // {0} {1} {2} {3} {4}
uf.unite(0, 1);
uf.unite(2, 3);
uf.unite(1, 2);              // {0,1,2,3} {4}

if (uf.connected(0, 3)) { /* same component */ }
std::cout << uf.count();     // 2
```

## Build & run

```bash
make run-union_find             # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. union_find.cpp -o demo && ./demo
```

## Things to watch

- Elements are `0 .. n-1`; there is no dynamic `add` — size is fixed at
  construction.
- `unite` on elements already in the same set is a no-op (count unchanged).
- Non-mutating `find` (const overload) does not compress paths; use the
  regular `find` when you want the optimization to apply.
- Classic applications: Kruskal's MST, detecting connected components,
  offline connectivity queries.
