# Vector — Contiguous Dynamic Array

> A `Vector<T>` is a resizable array. Elements live side-by-side in one block of
> heap memory, so reading element `i` is a single pointer addition, and looping
> over the whole thing is as cache-friendly as a raw C array. When it runs out
> of room it quietly grabs a bigger block, moves everything across, and frees
> the old one — all you ever call is `push_back`.

This is a from-scratch reimplementation of `std::vector` built for learning. The
header is [`vector.hpp`](vector.hpp), runnable examples are in
[`vector_example.cpp`](vector_example.cpp), and the test suite is in
[`../tests/vector_test.cpp`](../tests/vector_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | One contiguous heap block |
| Order | Insertion order (you control it) |
| Random access | **Yes**, O(1) by index |
| Grows | Automatically, by doubling |
| Object size | 3 pointers' worth (`data_`, `size_`, `capacity_`) — 24 bytes on 64-bit |

**Reach for a Vector when** you want a list of things you mostly append to and
index into. It is the default container in C++ for a reason: contiguous memory
is the fastest thing a CPU can chew through.

**Look elsewhere when** you frequently insert/erase in the *middle* of a large
sequence (see [`list`](../list/README.md)) or push/pop at the *front*
(see [`deque`](../deque/README.md)).

---

## 2. Mental Model

A Vector is three small numbers on the stack pointing at a block on the heap:

```
   Vector object (stack)                Heap block: capacity_ slots
   ┌───────────────────┐               ┌─────┬─────┬─────┬─────┬─────┐
   │ data_      ●──────┼──────────────▶│  A  │  B  │  C  │  ?  │  ?  │
   │ size_      = 3    │               └─────┴─────┴─────┴─────┴─────┘
   │ capacity_  = 5    │                 0     1     2     3     4
   └───────────────────┘               └─ size_ = 3 live ─┘ └ raw ┘
```

- Slots `[0, size_)` hold **live objects**.
- Slots `[size_, capacity_)` are **raw, uninitialized bytes** — reserved but not
  yet real `T` objects. The whole trick of `vector` is keeping these two regions
  straight.

---

## 3. Internal Representation

```cpp
T*        data_;      // pointer to the block (nullptr when capacity_ == 0)
size_type size_;      // count of constructed elements
size_type capacity_;  // count of slots the block can hold
```

**Invariant:** `0 <= size_ <= capacity_`, and `data_ != nullptr` whenever
`capacity_ > 0`.

### The four primitive operations

`std::vector` never uses `new T[n]` (that would construct every slot eagerly).
Instead it splits *allocation* from *construction*:

| Step | What | Code in `vector.hpp` |
|---|---|---|
| Allocate raw bytes | `::operator new(n * sizeof(T))` | `reserve`, constructors |
| Construct one element in a raw slot | placement `new (p) T(...)` | `construct_at` |
| Destroy one element (storage stays) | `p->~T()` | `destroy_range` |
| Free the raw bytes | `::operator delete(p)` | destructor, `reserve` |

Keeping these separate is what lets `reserve(1000)` allocate space for 1000
elements while `size()` stays `0`.

---

## 4. How It Works (Step by Step)

### 4.1 Geometric growth — why `push_back` is amortized O(1)

When `size_ == capacity_`, `push_back` must regrow. We **double**:

```
   capacity:  0 → 1 → 2 → 4 → 8 → 16 → 32 → ...
```

Why doubling and not "+1"? Consider appending N elements:

- **Grow by +1 each time:** copy 1 + 2 + 3 + ... + N ≈ N²/2 work → **O(N²)** total. Terrible.
- **Double each time:** copy 1 + 2 + 4 + ... + N < 2N work → **O(N)** total → **O(1) amortized** per push.

A single `push_back` that triggers a reallocation is genuinely O(N), but those
expensive pushes are rare enough (only at 1, 2, 4, 8, ...) that the *average*
cost is constant.

### 4.2 Reallocation, slot by slot (`reserve`)

This is the only place the block ever moves. It also **invalidates every
pointer, reference, and iterator** into the vector, because the elements
physically change address.

```
   START   data_ ─▶ [A][B][C]                 size_=3, capacity_=3 (full)

   (1) allocate a bigger RAW block
           new_data ─▶ [ ][ ][ ][ ][ ][ ]     capacity 6, nothing constructed

   (2) MOVE each live element across (placement new)
           new_data ─▶ [A][B][C][ ][ ][ ]     for Vector<string>, this steals
                                              each string's buffer, no char copy

   (3) destroy the moved-from originals       [x][x][x]  (husks → ~T())
   (4) ::operator delete the old block, repoint data_

   END     data_ ─▶ [A][B][C][ ][ ][ ]        size_=3, capacity_=6
```

### 4.3 Inserting in the middle (`insert`)

O(n): the tail slides right to open a one-slot gap. We walk **back-to-front** so
we never clobber a value we haven't moved yet.

```
   insert X before index 1 of [A][B][C]   (one spare capacity slot shown as ·)

       index:  0   1   2   3
              [A][B][C][·]

   shift tail right (i = size_ down to index+1):
              [A][ ][B][C]      B,C moved one slot right
   drop X into the gap:
              [A][X][B][C]
```

The rightmost destination (the old `·`) is **raw**, so it is filled with
`construct_at` (placement new); every other destination already holds a live
object, so it is **move-assigned**. Getting this distinction wrong is a classic
source of double-construction / use-of-uninitialized bugs.

### 4.4 Erasing (`erase`)

The mirror image: slide the tail **left** to close the gap, then destroy the now
-duplicate last element.

```
   erase index 1 of [A][B][C][D]:
              [A][C][D][D]      shift C,D left over B
              [A][C][D]         destroy the leftover trailing D, size_--
```

### 4.5 `clear` vs the destructor vs `shrink_to_fit`

| Call | Destroys elements? | Frees the block? | `capacity()` after |
|---|---|---|---|
| `clear()` | ✅ | ❌ | unchanged |
| `shrink_to_fit()` | ❌ (relocates) | ✅ old block | `== size()` |
| destructor | ✅ | ✅ | — |

`clear()` keeps the capacity so you can refill without reallocating.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Vector<T>()` | empty, no allocation |
| `Vector<T>(n)` | `n` default-constructed elements |
| `Vector<T>(n, v)` | `n` copies of `v` |
| `Vector<T>{a, b, c}` | from an initializer list |
| copy / move ctor | deep copy / O(1) steal |

### Element access
| Call | Bounds-checked? | Notes |
|---|---|---|
| `operator[](i)` | ❌ | fastest; UB if out of range |
| `at(i)` | ✅ | throws `std::out_of_range` |
| `front()` / `back()` | ❌ | first / last element |
| `data()` | — | raw `T*` for C interop |

### Capacity
`empty()`, `size()`, `capacity()`, `reserve(n)`, `shrink_to_fit()`, `max_size()`

### Modifiers
`push_back`, `emplace_back`, `pop_back`, `insert`, `erase`, `resize`, `clear`, `swap`

### Iterators
`begin/end`, `cbegin/cend` (iterators are plain `T*`, so all pointer arithmetic works)

### Non-member
`==`, `!=`, `<`, `<=`, `>`, `>=` (lexicographic), and a free `swap`.

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `operator[]`, `at`, `front`, `back` | O(1) | |
| `push_back`, `emplace_back` | O(1) amortized | O(n) on the reallocation |
| `pop_back` | O(1) | |
| `insert` / `erase` at position `i` | O(n − i) | tail shift |
| `reserve`, `shrink_to_fit`, `resize` (grow) | O(n) | relocation |
| `clear` | O(n) | runs `n` destructors |
| `swap` | O(1) | swaps 3 fields |
| copy | O(n) | move is O(1) |

---

## 7. Usage

```cpp
#include "vector/vector.hpp"

Vector<int> v = {1, 2, 3};

v.reserve(100);        // one allocation up front, no further regrows
v.push_back(4);        // O(1)
v.emplace_back(5);     // constructs in place

int first = v[0];      // fast, unchecked
int safe  = v.at(2);   // checked, may throw

v.insert(v.begin() + 1, 99);  // {1, 99, 2, 3, 4, 5}
v.erase(v.begin());           // {99, 2, 3, 4, 5}

for (int x : v) std::cout << x << ' ';
```

See [`vector_example.cpp`](vector_example.cpp) for the full, runnable tour
(growth visualization, move semantics, 2-D grids, and more).

---

## 8. Design Decisions & Trade-offs

- **Double on growth.** Simple and gives amortized O(1). (libstdc++ also doubles;
  MSVC grows by 1.5×, which wastes less memory but copies a bit more often.)
- **Move, don't copy, when relocating.** A reallocation move-constructs each
  element, so growing a `Vector<std::string>` doesn't deep-copy any characters.
- **Iterators are raw pointers.** Zero overhead and every STL algorithm works,
  at the cost of no debug-mode invalidation checks.
- **Strong exception guarantee** on growth: if an element's move/copy throws, the
  original vector is untouched (we build the new block before discarding the old).

---

## 9. Common Pitfalls

- **Dangling iterators after growth.** Any `push_back`/`insert`/`reserve` that
  reallocates invalidates *all* existing iterators, pointers, and references.
  ```cpp
  int& r = v[0];
  v.push_back(x);   // may reallocate...
  r = 5;            // ☠ r may now dangle
  ```
- **`operator[]` has no bounds check.** Use `at()` on untrusted indices.
- **`size()` vs `capacity()`.** `reserve(n)` changes capacity, not size; the new
  slots are not elements yet.
- **Resizing in a hot loop.** Call `reserve()` once if you know the final count.

---

## 10. Comparison with `std::vector`

**Same:** layout, complexity, growth strategy, exception guarantees, most of the API.

**Intentionally omitted for clarity:** custom allocators, reverse iterators,
range `insert`/`assign`, and `emplace` (only `emplace_back` is provided).

---

## 11. Build & Run

```bash
make run-vector     # build + run the examples
make test-vector    # build + run the unit tests
make all            # build everything in the repo
```

---

## 12. See Also

- [`array`](../array/README.md) — fixed-size, stack-allocated sibling
- [`deque`](../deque/README.md) — O(1) at *both* ends
- [`list`](../list/README.md) — O(1) middle insert, no contiguity
- [`allocator`](../allocator/README.md) — what lives under `::operator new`
