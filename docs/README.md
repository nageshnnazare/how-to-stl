# Cross-Cutting Concepts

This folder holds the **repository-wide** documentation: the ideas that show up
again and again across every container. Each individual container has its own
deep-dive README (e.g. [`../vector/README.md`](../vector/README.md)); this guide
covers the shared foundations so the per-container docs don't have to repeat them.

- [1. The shared memory pattern](#1-the-shared-memory-pattern)
- [2. RAII and the rule of five](#2-raii-and-the-rule-of-five)
- [3. Which container should I use?](#3-which-container-should-i-use)
- [4. Complexity matrix (all containers)](#4-complexity-matrix-all-containers)
- [5. Iterator invalidation cheat sheet](#5-iterator-invalidation-cheat-sheet)
- [6. Suggested learning path](#6-suggested-learning-path)

---

## 1. The shared memory pattern

Almost every dynamic container in this repo separates **memory** from **objects**.
The C++ standard library does the same. There are four primitive steps, and you
will see them everywhere (`vector`, `string`, `deque`, `unordered_map`, ...):

```
   ┌──────────────────────┐   ::operator new(bytes)         no constructors run
   │ 1. ALLOCATE raw      │ ───────────────────────────▶   [ ? ][ ? ][ ? ]
   │     bytes            │                                  (uninitialized)
   └──────────────────────┘

   ┌──────────────────────┐   new (slot) T(args...)          one object appears
   │ 2. CONSTRUCT in place│ ───────────────────────────▶   [ A ][ ? ][ ? ]
   │     (placement new)  │                                  (placement new)
   └──────────────────────┘

   ┌──────────────────────┐   slot->~T()                     object gone,
   │ 3. DESTROY object    │ ───────────────────────────▶   bytes remain
   │     (explicit dtor)  │                                  [ ? ][ ? ][ ? ]
   └──────────────────────┘

   ┌──────────────────────┐   ::operator delete(ptr)          bytes returned to OS
   │ 4. FREE raw bytes    │ ───────────────────────────▶
   └──────────────────────┘
```

**Why not just `new T[n]`?** Because `new T[n]` fuses steps 1 and 2: it
default-constructs *every* element immediately. Containers need to *reserve*
space (step 1) long before they have values to put there (step 2). Keeping the
steps separate is exactly what lets `vector::reserve(1000)` allocate room for a
thousand elements while `size()` stays `0`.

This is the single most important idea to internalise before reading the
sequence and hash-based containers.

---

## 2. RAII and the rule of five

Every owning type here follows **RAII**: a resource is acquired in a constructor
and released in the destructor, so cleanup is automatic and exception-safe.

Because these types own raw resources (heap blocks, file handles, ref-counts),
they implement the **rule of five**:

| Special member | Job |
|---|---|
| Destructor | release the resource |
| Copy constructor | **deep** copy the resource |
| Copy assignment | release old, deep-copy new (often via copy-and-swap) |
| Move constructor | **steal** the resource, null out the source (O(1)) |
| Move assignment | release old, steal new, null out source |

The recurring **copy-and-swap** idiom appears in `vector`, `string`, and others:

```cpp
Thing& operator=(const Thing& other) {
    Thing tmp(other);   // do all the throwing work on a copy
    swap(tmp);          // cheap, no-throw pointer swap
    return *this;       // tmp's destructor frees our old data
}
```

It gives the **strong exception guarantee** (if the copy throws, `*this` is
untouched) and handles self-assignment for free.

---

## 3. Which container should I use?

```
            need key → value lookup?
            ┌──────────── yes ───────────────┐
            no                                 │
            │                          need sorted keys / ranges?
   need fast lookup by VALUE?          ┌── yes ──┐   └── no ──┐
   ┌── yes ──┐      └── no ──┐         map         unordered_map
   │          │              │       (RB-tree)     (hash table)
 sorted?   sequence access pattern?
 ┌yes┐┌no┐  ┌──────────┬───────────┬────────────┐
 set  unordered_set   push/pop    insert/erase   index-heavy,
 (RB) (hash)          at ends?    in middle?     append-heavy?
                      │            │              │
                   deque/         list           vector
                   stack/queue    (linked)       (contiguous)  ← default
```

Quick rules of thumb:

- **`vector`** is the default. Contiguous memory beats clever data structures
  far more often than intuition suggests.
- Need **both ends**? `deque` (or `stack`/`queue` adapters on top of it).
- Lots of **middle** insert/erase with stable references? `list`.
- **Lookup by value/key**: hashed (`unordered_*`, average O(1)) unless you need
  **ordering / range queries**, then tree-based (`set`/`map`, O(log n)).
- **"Always give me the largest/smallest next"**: `priority_queue` (binary heap).
- **Fixed size known at compile time**: `array` (zero heap, zero overhead).

---

## 4. Complexity matrix (all containers)

Average-case time complexity. `n` = number of elements.

| Container | access | search | insert | erase | push/pop ends | ordered? |
|---|---|---|---|---|---|---|
| [`array`](../array/README.md) | O(1) | O(n) | — | — | — | no |
| [`vector`](../vector/README.md) | O(1) | O(n) | O(n)¹ | O(n) | back: O(1)² | no |
| [`deque`](../deque/README.md) | O(1) | O(n) | O(n) | O(n) | both: O(1) | no |
| [`list`](../list/README.md) | O(n) | O(n) | O(1)³ | O(1)³ | both: O(1) | no |
| [`set`](../set/README.md) / [`map`](../map/README.md) | — | O(log n) | O(log n) | O(log n) | — | **yes** |
| [`multiset`](../multiset/README.md) / [`multimap`](../multimap/README.md) | — | O(log n) | O(n)⁴ | O(n)⁴ | — | **yes** |
| [`unordered_set`](../unordered_set/README.md) / [`unordered_map`](../unordered_map/README.md) | — | O(1) | O(1) | O(1) | — | no |
| [`stack`](../stack/README.md) / [`queue`](../queue/README.md) | top/front O(1) | — | — | — | O(1) | no |
| [`priority_queue`](../priority_queue/README.md) | top: O(1) | — | O(log n) | top: O(log n) | — | partial |

¹ middle insert; back insert is amortized O(1). ² amortized; O(n) on reallocation.
³ given an iterator to the position. ⁴ sorted-vector implementation: O(log n) to
find, O(n) to shift.

---

## 5. Iterator invalidation cheat sheet

The number-one source of subtle bugs. "Invalidated" = the iterator/pointer/
reference may now dangle.

| Container | What invalidates iterators |
|---|---|
| `vector` | any reallocation (`push_back`, `insert`, `reserve`, `resize`); `erase` invalidates at/after the point |
| `deque` | `push_*`/`pop_*` invalidate iterators; references survive end operations |
| `list` | only the erased element's iterator; everything else is stable |
| `set`/`map` (RB-tree) | only the erased node; all others stable |
| `unordered_*` | a **rehash** (triggered by growth past the load factor) invalidates all iterators; references to elements survive |

---

## 6. Suggested learning path

1. **Read this file** — the memory pattern in §1 underpins everything.
2. [`array`](../array/README.md) → [`vector`](../vector/README.md) — start with contiguous storage.
3. [`string`](../string/README.md) — vector-like, plus Small String Optimization.
4. [`list`](../list/README.md) → [`deque`](../deque/README.md) — node-based and chunk-based layouts.
5. [`unique_ptr`](../unique_ptr/README.md) → [`shared_ptr`](../shared_ptr/README.md) — ownership and RAII in their purest form.
6. [`set`/`map`](../set/README.md) — Red-Black trees (the hardest, most rewarding).
7. [`unordered_set`/`unordered_map`](../unordered_set/README.md) — hashing and rehashing.
8. [`priority_queue`](../priority_queue/README.md) → [`stack`/`queue`](../stack/README.md) — heaps and adapters.
9. [`pair`](../pair/README.md), [`tuple`](../tuple/README.md), [`optional`](../optional/README.md), [`bitset`](../bitset/README.md) — utility types.
10. [`allocator`](../allocator/README.md), [`arena_allocator`](../arena_allocator/README.md), [`locks`](../locks/README.md), [`thread_pool`](../thread_pool/README.md) — systems-level building blocks.

---

← Back to the [project README](../README.md).
