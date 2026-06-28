# Set — Ordered Unique Elements (Red-Black Tree)

> A `Set<T>` keeps every value at most once, always in sorted order. You do not
> choose where an element lands — the tree's ordering does. Insert, erase, and
> lookup are O(log n) because the backing structure is a self-balancing
> Red-Black binary search tree, not a flat array.

This is a from-scratch reimplementation of `std::set` built for learning. The
header is [`set.hpp`](set.hpp), runnable examples are in
[`set_example.cpp`](set_example.cpp), and the test suite is in
[`../tests/set_test.cpp`](../tests/set_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | Red-Black binary search tree (heap-allocated nodes) |
| Order | Sorted by `Compare` (default: ascending) |
| Duplicates | **No** — second insert of same value is ignored |
| Random access | **No** — use iterators |
| Object size | 4 fields (`root_`, `nil_`, `size_`, `comp_`) — typically 32 bytes on 64-bit |

**Reach for a Set when** you need fast membership tests, unique elements, or
sorted traversal without maintaining sort order yourself.

**Look elsewhere when** you need duplicates ([`multiset`](../multiset/README.md)),
key→value mapping ([`map`](../map/README.md)), or O(1) average hashing
([`unordered_set`](../unordered_set/README.md) if present).

---

## 2. Mental Model

Think of a binary search tree where every "empty" child is a special sentinel
node `nil_` (always BLACK), not `nullptr`:

```
   Set object (stack)              Tree (B=black, r=red)
   ┌──────────────┐
   │ root_   ●────┼──▶        (5:B)
   │ nil_    ●────┼──┐       /       \
   │ size_ = 4    │  │    (2:B)     (8:B)
   └──────────────┘  │       \       /
                     │      (nil)  (9:r)
                     └────▶ nil_ (sentinel, self-linked)
```

- **In-order walk** (left, node, right) prints elements in sorted order.
- **`end()`** is an iterator pointing at `nil_` — one past the largest element.
- **Red-Black rules** keep height ≤ 2·log₂(n+1), so every operation touches
  only O(log n) nodes.

---

## 3. Internal Representation

```cpp
enum Color { RED, BLACK };

struct Node {
    T value;
    Color color;
    Node* left;    // child or nil_
    Node* right;   // child or nil_
    Node* parent;
};

Node*   root_;   // tree top; == nil_ when empty
Node*   nil_;    // sentinel leaf (BLACK, points to itself)
size_t  size_;   // element count
Compare comp_;   // strict weak ordering (default std::less<T>)
```

**Invariants:**

1. Every node is RED or BLACK; root is BLACK.
2. All leaves are `nil_` (no raw `nullptr` children).
3. No two consecutive RED nodes on any root-to-`nil_` path.
4. Every root-to-`nil_` path has the same black-node count (black-height).

### Why the nil_ sentinel?

Rotations and fixups always read `node->left->color` without null checks.
`nil_` is a real node (dummy value, BLACK) whose `left`, `right`, and `parent`
all point back to `nil_` itself.

---

## 4. How It Works (Step by Step)

### 4.1 Search (`find` / `contains`)

Classic BST descent from `root_`:

```
   find 6:  start at root 5 → go right (6 > 5) → found at right child
```

O(log n) comparisons; height is bounded by Red-Black invariants.

### 4.2 Insert

```
   (1) Walk BST to leaf slot; reject if key already present.
   (2) Attach new RED leaf under parent (or make root).
   (3) insert_fixup — recolor or rotate until invariants restored.
   (4) Force root BLACK.
```

**insert_fixup cases** (parent is left child of grandparent; mirror for right):

| Uncle color | Shape | Action |
|---|---|---|
| RED | parent & uncle both RED | Recolor parent, uncle → BLACK; grandparent → RED; climb |
| BLACK | "line" (z outer child) | Recolor + rotate grandparent |
| BLACK | "triangle" (z inner child) | Rotate parent to line, then line case |

### 4.3 Left rotation (building block)

```
   BEFORE:          AFTER:
       x                y
      / \              / \
     α   y     →      x   γ
        / \          / \
       β   γ        α   β
```

`β` (y's left child) slides under `x`; `y` takes `x`'s place in the tree.

### 4.4 Erase

BST delete with three cases for node `z`:

```
   0 children:  transplant(nil_)     — snip leaf
   1 child:     transplant(child)   — promote only child
   2 children:  y = min(right); swap data/links; delete y (≤1 child)
```

If the physically removed node `y` was **BLACK**, `delete_fixup` fixes the
"extra black" deficit on one root-to-leaf path via sibling recoloring and
rotations.

### 4.5 Iterator increment (in-order successor)

```
   if right child exists:  node = minimum(right)
   else:                   climb until we came from left; that parent is next
```

`--` is the symmetric predecessor walk (uses `maximum` on left subtree).

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Set<T>()` | empty tree + sentinel |
| `Set<T>{a, b, c}` | insert each (duplicates dropped) |
| copy / move ctor | deep clone / O(1) steal nodes + sentinel |

### Modifiers
| Call | Returns | Notes |
|---|---|---|
| `insert(value)` | `{iterator, bool}` | `bool` false if duplicate |
| `erase(value)` | `0` or `1` | by key |
| `erase(iterator)` | iterator to next | |
| `clear()` | — | deletes all nodes, keeps sentinel |

### Lookup
| Call | Complexity | Notes |
|---|---|---|
| `find(value)` | O(log n) | `end()` if missing |
| `contains(value)` | O(log n) | |
| `count(value)` | O(log n) | always 0 or 1 |

### Iterators
`begin()` → smallest element; `end()` → `nil_`. Bidirectional (`++` / `--`).

### Capacity
`empty()`, `size()`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `insert` | O(log n) | ≤ 2 rotations + O(log n) recolor |
| `erase` | O(log n) | delete_fixup ≤ O(log n) |
| `find`, `contains`, `count` | O(log n) | BST height |
| `begin` / `end` | O(log n) / O(1) | `begin` walks left spine |
| iterator `++` / `--` | amortized O(1) | occasional climb |
| copy | O(n) | move is O(1) |
| `clear` | O(n) | post-order delete |

---

## 7. Usage

```cpp
#include "set/set.hpp"

Set<int> s = {5, 2, 8, 1, 9};   // sorted: 1 2 5 8 9

auto [it, ok] = s.insert(5);     // ok == false (duplicate)
s.insert(3);

if (s.contains(8)) { /* ... */ }

s.erase(2);

for (int x : s) {                // in-order: 1 3 5 8 9
    std::cout << x << ' ';
}
```

See [`set_example.cpp`](set_example.cpp) for insertion results, erase,
bidirectional iteration, strings, copy/move, and deduplication from an array.

---

## 8. Design Decisions & Trade-offs

- **Red-Black over AVL.** Fewer rotations on insert/delete; still O(log n).
  Slightly looser balance than AVL but simpler fixup code for teaching.
- **nil_ sentinel.** Eliminates nullptr branches in rotations and fixups at
  the cost of one extra heap node per set.
- **Iterators hold raw `Node*`.** Erasing the node an iterator points to
  invalidates it (same as `std::set`).
- **No allocator parameter.** Every node uses `new`/`delete` for clarity.

---

## 9. Common Pitfalls

- **Assuming insertion order is preserved.** Only sorted order is guaranteed.
  ```cpp
  Set<int> s; s.insert(5); s.insert(1);  // iteration prints 1 then 5
  ```
- **Using iterators after erase.** `erase(it)` returns the next valid iterator;
  do not dereference the old one.
- **`insert` does not replace.** Existing key → `{iterator, false}`; value unchanged.
- **Compare must define strict weak ordering.** Undefined behavior if `comp_(a,b)`
  and `comp_(b,a)` are both false yet `a != b`.

---

## 10. Comparison with `std::set`

**Same:** sorted unique elements, `insert`/`erase`/`find` complexity, bidirectional
iterators, Red-Black-style balancing semantics.

**Intentionally omitted for clarity:** custom allocators, `emplace`, `emplace_hint`,
`lower_bound`/`upper_bound`, `extract`/`merge`, node handles, exception-safe
iterator invalidation guarantees beyond basics.

---

## 11. Build & Run

```bash
make run-set      # build + run the examples
make test-set     # build + run the unit tests
make all          # build everything in the repo
```

Or manually from the repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. set/set_example.cpp -o /tmp/x_set
/tmp/x_set
```

---

## 12. See Also

- [`map`](../map/README.md) — unique keys mapped to values (same RB-tree core)
- [`multiset`](../multiset/README.md) — sorted elements **with** duplicates
- [`multimap`](../multimap/README.md) — duplicate keys allowed
- [`vector`](../vector/README.md) — contiguous storage when order is insertion order
