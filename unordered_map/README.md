# UnorderedMap — Hash Table of Key–Value Pairs

> An `UnorderedMap<Key, T>` maps **unique keys** to **values** with **no key ordering**.
> Look up a key in **O(1) average** by hashing the key to a bucket, then scanning a
> short linked list of `(key, value)` pairs. `operator[]` quietly inserts a default
> value when the key is missing; `at()` throws instead.

This is a from-scratch reimplementation of `std::unordered_map` built for learning. The
header is [`unordered_map.hpp`](unordered_map.hpp), runnable examples are in
[`unordered_map_example.cpp`](unordered_map_example.cpp), and the test suite is in
[`../tests/unordered_map_test.cpp`](../tests/unordered_map_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | `vector<list<pair<Key,T>>>` — chained buckets of pairs |
| Key order | **None** |
| Key uniqueness | **Yes** |
| Average lookup | **O(1)** by key hash + chain scan |
| Default access | `operator[]` inserts `{key, T()}` if missing |

**Reach for an UnorderedMap when** you need fast lookups/updates by key (counts, caches,
indexes) and do not need keys sorted.

**Look elsewhere when** you need keys in sorted order ([`map`](../map/README.md)) or
only a set of keys ([`unordered_set`](../unordered_set/README.md)).

---

## 2. Mental Model

Same bucket array as `UnorderedSet`, but each chain node stores **both** key and value:

```
   UnorderedMap<string,int>              buckets_
   ┌─────────────────┐                  ┌───┬───────────────┬───┐
   │ buckets_     ●──┼─────────────────▶│   │("bob",25)──┐  │   │
   │ size_ = 2       │                  │   │             ▼  │   │
   └─────────────────┘                  └───┘        ("eve",31) └───┘
                                              chain in bucket 2
```

- Only the **key** is hashed: `idx = hash(key) % bucket_count`.
- **Collisions**: distinct keys with the same bucket index share one chain.
- **Load factor** `size / buckets`; rehash when **> 2.0**.

---

## 3. Internal Representation

```cpp
using value_type_internal = std::pair<Key, T>;
std::vector<std::list<value_type_internal>> buckets_;
size_t size_;
Hash hash_;  // std::hash<Key> by default
static constexpr size_t DEFAULT_BUCKETS = 16;
```

**Invariant:** each key appears in at most one chain node; `size_` is the node count.

### Chain node layout

```
   list node:  ┌─────────────┐
               │ first: Key  │  ← compared during lookup
               │ second: T   │  ← returned by operator[] / at
               └─────────────┘
```

---

## 4. How It Works (Step by Step)

### 4.1 `operator[]` — find or default-insert

```
   ages["Alice"] = 30   (first time):

   hash("Alice") % 16 → bucket 5
   chain empty → rehash? → push_back {"Alice", 0} → return .second → assign 30

   ages["Alice"]        (second time):

   bucket 5: ("Alice",30) found → return reference to 30 (no size change)
```

### 4.2 `at` — find or throw

Same chain walk as `operator[]`, but **no insert** on miss → `std::out_of_range`.

### 4.3 `insert(pair)` — fail if key exists

```
   insert({"Bob", 25}):
   contains("Bob")?  no → push on chain, size_++
   insert({"Bob", 99}):
   contains("Bob")?  yes → return false (value 25 kept)
```

### 4.4 `erase(key)` — unlink matching pair node

```
   bucket 2:  ("bob",25)──▶("eve",31)
   erase("bob"):
   ("eve",31)          size_--
```

### 4.5 Rehash — double buckets, re-hash by key

```
   load = size / buckets > 2.0  →  new_buckets has 2× slots

   for every (k,v) in old table:
       new_buckets[ hash(k) % new_size ].push_back({k,v})
```

Pairs move to new buckets based on **key** hash only; values move with their keys.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `UnorderedMap<K,T>()` | 16 empty buckets |
| `UnorderedMap{{k1,v1},{k2,v2}}` | insert each pair |

### Element access
| Call | On missing key |
|---|---|
| `operator[](key)` | inserts `T()`, returns reference |
| `at(key)` | throws `std::out_of_range` |

### Capacity
`empty()`, `size()`

### Modifiers
| Call | Returns | Notes |
|---|---|---|
| `insert({k,v})` | `bool` | `false` if key exists |
| `erase(key)` | `0` or `1` | |
| `clear()` | — | |

### Lookup
`contains(key)`, `count(key)` → 0 or 1

---

## 6. Complexity Summary

| Operation | Average | Worst | Note |
|---|---|---|---|
| `operator[]` (hit) | O(1) | O(n) | chain scan |
| `operator[]` (miss) | O(1) amortized | O(n) | may rehash |
| `at` | O(1) | O(n) | no insert |
| `insert` | O(1) amortized | O(n) | |
| `erase` | O(1) | O(n) | |
| rehash | O(n) | O(n) | doubles buckets |

---

## 7. Usage

```cpp
#include "unordered_map/unordered_map.hpp"

UnorderedMap<std::string, int> ages;
ages["Alice"] = 30;
ages["Bob"] = 25;

int a = ages.at("Alice");           // 30
bool ok = ages.contains("Bob");     // true

ages.erase("Bob");

UnorderedMap<int, std::string> items = {{1, "one"}, {2, "two"}};
std::cout << items[2];              // "two"
```

See [`unordered_map_example.cpp`](unordered_map_example.cpp) for `operator[]`, `at`,
insert, erase, and initializer lists.

---

## 8. Design Decisions & Trade-offs

- **Chaining with `list<pair>`** — teaches collision lists clearly; pointer overhead per entry.
- **`operator[]` default-insert** — matches STL ergonomics; easy to accidentally grow the map
  with typos (`map["Alcie"]`).
- **`insert` does not overwrite** — explicit choice; use `operator[]` or erase-then-insert to update.
- **Rehash at load > 2.0** — same policy as sibling `UnorderedSet` in this repo.
- **No iterators** — keeps the teaching surface small.

---

## 9. Common Pitfalls

- **`operator[]` always inserts.** `map["missing"]` creates an entry. Use `contains` or `at` if you only want to read.
- **No stable iteration order** — do not rely on chain order for determinism across runs/rehashes.
- **Non-const keys in internal pair** — public `value_type` uses `const Key` (STL style); internal storage uses mutable `Key` in nodes for simplicity.
- **Floating-point keys** — avoid unless you enjoy pain; equality and hash are treacherous.

---

## 10. Comparison with `std::unordered_map`

**Same:** hash + chaining model, `operator[]`/`at`/`insert`/`erase` semantics, average O(1).

**Omitted:** iterators, `try_emplace`, `insert_or_assign`, node API, allocators, `max_load_factor`.

---

## 11. Build & Run

```bash
make run-associative
make test-associative

g++ -std=c++14 -Wall -Wextra -Wpedantic -I. unordered_map/unordered_map_example.cpp -o /tmp/x_unordered_map
/tmp/x_unordered_map
```

---

## 12. See Also

- [`unordered_set`](../unordered_set/README.md) — keys only, no mapped values
- [`map`](../map/README.md) — ordered red-black tree map
- [`multimap`](../multimap/README.md) — multiple values per key (ordered)
