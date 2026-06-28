# Multiset — Ordered Elements with Duplicates (Sorted Vector)

> A `Multiset<T>` is a sorted bag: every insertion is kept, including copies of
> the same value, and the whole sequence stays in order. Search and count are
> O(log n) binary search; insert and erase cost O(n) because the backing store is
> a contiguous vector that must shift elements to preserve sort and grouping.

This is a from-scratch reimplementation of `std::multiset` built for learning.
The header is [`multiset.hpp`](multiset.hpp), runnable examples are in
[`multiset_example.cpp`](multiset_example.cpp), and the test suite is in
[`../tests/multiset_test.cpp`](../tests/multiset_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | `std::vector<T>` kept sorted by `Compare` |
| Order | Sorted ascending (default) |
| Duplicates | **Yes** — equal values stored in adjacent runs |
| Random access | **Yes** via vector iterators (contiguous) |
| Object size | vector + comparator (typically 24–32 bytes for empty vector) |

**Reach for a Multiset when** you need sorted elements *and* frequency counts, e.g.
histogram buckets, rolling medians with duplicates, or "how many 5s?" queries.

**Look elsewhere when** values must be unique ([`set`](../set/README.md)), you need
O(log n) insert at scale ([`set`](../set/README.md) / libc++ `std::multiset` tree),
or key→value mapping ([`multimap`](../multimap/README.md)).

---

## 2. Mental Model

Picture a sorted array where equal keys sit together in one block:

```
   Multiset (stack)                 data_ (sorted, duplicates grouped)
   ┌──────────────┐               index:  0   1   2   3   4   5   6
   │ data_   ●────┼──────────────▶       [1][1][2][3][3][4][5][5][5]
   │ comp_        │                      └──┘     └──┘     └─────┘
   └──────────────┘                      run of 1s   3s      5s
```

- **`lower_bound(x)`** — first position where element ≥ x.
- **`upper_bound(x)`** — first position where element > x.
- **`equal_range(x)`** — `[lower, upper)` — the whole duplicate run.

---

## 3. Internal Representation

```cpp
std::vector<T> data_;  // sorted ascending by comp_
Compare        comp_;  // default std::less<T>
```

**Invariant:** `data_` is sorted with `comp_`, and all elements equal under
`comp_` occupy one contiguous subrange (required for `equal_range` / `count`).

### Binary search primitives

| Algorithm | Role in Multiset |
|---|---|
| `lower_bound` | insertion point; start of duplicate run |
| `upper_bound` | end of duplicate run (exclusive) |
| `equal_range` | `[lower, upper)` — whole run at once |

---

## 4. How It Works (Step by Step)

### 4.1 Insert

```
   (1) pos = lower_bound(value)     O(log n) — first slot not less than value
   (2) data_.insert(pos, value)     O(n) — shift elements at pos..end right
```

```
   before: [1][3][5][5]     insert 3 at lower_bound → index 1
   shift:  [1][·][3][5][5]
   write:  [1][3][3][5][5]
```

New duplicates land **with** existing equal values (stable grouping).

### 4.2 Find / contains

`lower_bound`, then check equivalence: neither `comp_(value, *it)` nor
`comp_(*it, value)` may hold.

### 4.3 Count

```
   equal_range(5) on [1][5][5][5][7]:
              └──── count = 3 ────┘
   indices [1, 4)
```

### 4.4 Erase by value

Erases the **entire** equal_range — all copies of that value in one call.

```
   erase(5): remove [5][5][5] → [1][7]
```

### 4.5 vs Red-Black `Set`

| | Multiset (this code) | Set (RB tree) |
|---|---|---|
| Duplicates | Yes | No |
| Insert | O(n) shift | O(log n) |
| Find | O(log n) | O(log n) |
| Memory | contiguous, cache-friendly scans | scattered nodes |

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Multiset<T>()` | empty |
| `Multiset{a,b,c}` | insert each (order in vector = sorted) |

### Modifiers
| Call | Returns | Notes |
|---|---|---|
| `insert(value)` | iterator to new element | O(n) |
| `erase(value)` | count removed | all duplicates |
| `clear()` | — | |

### Lookup
| Call | Complexity |
|---|---|
| `find(value)` | O(log n) |
| `count(value)` | O(log n) |
| `contains(value)` | O(log n) |

### Iterators
`begin()` / `end()` — const vector iterators; in-order = sorted order.

### Capacity
`empty()`, `size()`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `insert` | O(n) | binary search + vector shift |
| `erase(value)` | O(n) | erase range + shift |
| `find`, `count`, `contains` | O(log n) | binary search |
| scan all elements | O(n) | contiguous → cache-friendly |
| copy | O(n) | vector copy |

---

## 7. Usage

```cpp
#include "multiset/multiset.hpp"

Multiset<int> ms = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
// stored: 1 1 2 3 3 4 5 5 5 6 9

std::cout << ms.count(5) << '\n';  // 3

ms.insert(5);                      // four 5s now

ms.erase(5);                       // removes ALL 5s

for (int x : ms) { /* sorted */ }
```

See [`multiset_example.cpp`](multiset_example.cpp) for insert, count, erase-all,
and iteration.

---

## 8. Design Decisions & Trade-offs

- **Sorted vector instead of tree.** Simpler code; duplicate runs are literally
  adjacent memory. Pay O(n) on insert/erase — fine for teaching and small n.
- **No `equal_range` public API.** Logic is internal via `std::equal_range`; could
  be exposed like `std::multiset`.
- **Const iterators only.** Matches a minimal teaching surface; no in-place mutate.

---

## 9. Common Pitfalls

- **`erase(value)` removes every copy**, not just one — unlike `erase(iterator)`
  on `std::multiset`.
- **Insert is O(n).** Bulk loading many elements? Insert one-by-one is quadratic;
  sort once + merge would be faster (not implemented here).
- **Comparator must match sort invariant.** Changing `comp_` after inserts would
  break ordering (not applicable — comp is fixed per type).

---

## 10. Comparison with `std::multiset`

**Same:** sorted order, duplicates allowed, `count` / `equal_range` semantics,
`lower_bound`-style positioning.

**Different:** this uses `vector` (O(n) insert); libc++ `std::multiset` uses a
Red-Black tree (O(log n) insert). No `emplace`, allocators, or `erase(iterator)`.

---

## 11. Build & Run

```bash
make run-multiset
make test-multiset
make all
```

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. multiset/multiset_example.cpp -o /tmp/x_multiset
/tmp/x_multiset
```

---

## 12. See Also

- [`set`](../set/README.md) — unique elements, Red-Black tree (O(log n) insert)
- [`multimap`](../multimap/README.md) — duplicate **keys** with values
- [`map`](../map/README.md) — unique keys
- [`vector`](../vector/README.md) — contiguous storage mechanics
