# PriorityQueue — Binary Heap Adapter

> A `PriorityQueue<T>` always exposes the **best** element according to a comparator
> (default: **largest** on top). Internally it is a **complete binary tree** stored
> in a **flat array** (`std::vector<T>`): `push` **sifts up**, `pop` **sifts down**,
> and bulk construction uses **Floyd's heapify** in O(n).

This is a from-scratch reimplementation of `std::priority_queue` built for learning. The
header is [`priority_queue.hpp`](priority_queue.hpp), runnable examples are in
[`priority_queue_example.cpp`](priority_queue_example.cpp), and the test suite is in
[`../tests/priority_queue_test.cpp`](../tests/priority_queue_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Kind | **Container adapter** over a random-access sequence |
| Default storage | `std::vector<T>` (level-order heap) |
| Default order | **Max-heap** (`std::less` — top is largest) |
| `top()` | O(1) — root at index 0 |
| `push` / `pop` | O(log n) — sift up / down |

**Reach for a PriorityQueue when** you repeatedly need the extremal element (Dijkstra,
scheduling, top-K, event simulation, heap sort).

**Look elsewhere when** you need FIFO fairness ([`queue`](../queue/README.md)) or
sorted traversal of all keys ([`set`](../set/README.md)).

---

## 2. Mental Model

Same heap as two views — tree and array:

```
   Tree (max-heap)                 c_ (level-order indices)
          50                         [0]=50  [1]=30  [2]=40
         /  \                        [3]=10  [4]=20  [5]=35  [6]=25
       30    40
      / \   / \
    10  20 35 25

   Index relations (0-based):
     parent(i) = (i - 1) / 2
     left(i)   = 2 * i + 1
     right(i)  = 2 * i + 2
```

`top()` is always `c_[0]`. The tree is **complete** — no gaps in the array.

---

## 3. Internal Representation

```cpp
Container c_;    // default std::vector<T>, heap layout
Compare comp_;   // default std::less<T> → max-heap at top()
```

**Heap property (max-heap):** for every node `i`, `c_[i]` is not less than either child
per `comp_` (equivalently: `comp_(c_[i], child)` is false when child exists).

**Min-heap:** `PriorityQueue<int, std::vector<int>, std::greater<int>>`.

---

## 4. How It Works (Step by Step)

### 4.1 Push — append leaf, sift-up

```
   push(45) into existing max-heap:

   (1) push_back(45) at index 7 (new rightmost leaf)
   (2) while comp_(parent, me): swap with parent, repeat

        50                              50
       /  \                            /  \
     30    40          →             30    45
    / \   / \                       / \   / \
  10 20 35 25                     10 20 35 25
  /
 45  → bubbles up until parent ≥ 45
```

At most `⌊log₂ n⌋` swaps.

### 4.2 Pop — replace root with last, sift-down

```
   pop() removes 50:

   (1) move c_.back() → c_[0]
   (2) pop_back()  (size shrinks)
   (3) heapify_down(0) — swap with larger child until heap restored

   root had 25 after move → sinks past 40, 35, …
```

### 4.3 Floyd's `build_heap` — O(n) bulk fix

Used by range/container constructors:

```
   start i = parent(n-1), count down to 0:
       heapify_down(i)

   Fixes subtrees bottom-up; faster than n successive pushes for large n.
```

### 4.4 Comparator semantics (read carefully)

`comp_(a, b) == true` means **a is lower priority than b**. For max-heap,
`std::less` makes the **largest** float to the top because parents are not less
than children.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `PriorityQueue<T>()` | empty max-heap |
| `PriorityQueue(cmp)` | empty with comparator |
| `PriorityQueue(cmp, cont)` | copy container + `build_heap()` |
| `PriorityQueue(first, last, cmp)` | range + heapify |
| copy / move | duplicate or steal `c_` and `comp_` |

### Element access
| Call | On empty |
|---|---|
| `top()` | throws `std::out_of_range` |

### Modifiers
`push`, `emplace`, `pop` (throws if empty), `swap`

### Debug
`is_heap()` — validates heap property

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `top` | O(1) | index 0 |
| `push` | O(log n) | sift-up height |
| `pop` | O(log n) | sift-down height |
| `build_heap` | O(n) | Floyd |
| `size`, `empty` | O(1) | |

---

## 7. Usage

```cpp
#include "priority_queue/priority_queue.hpp"

PriorityQueue<int> pq;   // max-heap
pq.push(30);
pq.push(10);
pq.push(50);
std::cout << pq.top();   // 50
pq.pop();

// min-heap
PriorityQueue<int, std::vector<int>, std::greater<int>> minpq;
minpq.push(30);
std::cout << minpq.top();  // 10
```

See [`priority_queue_example.cpp`](priority_queue_example.cpp) for range construction,
custom comparators, median-from-stream, and top-K.

---

## 8. Design Decisions & Trade-offs

- **Vector-backed binary heap** — cache-friendly, matches `std::priority_queue`.
- **0-based index formulas** — standard textbook layout; root at `front()`.
- **Throws on empty `top`/`pop`** — stricter than STL (UB there); clearer for learners.
- **Floyd build on bulk ctor** — teaches O(n) heapify vs repeated O(log n) pushes.
- **Not stable** — equal priorities may reorder; tie-breaking is unspecified.

---

## 9. Common Pitfalls

- **Comparator direction.** `std::less` → max-heap; `std::greater` → min-heap. Easy to invert.
- **Assuming FIFO among equal keys** — heap does not guarantee insertion order.
- **`top` on empty** — throws here; still UB in `std::priority_queue`.
- **Using `operator[]` mental model** — only `top()` is visible; rest is hidden in the heap.

---

## 10. Comparison with `std::priority_queue`

**Same:** vector default, comparator template param, max-heap by default, push/pop/top.

**Differences:** we throw on empty access; expose `is_heap()` for tests; same core algorithms.

---

## 11. Build & Run

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. priority_queue/priority_queue_example.cpp -o /tmp/x_priority_queue
/tmp/x_priority_queue

make run-priority_queue
make test-priority_queue
```

---

## 12. See Also

- [`vector`](../vector/README.md) — storage array under the heap
- [`queue`](../queue/README.md) — FIFO, not priority-ordered
- [`set`](../set/README.md) — sorted all-elements view, O(log n) every op
