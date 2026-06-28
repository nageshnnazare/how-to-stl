# Map — Ordered Key-Value Pairs (Red-Black Tree)

> A `Map<Key, T>` stores at most one value per key, with keys kept in sorted
> order. Look up by key, insert pairs, or walk in key order — all in O(log n)
> thanks to the same Red-Black tree machinery as [`Set`](../set/README.md),
> except each node holds a `{key, value}` pair instead of a lone element.

This is a from-scratch reimplementation of `std::map` built for learning. The
header is [`map.hpp`](map.hpp), runnable examples are in
[`map_example.cpp`](map_example.cpp), and the test suite is in
[`../tests/map_test.cpp`](../tests/map_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | Red-Black BST of `std::pair<const Key, T>` nodes |
| Order | Sorted by **key** (`Compare` on `Key`, default ascending) |
| Duplicate keys | **No** |
| Value access | `operator[]`, `at`, iterators dereference to `pair` |
| Object size | 4 fields (`root_`, `nil_`, `size_`, `comp_`) |

**Reach for a Map when** you need associative lookup: names→ages, ids→records,
counts keyed by string, etc., and you want keys sorted for range scans.

**Look elsewhere when** duplicate keys are required ([`multimap`](../multimap/README.md)),
you only need keys without values ([`set`](../set/README.md)), or average O(1)
hashing beats ordering ([`unordered_map`](../unordered_map/README.md) if present).

---

## 2. Mental Model

Each tree node is a small record:

```
   ┌──────────────────────────────┐
   │  pair: { "Alice", 30 }       │  ◀── tree order = key order only
   │  color, left, right, parent  │
   └──────────────────────────────┘

   Map (stack)                    Example tree (keys)
   ┌──────────────┐
   │ root_   ●────┼──▶     ("Bob", 25)
   │ nil_    ●    │       /              \
   │ size_ = 3    │  ("Alice",30)    ("Charlie",35)
   └──────────────┘
```

Iteration visits `(key, value)` pairs in ascending key order — not insertion order.

---

## 3. Internal Representation

```cpp
struct Node {
    std::pair<const Key, T> value;  // key is const (cannot re-key in place)
    Color color;
    Node* left, *right, *parent;
};

Node*   root_;
Node*   nil_;      // BLACK sentinel (see Set README)
size_t  size_;
Compare comp_;     // applied to value.first only
```

**Search / insert / erase** compare `comp_(key, node->value.first)` — the
mapped value `T` does not participate in tree placement.

### nil_ sentinel

Same design as `Set`: every missing child is `nil_`, so rotations and fixups
never dereference `nullptr`. See [`set/README.md`](../set/README.md) §3 for the
full sentinel diagram and five Red-Black invariants.

---

## 4. How It Works (Step by Step)

### 4.1 `operator[]` — find or default-insert

```
   m["Alice"]:
     find "Alice" ──hit──▶ return reference to existing .second
                 └──miss──▶ insert {"Alice", T()} ──▶ return .second
```

Convenient but **inserts** default values for missing keys. Use `at()` when the
key must already exist.

### 4.2 Insert pair

Identical BST + `insert_fixup` pipeline as `Set::insert`, but equality is on keys:

```
   (1) Descend comparing value.first vs current key.
   (2) Duplicate key → return {iterator, false} (value not overwritten).
   (3) New RED leaf with pair; fixup; root BLACK.
```

### 4.3 Rotations & fixups

Left/right rotation and insert/delete fixup cases are **the same** as in
`Set` — only the payload differs (`pair` vs single `T`). See
[`set/README.md`](../set/README.md) §4.2–4.4 for ASCII case diagrams.

### 4.4 Erase by key

BST delete with successor swap for two-child nodes; `delete_fixup` if removed
node was BLACK. Returns `1` if key existed, `0` otherwise.

### 4.5 Iterator increment

In-order successor on keys: right-subtree minimum, or climb from right child.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Map<K,T>()` | empty |
| `Map{{k,v}, ...}` | insert each pair |
| copy / move | deep clone / steal tree |

### Element access
| Call | Inserts if missing? | On missing key |
|---|---|---|
| `operator[](key)` | **Yes** (`T()`) | creates entry |
| `at(key)` | No | throws `out_of_range` |

### Modifiers
`insert(pair)`, `erase(key)`, `clear()`

### Lookup
`find(key)`, `contains(key)`, `count(key)` — count is 0 or 1

### Iterators
`begin()` / `end()` — dereference to `const pair<const Key,T>&` (or mutable
`.second` through non-const iterator in this implementation).

### Capacity
`empty()`, `size()`

---

## 6. Complexity Summary

| Operation | Complexity |
|---|---|
| `insert`, `erase`, `find`, `contains`, `operator[]` | O(log n) |
| iterator `++` / `--` | amortized O(1) |
| copy | O(n) |
| move, `swap` | O(1) |

Same bounds as `Set`; constant factors include pair storage per node.

---

## 7. Usage

```cpp
#include "map/map.hpp"

Map<std::string, int> ages;
ages["Alice"] = 30;
ages["Bob"] = 25;
ages.insert({"Charlie", 35});

std::cout << ages.at("Alice") << '\n';   // 30, throws if missing

ages.erase("Bob");

for (const auto& p : ages) {             // keys sorted: Alice, Charlie
    std::cout << p.first << ": " << p.second << '\n';
}
```

See [`map_example.cpp`](map_example.cpp) for `operator[]`, `at`, erase,
initializer lists, and ordered iteration.

---

## 8. Design Decisions & Trade-offs

- **Tree keyed on Key only.** Values float with the node; updating `m[k]` does
  not rebalance (key unchanged).
- **`operator[]` default-inserts.** Matches `std::map` ergonomics; easy to
  accidentally grow the map — prefer `insert` or `contains` + `at` when unsure.
- **const Key in pair.** Prevents silent re-ordering by mutating a key in place
  (would break BST invariant).
- **Shared RB implementation pattern with Set.** Pedagogical duplication vs a
  shared internal tree base — clarity wins here.

---

## 9. Common Pitfalls

- **`operator[]` creates entries.** `if (m[key])` inserts `key` even when you
  only meant to read.
- **`insert` does not update existing keys.** Second insert with same key leaves
  old value; use `m[key] = v` or erase then insert.
- **Iteration order is key order, not insertion order.**
- **Invalidated iterators after erase** of the pointed-to node (as in `std::map`).

---

## 10. Comparison with `std::map`

**Same:** unique sorted keys, `operator[]`/`at` semantics, RB-tree complexity,
bidirectional iterators over pairs.

**Omitted:** `emplace`, `try_emplace`, `insert_or_assign`, `lower_bound` /
`upper_bound`, allocators, node handles, `extract`/`merge`.

---

## 11. Build & Run

```bash
make run-map
make test-map
make all
```

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. map/map_example.cpp -o /tmp/x_map
/tmp/x_map
```

---

## 12. See Also

- [`set`](../set/README.md) — Red-Black tree without mapped values (full rotation/fixup diagrams)
- [`multimap`](../multimap/README.md) — duplicate keys (sorted-vector implementation)
- [`multiset`](../multiset/README.md) — duplicate elements
- [`vector`](../vector/README.md) — contiguous storage
