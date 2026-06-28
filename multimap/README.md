# Multimap — Ordered Key-Value with Duplicate Keys (Sorted Vector)

> A `Multimap<Key, T>` stores many `(key, value)` pairs sorted by key. The same
> key may appear multiple times — useful for one-to-many relations (all grades
> per student). Like [`Multiset`](../multiset/README.md), the implementation is a
> sorted `std::vector`, so lookup is O(log n) but insert/erase shift in O(n).

This is a from-scratch reimplementation of `std::multimap` built for learning.
The header is [`multimap.hpp`](multimap.hpp), runnable examples are in
[`multimap_example.cpp`](multimap_example.cpp), and the test suite is in
[`../tests/multimap_test.cpp`](../tests/multimap_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | `std::vector<std::pair<Key, T>>` sorted by `pair.first` |
| Order | Keys sorted; duplicate keys grouped contiguously |
| Duplicate keys | **Yes** |
| `operator[]` | **Not provided** (ambiguous which value to return) |
| Object size | vector + comparator |

**Reach for a Multimap when** one key maps to many values and you want them
visited in key order: event log by user, inverted index postings, multiple
scores per player.

**Look elsewhere for** unique keys ([`map`](../map/README.md)), values only
([`multiset`](../multiset/README.md)), or O(log n) insert at large scale
(tree-based `std::multimap`).

---

## 2. Mental Model

Each vector slot is one association; sorting uses **key only**:

```
   ┌──────────────────┐
   │ key: "Alice"     │  ◀── determines position
   │ val: 90          │  ◀── does not affect sort among keys
   └──────────────────┘

   data_ (by key):
   index:    0           1           2           3
          [Alice,90] [Alice,95] [Alice,88] [Bob,85]
          └──────── equal_range("Alice") ────────┘
```

Iteration prints all Alice entries together, then Bob, etc.

---

## 3. Internal Representation

```cpp
std::vector<std::pair<Key, T>> data_;
Compare comp_;  // applied to .first only

struct KeyCompare { /* mixed Key / pair comparisons for lower_bound */ };
```

**Invariant:** sorted by `comp_(a.first, b.first)`; all pairs with equivalent
keys occupy one contiguous subrange.

### equal_range as a window

```
   lower_bound(key) ──▶ first index with key' >= key
   upper_bound(key) ──▶ first index with key' >  key
   equal_range      ──▶ [lower, upper)
```

---

## 4. How It Works (Step by Step)

### 4.1 Insert

```
   KeyCompare finds lower_bound by key only:
   insert({Alice, 95}) after {Alice, 90} → both sit in Alice's block.

   [Bob,85]  + insert(Alice,90)  →  [Alice,90][Bob,85]
   + insert(Alice,95)             →  [Alice,90][Alice,95][Bob,85]
```

O(log n) search + O(n) vector insert shift.

### 4.2 Find

Returns iterator to **first** pair with matching key (start of run).

### 4.3 Count

Width of equal_range — how many pairs share the key.

### 4.4 Erase by key

Removes **all** pairs with that key (entire block), like multiset erase.

### 4.5 vs `Map` (Red-Black tree)

| | Multimap (vector) | Map (RB tree) |
|---|---|---|
| Duplicate keys | Yes | No |
| Insert | O(n) | O(log n) |
| `operator[]` | N/A | Yes |
| Locality | excellent scan | pointer chasing |

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Multimap<K,T>()` | empty |
| `Multimap{{k,v},...}` | insert each pair |

### Modifiers
| Call | Returns | Notes |
|---|---|---|
| `insert({k,v})` | iterator | duplicate keys OK |
| `erase(key)` | count erased | all pairs with key |
| `clear()` | — | |

### Lookup
`find(key)`, `count(key)`, `contains(key)`

### Iterators
`begin()` / `end()` — const iterators over pairs (sorted by key).

### Capacity
`empty()`, `size()`

---

## 6. Complexity Summary

| Operation | Complexity |
|---|---|
| `insert` | O(n) |
| `erase(key)` | O(n) |
| `find`, `count`, `contains` | O(log n) |
| full iteration | O(n) |
| copy | O(n) |

---

## 7. Usage

```cpp
#include "multimap/multimap.hpp"

Multimap<std::string, int> scores;
scores.insert({"Alice", 90});
scores.insert({"Bob", 85});
scores.insert({"Alice", 95});
scores.insert({"Alice", 88});

std::cout << scores.count("Alice") << '\n';  // 3

for (const auto& p : scores) {
    std::cout << p.first << ": " << p.second << '\n';
}

scores.erase("Alice");  // removes all three Alice entries
```

See [`multimap_example.cpp`](multimap_example.cpp) for multi-key inserts, counts,
and erase-all behavior.

---

## 8. Design Decisions & Trade-offs

- **No `operator[]`.** With duplicate keys, subscript semantics are ambiguous;
  explicit `insert` is clearer.
- **Sorted vector** for parity with [`Multiset`](../multiset/README.md) — same
  equal_range / lower_bound story, extended to pairs.
- **`KeyCompare` helper** — lets `lower_bound` take a bare `Key` without
  constructing dummy pairs.

---

## 9. Common Pitfalls

- **`erase(key)` drops every pair with that key**, not a single insertion.
- **Values for the same key are not sorted** — only keys are ordered; among
  equal keys, order reflects insertion order into the block.
- **O(n) insert** — loading millions of pairs one-by-one is slow; batch + sort
  would be better for production bulk loads.

---

## 10. Comparison with `std::multimap`

**Same:** sorted keys, duplicate keys allowed, `equal_range` semantics, no
`operator[]`.

**Different:** vector backend (O(n) insert) vs tree; no `emplace`, `erase(it)`,
or allocator support in this teaching version.

---

## 11. Build & Run

```bash
make run-multimap
make test-multimap
make all
```

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. multimap/multimap_example.cpp -o /tmp/x_multimap
/tmp/x_multimap
```

---

## 12. See Also

- [`map`](../map/README.md) — unique keys, Red-Black tree
- [`multiset`](../multiset/README.md) — duplicate values without separate mapped type
- [`set`](../set/README.md) — unique sorted elements, full RB-tree diagrams
- [`vector`](../vector/README.md) — insert shift mechanics underlying this design
