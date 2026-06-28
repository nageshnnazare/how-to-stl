# Deque — Chunked Double-Ended Queue

> A `Deque<T>` is a sequence you can grow at **both** ends in O(1) while still
> indexing element `i` in O(1). Memory is split into fixed-size chunks (16
> elements each) reached through a map of pointers — like a bookshelf of small
> arrays instead of one long shelf. Existing elements never move when you
> `push_front` or `push_back`; only new chunks are allocated.

This is a from-scratch reimplementation of `std::deque` built for learning. The
header is [`deque.hpp`](deque.hpp), runnable examples are in
[`deque_example.cpp`](deque_example.cpp), and the test suite is in
[`../tests/deque_test.cpp`](../tests/deque_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | Map of pointers → fixed-size chunks (16 `T` each) |
| Order | Insertion order (logical sequence across chunks) |
| Random access | **Yes**, O(1) via chunk index + offset |
| Grows | New chunks at front/back; map may double |
| Contiguous? | **No** — elements span multiple non-adjacent chunks |

**Reach for a Deque when** you need frequent `push_front` and `push_back` with
random access, or when you must avoid invalidating pointers to existing elements
during growth.

**Look elsewhere when** you only append at the back and want maximum cache
locality (see [`vector`](../vector/README.md)).

---

## 2. Mental Model

Think of a deque as a **map** (array of chunk pointers) pointing at fixed-size
**chunks** on the heap. Two cursors — `first_chunk_`/`first_index_` and
`last_chunk_`/`last_index_` — mark where the live elements begin and end.

```
   Deque object                         map_ (chunk pointer table)
   ┌──────────────────┐                ┌───┬───┬───┬───┬───┬───┬───┐
   │ map_      ●──────┼───────────────▶│ ∅ │ ∅ │ ● │ ● │ ∅ │ ∅ │ ∅ │
   │ first_chunk_ = 2 │                └───┴───┴─┬─┴─┬─┴───┴───┴───┘
   │ last_chunk_  = 3 │                        │   │
   │ first_index_ = 6 │               chunk 2   │   │  chunk 3
   │ last_index_  = 3 │                        ▼   ▼
   │ size_        = 5 │               ┌───┬───┬───┬───┬───┬─── ...
   └──────────────────┘               │ ? │ ? │ A │ B │ C │ D │ E │
                                      └───┴───┴───┴───┴───┴───┴───┘
                                        6   7   8   9  (chunk 2)
                                      chunk 3: D at [0], E at [1] if spanning...
```

Logical index `0` is `map_[first_chunk_][first_index_]`. Index `pos` is found by
chunk arithmetic (see §4.3).

---

## 3. Internal Representation

```cpp
static constexpr size_type CHUNK_SIZE = 16;

T**       map_;           // array of chunk pointers (nullptr = slot unused)
size_type map_size_;      // number of slots in map_
size_type first_chunk_;   // map index of front chunk
size_type last_chunk_;    // map index of back chunk
size_type first_index_;   // offset to front element within first chunk
size_type last_index_;    // offset one-past last element in last chunk
size_type size_;          // total element count
```

**Invariant:** `size_` equals the number of constructed elements spanning from
`(first_chunk_, first_index_)` through `(last_chunk_, last_index_ - 1)`.

### Chunk vs map

| Structure | Holds | Grows when |
|---|---|---|
| **Chunk** | Up to 16 `T` objects in contiguous raw storage | `push_*` fills current chunk |
| **Map** | `T*` pointers to chunks | No free map slots at front/back edge |

Elements inside an existing chunk **never relocate**. Only the map array is
reallocated and re-centered (chunk pointers copied, not the `T` objects).

---

## 4. How It Works (Step by Step)

### 4.1 Initialization (`init_map`)

An empty deque allocates a map with spare nullptr slots on both sides and one
chunk centered in the middle. Elements start at `first_index_ = CHUNK_SIZE / 2`
so there is room to grow in both directions within the first chunk.

```
   map_:  [∅][∅][●chunk][∅][∅]
               ▲
         first_chunk_ = last_chunk_ = center
         first_index_ = last_index_ = 8  (middle of 16-slot chunk)
```

### 4.2 `push_back` — append at the tail

```
   room in current chunk (last_index_ < 16):
       construct at map_[last_chunk_][last_index_++]

   chunk full (last_index_ == 16):
       ++last_chunk_; allocate new chunk if nullptr
       last_index_ = 0; construct; ++last_index_
```

```
   before:  chunk N: [e0][e1]...[e15]  (full)
   after:   chunk N+1: [new][ ][ ]...   last_index_ = 1
```

### 4.3 `push_front` — prepend at the head

Mirror of `push_back`. When `first_index_ == 0`, step to the previous map slot
(maybe allocate a chunk), set `first_index_ = CHUNK_SIZE`, then decrement and
construct.

```
   before:  first_index_=0 at chunk boundary
   after:   new chunk to the left: [X][?][?]...
            first_index_ points at X
```

### 4.4 Random access — `operator[](pos)`

```
   absolute = first_index_ + pos
   chunk    = first_chunk_ + absolute / CHUNK_SIZE
   index    = absolute % CHUNK_SIZE
   return map_[chunk][index];
```

Two integer divisions per access — constant time, but slower in practice than
vector's single pointer add due to chunk hopping and cache misses.

### 4.5 Map reallocation (`reallocate_map`)

When `first_chunk_ == 0` with no room (or symmetrically at the back), the map
doubles and existing chunk pointers copy to the **center** of the new array:

```
   old:  [∅][C0][C1][∅]           first_chunk_ = 1
   new:  [∅][∅][∅][C0][C1][∅][∅][∅] first_chunk_ = 3
```

Chunk memory is untouched — only pointer table moves.

### 4.6 `pop_front` / `pop_back`

Destroy the end element, advance `first_index_` or retreat `last_index_`. When
an index walks off the chunk edge, move to the adjacent chunk.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Deque<T>()` | empty deque, one centered chunk |
| `Deque<T>(n)` | `n` default-constructed elements |
| `Deque<T>(n, v)` | `n` copies of `v` |
| `Deque<T>{a, b, c}` | initializer list |
| copy / move ctor | deep copy / O(1) steal map + chunks |

### Element access
| Call | Bounds-checked? | Notes |
|---|---|---|
| `operator[](i)` | ❌ | O(1) chunk arithmetic |
| `at(i)` | ✅ | throws `std::out_of_range` |
| `front()` / `back()` | ❌ | direct chunk slot access |

### Capacity
`empty()`, `size()`, `max_size()`

### Modifiers
`push_front`, `push_back`, `emplace_front`, `emplace_back`, `pop_front`,
`pop_back`, `resize`, `clear`, `swap`

### Iterators
`begin/end`, `cbegin/cend` — random-access iterators (delegate to `operator[]`)

### Non-member
`==`, `!=`, `<`, `<=`, `>`, `>=` (lexicographic), and a free `swap`.

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `operator[]`, `at`, `front`, `back` | O(1) | two div/mod per `[]` |
| `push_front`, `push_back` | O(1) amortized | O(n) when map reallocates |
| `pop_front`, `pop_back` | O(1) | |
| `resize` (grow/shrink) | O(n) | repeated push/pop |
| `clear` | O(n) | destroys all elements |
| `swap` | O(1) | swaps internal pointers |
| copy | O(n) | move is O(1) |

---

## 7. Usage

```cpp
#include "deque/deque.hpp"

Deque<int> dq = {1, 2, 3};

dq.push_front(0);     // {0, 1, 2, 3}
dq.push_back(4);      // {0, 1, 2, 3, 4}

int x = dq[2];        // 2 — random access
int safe = dq.at(2);  // bounds-checked

dq.pop_front();
dq.pop_back();

for (int n : dq) std::cout << n << ' ';
```

See [`deque_example.cpp`](deque_example.cpp) for queue/stack patterns, sliding
windows, emplace, and alternating front/back growth.

---

## 8. Design Decisions & Trade-offs

- **CHUNK_SIZE = 16.** Small enough to limit waste in partially filled chunks;
  large enough to amortize map overhead. libstdc++ uses a platform-tuned size.
- **Centered initial chunk.** Starting at `CHUNK_SIZE/2` delays the first
  chunk-boundary allocation when growing in both directions.
- **Map doubling with re-centering.** Leaves headroom at both ends; avoids
  shifting all elements (there is no single block to shift).
- **No contiguity.** Iterators are random-access by logical index, but memory
  is not one slab — `&dq[0]` and `&dq[1]` may not be adjacent.
- **Stable element addresses.** Pointers/references to elements stay valid
  across `push_front`/`push_back` (unlike vector reallocation).

---

## 9. Common Pitfalls

- **Not contiguous — don't pass to C APIs expecting one block.** Use vector or
  copy to a contiguous buffer for `data()`-style interop.
- **Worse cache locality than vector.** Looping a large deque hits disjoint
  chunks; vector is faster for pure sequential scans.
- **`operator[]` has no bounds check.** Use `at()` on untrusted indices.
- **Chunks are not freed on `pop_*`.** `clear()` destroys elements but leaves
  chunk allocations in place (similar to vector's `clear` keeping capacity).
- **Iterator invalidation is limited but not absent.** This implementation does
  not insert in the middle; if you extended it, iterators could still invalidate
  on map reallocation in some designs.

---

## 10. Comparison with `std::deque`

**Same:** chunk/map architecture concept, O(1) both ends, O(1) random access,
no relocation of existing elements.

**Intentionally omitted for clarity:** custom allocators, `shrink_to_fit`,
middle `insert`/`erase`, reverse iterators.

**Differs:** fixed `CHUNK_SIZE = 16`; simplified map growth; iterators store a
logical index rather than a `(chunk, offset)` pair.

---

## 11. Build & Run

```bash
make run-deque       # build + run the examples
make test-deque      # build + run the unit tests
make all             # build everything in the repo
```

---

## 12. See Also

- [`vector`](../vector/README.md) — contiguous dynamic array, O(1) back only
- [`list`](../list/README.md) — linked nodes, O(1) splice, no random access
- [`array`](../array/README.md) — fixed-size stack array
- [`string`](../string/README.md) — specialized char sequence with SSO
