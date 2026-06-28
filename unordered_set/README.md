# UnorderedSet — Hash Table with Separate Chaining

> An `UnorderedSet<T>` stores **unique** values with **no ordering guarantee**.
> You ask "is `x` in the set?" and get an answer in **O(1) average** time by
> hashing `x` to a bucket index and walking a short linked list of collisions.
> When too many elements pile into too few buckets, the table **doubles** and
> every entry is **re-hashed** into the larger array.

This is a from-scratch reimplementation of `std::unordered_set` built for learning. The
header is [`unordered_set.hpp`](unordered_set.hpp), runnable examples are in
[`unordered_set_example.cpp`](unordered_set_example.cpp), and the test suite is in
[`../tests/unordered_set_test.cpp`](../tests/unordered_set_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | `vector<list<T>>` — bucket array of collision chains |
| Order | **None** (iteration order is arbitrary and unstable) |
| Uniqueness | **Yes** — duplicate inserts are rejected |
| Average lookup | **O(1)** by hash + short chain scan |
| Object size | `vector` + `size_t` + `Hash` functor |

**Reach for an UnorderedSet when** you need fast membership tests (`contains`) on a
large collection and do not care about sorted order or range traversal by key order.

**Look elsewhere when** you need elements sorted by value (see [`set`](../set/README.md))
or key-sorted maps (see [`map`](../map/README.md)), or predictable iteration order.

---

## 2. Mental Model

Picture a row of mail slots (buckets). Each slot either holds nothing or the head of a
linked list of values that hashed to the same slot:

```
   UnorderedSet                         buckets_ (8 slots)
   ┌──────────────┐                    ┌───┬───┬───┬───┬───┬───┬───┬───┐
   │ buckets_  ●──┼───────────────────▶│   │42─│   │   │   │   │7──│   │
   │ size_ = 4  │                    │   │99 │   │   │   │   │31 │   │
   │ hash_      │                    └───┴─┬─┴───┴───┴───┴───┴─┬─┴───┘
   └──────────────┘                        │                     │
                                      chain at 1              chain at 6
```

- **Hash** maps a value to a large integer; **modulo** maps that to `[0, bucket_count)`.
- **Collisions** (two values, same bucket) share a **chain** — neither overwrites the other.
- **Load factor** `= size / buckets`. When it exceeds **2.0**, we **rehash** (double buckets).

---

## 3. Internal Representation

```cpp
std::vector<std::list<T>> buckets_;  // bucket array; each list is one chain
size_t                    size_;     // element count (unique values)
Hash                      hash_;     // std::hash<T> by default
static constexpr size_t DEFAULT_BUCKETS = 16;
```

**Invariant:** `size_` equals the total number of nodes across all bucket lists.

### Bucket index

```cpp
idx = hash_(value) % buckets_.size();
```

### The three primitive operations on chains

| Step | What | Where in `unordered_set.hpp` |
|---|---|---|
| Locate bucket | `hash % bucket_count` | `bucket_index` |
| Append to chain | `list::push_back` | `insert` |
| Scan chain | linear walk with `==` | `contains`, `erase` |

---

## 4. How It Works (Step by Step)

### 4.1 Insert — hash, maybe rehash, push on chain

```
   insert("apple"):

   (1) contains?  no
   (2) load OK?   size=20, buckets=16 → load 1.25 → skip rehash
   (3) idx = hash("apple") % 16  →  say bucket 7
   (4) buckets_[7]:  [pear]  →  push_back  →  [pear]──▶[apple]
       ++size_
```

Duplicates short-circuit at step (1): `insert` returns `false` and `size_` is unchanged.

### 4.2 Lookup (`contains`) — same bucket, scan the chain

```
   contains("apple"):

   idx = hash("apple") % 16  →  7
   walk buckets_[7]:  pear? no  →  apple? yes  →  return true
```

Worst case: every value collides in bucket 0 → O(n) chain walk. Average: O(1) with a good hash and load factor.

### 4.3 Erase — find node in chain, unlink it

```
   erase(99) in bucket 1:

   [42]──▶[99]──▶[17]
              │
         erase iterator
              ▼
   [42]──▶[17]        size_--
```

### 4.4 Rehash — double buckets, re-hash every element

Triggered when `size_ > buckets_.size() * 2` (load factor **> 2.0**).

```
   BEFORE  (4 buckets, size_=9)
   b[0]: a,b        b[1]: c        b[2]: (empty)    b[3]: d,e,f,g,h,i

   AFTER   (8 buckets — each value re-inserted with % 8)
   b[0]: a          b[1]: b,c      b[2]: (empty)    b[3]: d
   b[4]: e          b[5]: f        b[6]: g          b[7]: h,i
```

**Why re-hash everything?** The modulus changed. Value `x` that lived in bucket `hash(x)%4`
may belong in bucket `hash(x)%8` after growth. Chains cannot simply be split in place.

### 4.5 `clear` vs destruction

| Call | Clears chains? | Frees bucket vector? | `size_` after |
|---|---|---|---|
| `clear()` | ✅ | ❌ (keeps 16+ buckets) | 0 |
| destructor | ✅ | ✅ (RAII on members) | — |

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `UnorderedSet<T>()` | 16 empty buckets, `size_ = 0` |
| `UnorderedSet<T>{a, b, c}` | insert each; duplicates skipped |

### Capacity
`empty()`, `size()`

### Modifiers
| Call | Returns | Notes |
|---|---|---|
| `insert(value)` | `bool` | `true` if new, `false` if duplicate |
| `erase(value)` | `0` or `1` | removes one matching element |
| `clear()` | — | all chains emptied |

### Lookup
| Call | Effect |
|---|---|
| `contains(value)` | `bool` membership test |
| `count(value)` | `0` or `1` (set semantics) |

---

## 6. Complexity Summary

| Operation | Average | Worst case | Note |
|---|---|---|---|
| `insert` | O(1) | O(n) | worst = one long chain |
| `contains` | O(1) | O(n) | same chain walk as insert |
| `erase` | O(1) | O(n) | find + list erase |
| `clear` | O(n) | O(n) | destroy every node |
| rehash | O(n) | O(n) | rare; doubles buckets |

---

## 7. Usage

```cpp
#include "unordered_set/unordered_set.hpp"

UnorderedSet<int> us = {3, 1, 4, 1, 5};  // duplicates dropped → size 4

us.insert(9);                  // true
us.insert(5);                  // false — already there

if (us.contains(4)) { /* ... */ }

us.erase(3);
std::cout << us.size();        // 4

UnorderedSet<std::string> words = {"apple", "banana"};
words.insert("cherry");
```

See [`unordered_set_example.cpp`](unordered_set_example.cpp) for initializer lists,
string keys, insert/erase/count, and duplicate handling.

---

## 8. Design Decisions & Trade-offs

- **Separate chaining** (lists per bucket) instead of open addressing: simpler to
  teach erase and no tombstone bookkeeping; uses extra pointers per element.
- **Load factor threshold 2.0** (`size > 2 × buckets`): aggressive chaining allowed
  before growth; doubles bucket count when triggered (same spirit as geometric growth).
- **`std::hash` + modulo**: production tables often apply extra mixing (e.g. Fibonacci
  hashing); we keep the textbook `hash % N` for clarity.
- **No iterators** in this teaching build: reduces scope; `std` provides full iteration.
- **Duplicate check before insert**: two lookups on insert path in exchange for clear
  set semantics and correct `size_`.

---

## 9. Common Pitfalls

- **Assuming any iteration order.** `{3,1,4}` will not print sorted; order changes on rehash.
- **Bad hash for your key type.** Custom types need a good `Hash` and consistent `operator==`.
- **Confusing `count` with `multiset`.** Here `count` is always 0 or 1.
- **Performance cliffs under attack.** Adversarial inputs can force all keys into one bucket → O(n).
  Production libs use implementation-defined rehash strategies and hash hardening.

---

## 10. Comparison with `std::unordered_set`

**Same:** hash table + chaining mental model, average O(1) insert/find/erase, unique keys,
`count` returns 0 or 1.

**Intentionally omitted for clarity:** iterators, `bucket_count` / `load_factor` accessors,
`reserve` / `rehash(n)`, node handles, allocator support, exception guarantees on rehash.

---

## 11. Build & Run

```bash
make run-associative   # runs set, map, multiset, multimap, unordered_set, unordered_map examples
make test-associative  # unit tests for associative containers

# Or compile this module alone from the repo root:
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. unordered_set/unordered_set_example.cpp -o /tmp/x_unordered_set
/tmp/x_unordered_set
```

---

## 12. See Also

- [`unordered_map`](../unordered_map/README.md) — same table, stores `key → value` pairs in chains
- [`set`](../set/README.md) — ordered unique tree, O(log n) operations
- [`map`](../map/README.md) — ordered key–value tree
- [`hash`](../hash/README.md) — how `std::hash` is supposed to behave (conceptual)
