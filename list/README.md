# List — Doubly-Linked Sequence

> A `List<T>` stores each element in its own heap node, wired to its neighbors
> with `prev` and `next` pointers — like a train where every car knows the car
> in front and behind. You can add or remove at the ends in O(1) without moving
> other elements, but finding element `i` by index requires walking the chain.

This is a from-scratch reimplementation of the core ideas behind `std::list`
built for learning. The header is [`list.hpp`](list.hpp), runnable examples are
in [`list_example.cpp`](list_example.cpp), and the test suite is in
[`../tests/list_test.cpp`](../tests/list_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | Scattered heap `Node`s linked by pointers |
| Order | Insertion order along the chain |
| Random access | **No** — no `operator[]` |
| Grows | One `new Node` per `push_*` |
| Object size | 3 words (`head_`, `tail_`, `size_`) on 64-bit |

**Reach for a List when** you need stable element addresses, frequent
insert/erase at positions you already have iterators to, or O(1) work at both
ends without shifting (once extended with `insert`/`erase`).

**Look elsewhere when** you mostly index or scan sequentially (see
[`vector`](../vector/README.md)) — contiguous memory wins on cache.

---

## 2. Mental Model

A list is a chain of nodes. The `List` object only holds pointers to the first
and last node plus a count:

```
   List object                    Node chain on the heap
   ┌─────────────────┐
   │ head_  ●────────┼──▶ ┌─────────┐    ┌─────────┐    ┌─────────┐
   │ tail_  ●────────┼──┐ │prev: ∅  │◀──▶│prev/next│◀──▶│next: ∅  │
   │ size_  = 3      │  └─│data: A  │    │data: B  │    │data: C  │
   └─────────────────┘    └─────────┘    └─────────┘    └─────────┘
   end() == iterator(nullptr)  —  NOT a sentinel node
```

- Each `Node` holds `T data`, `Node* prev`, `Node* next`.
- Empty list: `head_ == tail_ == nullptr`, `size_ == 0`.
- Iterators are raw `Node*` wrappers; `end()` is `nullptr`.

---

## 3. Internal Representation

```cpp
struct Node {
    T data;
    Node* prev;
    Node* next;
};

Node*   head_;   // first node, or nullptr
Node*   tail_;   // last node, or nullptr
size_t  size_;   // node count
```

**Invariant:** walking `next` from `head_` exactly `size_` steps reaches
`tail_`; `tail_->next == nullptr`; `head_->prev == nullptr`.

### Per-element memory cost

| Part | Cost |
|---|---|
| `T` object | `sizeof(T)` |
| `prev` + `next` pointers | 16 bytes on 64-bit |
| Allocation overhead | malloc metadata per node |

Compare to vector: one pointer + contiguous `T` array — far better cache use.

---

## 4. How It Works (Step by Step)

### 4.1 `push_back` — append at tail

```
   empty list:
       head_ = tail_ = new_node

   non-empty:
       tail_->next = new_node
       new_node->prev = tail_
       tail_ = new_node
```

```
   before:  head ─▶ [A] ◀─▶ [B] ◀── tail
   push_back(C):
   after:   head ─▶ [A] ◀─▶ [B] ◀─▶ [C] ◀── tail
```

### 4.2 `push_front` — prepend at head

Mirror of `push_back` on the head side:

```
   before:  head ─▶ [B] ◀── tail
   push_front(A):
   after:   head ─▶ [A] ◀─▶ [B] ◀── tail
```

### 4.3 `pop_front` / `pop_back` — unlink and delete

```
   pop_front:
       old = head_;  head_ = head_->next
       if head_: head_->prev = nullptr;  else tail_ = nullptr
       delete old;  --size_
```

Only pointer updates — remaining nodes stay at the same addresses.

### 4.4 Iteration

```
   for (it = begin(); it != end(); ++it)
       it.node_ walks: head_->next->next->...->nullptr
```

Bidirectional: `--it` follows `prev` links.

### 4.5 What full `std::list` adds (not in this minimal impl)

A complete list also provides O(1) `insert`/`erase` at an iterator by splicing
nodes in/out of the chain — the same pointer-rewiring idea as push/pop, but in
the middle. This teaching version implements end operations only.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `List<T>()` | empty list |
| `List<T>{a, b, c}` | initializer list |
| copy / move ctor | deep copy / O(1) steal chain |
| copy / move assign | replace contents |

### Element access
| Call | Notes |
|---|---|
| `front()` / `back()` | O(1); undefined if empty |
| No `operator[]` | must iterate to reach index `i` |

### Capacity
`empty()`, `size()`

### Modifiers
`push_front`, `push_back`, `pop_front`, `pop_back`, `clear`

### Iterators
`begin()` → first node; `end()` → `nullptr` (bidirectional)

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `push_front`, `push_back` | O(1) | one `new Node` |
| `pop_front`, `pop_back` | O(1) | one `delete` |
| `front`, `back` | O(1) | |
| Access by index | O(n) | walk the chain |
| Search | O(n) | linear scan |
| `clear` | O(n) | delete every node |
| copy | O(n) | clone all elements |
| move | O(1) | steal pointers |

---

## 7. Usage

```cpp
#include "list/list.hpp"

List<int> l;
l.push_back(1);
l.push_back(2);
l.push_front(0);   // {0, 1, 2}

for (int x : l) std::cout << x << ' ';

l.pop_front();
l.pop_back();
```

See [`list_example.cpp`](list_example.cpp) for LRU-style patterns, playlists,
and list-vs-vector trade-offs.

---

## 8. Design Decisions & Trade-offs

- **No sentinel node.** `end()` is `nullptr` — simpler allocation, but unlike
  some textbook implementations that use a dummy head.
- **Separate allocation per node.** Simple `new`/`delete`; a production list
  might use an arena or cached node allocator.
- **Doubly linked.** Extra pointer per node vs singly linked, but enables O(1)
  `pop_back` and bidirectional iterators.
- **Stable addresses.** A `Node*` (or iterator) stays valid when other nodes
  are added/removed — unlike vector reallocation.
- **Poor locality.** Pointer chasing defeats CPU prefetch; vector wins on scans.

---

## 9. Common Pitfalls

- **No random access.** `l[i]` does not exist — use an iterator or walk manually.
- **`front()`/`back()` on empty list.** Undefined behavior, like `std::list`.
- **Iterator invalidation.** Iterators to erased nodes dangle; iterators to other
  nodes remain valid when this impl only pushes/pops at ends.
- **Memory overhead.** Two pointers + allocator overhead per element — wasteful
  for tiny `T` (consider vector or deque).
- **Don't assume cache performance.** Benchmarks favor vector for most workloads.

---

## 10. Comparison with `std::list`

**Same:** doubly-linked node structure, O(1) end push/pop, bidirectional
iterators, stable element addresses.

**Intentionally omitted for clarity:** `insert`/`erase`/`splice` at iterator,
`emplace`, allocators, `remove`/`unique`/`merge`/`sort`, size guaranteed O(1)
via sentinel tricks.

---

## 11. Build & Run

```bash
make run-list       # build + run the examples
make test-list      # build + run the unit tests
make all            # build everything in the repo
```

---

## 12. See Also

- [`vector`](../vector/README.md) — contiguous, cache-friendly dynamic array
- [`deque`](../deque/README.md) — chunked, O(1) both ends + random access
- [`array`](../array/README.md) — fixed-size stack wrapper
