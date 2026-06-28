# SharedPtr — Reference-Counted Shared Ownership

> A `SharedPtr<T>` is a copyable smart pointer that shares ownership of one heap
> object with any number of sibling `SharedPtr`s. A separate **control block**
> holds atomic strong and weak counts; when the last strong owner disappears the
> object is deleted, and when both counts hit zero the control block is freed.

This is a from-scratch reimplementation of `std::shared_ptr` and `std::weak_ptr`
built for learning. The header is [`shared_ptr.hpp`](shared_ptr.hpp), runnable
examples are in [`shared_ptr_example.cpp`](shared_ptr_example.cpp), and the test
suite is in [`../tests/shared_ptr_test.cpp`](../tests/shared_ptr_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Ownership | **Shared** — many `SharedPtr` instances, one object |
| Copyable | **Yes** — copy increments strong count |
| Moveable | **Yes** — transfers one owner's slot without changing total count |
| Overhead | **16 bytes** per `SharedPtr` (object ptr + control-block ptr) + control block |
| Thread-safe counting | **Yes** — `std::atomic` on strong/weak counts |
| Thread-safe object access | **No** — you must synchronize mutations of `*ptr` |

**Reach for a SharedPtr when** several independent owners must keep an object alive
and you cannot pick a single clear owner — caches, graphs with `weak_ptr` back-edges,
observer lists, or APIs that copy handles freely.

**Look elsewhere when** one owner is obvious (see [`unique_ptr`](../unique_ptr/README.md))
or you only need a non-owning borrow with a guaranteed lifetime (raw pointer/reference
under a `unique_ptr` scope).

---

## 2. Mental Model

Every live `SharedPtr` points at **two** things: the user object and a shared
**control block** that remembers how many strong and weak handles exist.

```
   SharedPtr A (stack)          Control block (heap)         Managed object (heap)
   ┌──────────────────┐        ┌─────────────────────┐      ┌──────────────┐
   │ ptr_      ●──────┼───────>│ ptr_                │─────>│   Resource   │
   │ cb_       ●──────┼───┐    │ shared_count_ = 2   │      │   members    │
   └──────────────────┘   │    │ weak_count_   = 1   │      └──────────────┘
                          │    │ deleter_            │
   SharedPtr B (stack)     │    └─────────────────────┘
   ┌──────────────────┐   │
   │ ptr_      ●──────┼───┘  (same cb_ as A)
   │ cb_       ●──────┼──────> (same control block)
   └──────────────────┘

   WeakPtr W holds cb_ + weak_count bump, but does NOT increment shared_count_
```

- **Strong count** = number of `SharedPtr` handles → object deleted at **0**.
- **Weak count** = number of `WeakPtr` handles → control block deleted when
  **both** counts are **0**.

---

## 3. Internal Representation

### SharedPtr

```cpp
T* ptr_;                                    // cached address of managed object
ControlBlock<T, SharedPtrDeleter<T>>* cb_;  // shared metadata + counts
```

### ControlBlock

```cpp
T* ptr_;                         // object address (nulled after deletion)
std::atomic<long> shared_count_; // strong owners (SharedPtr)
std::atomic<long> weak_count_;   // weak observers (WeakPtr)
Deleter deleter_;                // called when last strong ref goes away
```

**Invariant:** all `SharedPtr`/`WeakPtr` sharing one object point at the **same**
`cb_`. `ptr_` in each handle is a denormalized copy for fast `operator->`.

### Why split object and control block?

```
┌─────────────┐     ┌──────────────────┐     ┌─────────────┐
│  SharedPtr  │────>│  ControlBlock    │────>│   Object    │
│  (16 bytes) │     │  counts+deleter  │     │  (sizeof T) │
└─────────────┘     └──────────────────┘     └─────────────┘
     ^                      ^
     │                      │
  second SharedPtr      WeakPtr also points here
  shares same cb_       (does not keep Object alive)
```

This layout lets `WeakPtr` outlive the object while still knowing when the control
block itself can be reclaimed.

---

## 4. How It Works (Step by Step)

### 4.1 Construction from raw pointer

```
SharedPtr<T> sp(new T);

   (1) allocate T on heap
   (2) allocate ControlBlock(ptr, deleter)   shared_count_ = 1, weak_count_ = 0
   (3) sp.ptr_ = T*,  sp.cb_ = block*

If step (2) throws → delete T (no leak).
```

### 4.2 Copy — sharing ownership

```
Before copy:   sp1 ──> cb (shared=1) ──> Object

sp2 = sp1;

After copy:    sp1 ──┐
                      ├──> cb (shared=2) ──> Object
               sp2 ──┘
```

`add_shared_ref()` does `shared_count_.fetch_add(1, relaxed)`.

### 4.3 Move — transfer one handle slot

```
Before: sp1 ──> cb (shared=2) <── sp2

sp1 = std::move(sp2);

After:  sp1 ──> cb (shared=2)     sp2 empty (ptr_=cb_=nullptr)
```

Total strong count unchanged — one owner's seat moved, not duplicated.

### 4.4 Destruction — last strong owner deletes object

```
~SharedPtr on last owner (shared_count goes 1 → 0):

   release_shared_ref():
      deleter_(ptr_);  ptr_ = nullptr inside cb
      if weak_count == 0 → delete control block
      else → keep control block for WeakPtr observers
```

```
Timeline with one SharedPtr + one WeakPtr:

   destroy last SharedPtr:  shared=0  → Object DELETED, cb KEPT (weak=1)
   destroy WeakPtr:         weak=0    → Control block DELETED
```

### 4.5 WeakPtr::lock() — promote observer to owner

```
weak.expired()?  (shared_count == 0) → return empty SharedPtr

else try_add_shared_ref(): CAS loop on shared_count_
   success → return SharedPtr(ptr, cb, add_ref=false)
   fail (count hit 0 mid-flight) → return empty SharedPtr
```

**Why CAS:** between `expired()` and `lock()`, another thread might drop the last
`SharedPtr`; the atomic increment must fail cleanly.

### 4.6 Breaking cyclic references

**Problem — both edges strong:**

```
   Node A <──────── strong ────────> Node B
        └──── strong parent/child ────┘

   A.use_count() >= 1 because B holds A
   B.use_count() >= 1 because A holds B
   → neither destructor runs → LEAK
```

**Fix — weak back-edge:**

```
   Node A ──strong──> Node B
   Node A <──weak────  Node B   (parent_ is WeakPtr)

   When external SharedPtrs released:
   strong counts hit 0 → nodes destroyed → weak refs decay → cb freed
```

### 4.7 makeShared vs two-step new

This teaching implementation allocates **twice**:

```cpp
SharedPtr<T>(new T());  // (1) object  (2) control block
makeShared<T>();        // same here — not fused
```

Production `std::make_shared` uses one allocation for object + control block
(faster, better cache locality). Our version keeps the two blocks separate so
each allocation is visible when learning.

---

## 5. API Reference

### SharedPtr — construction
| Call | Effect |
|---|---|
| `SharedPtr<T>()` | empty |
| `SharedPtr<T>(T* p)` | new control block, `shared_count = 1` |
| copy ctor | share `cb_`, increment strong count |
| move ctor | steal pointers, null source |
| converting copy/move | `SharedPtr<U>` → `SharedPtr<T>` if `U*` → `T*` |

### SharedPtr — modifiers / observers
| Call | Effect |
|---|---|
| `reset(p = nullptr)` | replace managed object (via temp + swap) |
| `swap` | exchange `ptr_` and `cb_` |
| `get()` | raw pointer, no count change |
| `use_count()` | current strong count (atomic load) |
| `unique()` | `use_count() == 1` |
| `operator*`, `operator->` | dereference (UB if null) |

### WeakPtr
| Call | Effect |
|---|---|
| `WeakPtr(SharedPtr const&)` | increment `weak_count_` only |
| `expired()` | `use_count() == 0` |
| `lock()` | `SharedPtr` if object alive, else empty |
| `reset()` | release weak handle |

### Non-member
`makeShared`, `static_pointer_cast`, `dynamic_pointer_cast`, comparisons, `swap`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| Copy `SharedPtr` | O(1) | atomic increment |
| Move `SharedPtr` | O(1) | pointer copies |
| `~SharedPtr` | O(1) amortized | atomic decrement; may delete object + cb |
| `lock()` | O(1) typical | CAS retry under contention |
| `get` / `operator->` | O(1) | no atomic on dereference |
| Memory | 16 B + cb | cb ≈ two atomics + pointer + deleter |

---

## 7. Usage

```cpp
#include "shared_ptr/shared_ptr.hpp"

auto a = makeShared<Resource>("cfg", 1);
auto b = a;                        // shared_count == 2
std::cout << a.use_count();        // 2

WeakPtr<Resource> weak = a;
a.reset();                         // object destroyed if a was last owner
if (auto locked = weak.lock()) {   // empty — expired
    locked->use();
}

// Graph edge: strong child, weak parent
struct Node {
    SharedPtr<Node> child_;
    WeakPtr<Node>   parent_;
};
```

See [`shared_ptr_example.cpp`](shared_ptr_example.cpp) for containers of `SharedPtr`,
cyclic-reference breaking, casts, and cache patterns.

---

## 8. Design Decisions & Trade-offs

- **Separate control block.** Mirrors libstdc++/libc++ layout pedagogy; enables
  `WeakPtr` and makes counts visible. Cost: extra allocation vs fused `make_shared`.
- **Atomic `long` counts.** Reference updates are thread-safe; managed object data
  is not — same split as `std::shared_ptr`.
- **Denormalized `ptr_` in handles.** Fast dereference without loading through
  `cb_` every time; must stay consistent with control block until destruction.
- **Assignment via temp + swap.** Strong guarantee when re-binding handles; old
  state cleaned when `temp` dies.
- **`try_add_shared_ref` CAS loop.** Prevents `lock()` from resurrecting an object
  already at `shared_count == 0`.
- **Simplified deleter story.** Always `SharedPtrDeleter<T>` with `delete` — no
  custom deleter template parameter (unlike production STL).

---

## 9. Common Pitfalls

- **Cyclic `SharedPtr` graphs leak.** Use `WeakPtr` for back-references.
- **Mistaking thread-safe counts for thread-safe data.**
  ```cpp
  SharedPtr<State> s = makeShared<State>();
  // Thread 1: s->x = 1;   // data race on State — need a mutex
  // Thread 2: s->x = 2;
  ```
- **`use_count()` is expensive and racy** for synchronization — it is a snapshot;
  do not use it as a lock protocol.
- **`enable_shared_from_this` not implemented.** Calling `SharedPtr(this)` in a
  constructor creates a **second** control block → double-delete. (Not in this repo.)
- **Mixing raw pointer and multiple `SharedPtr`s.**
  ```cpp
  T* raw = new T;
  SharedPtr<T> a(raw);
  SharedPtr<T> b(raw);  // ☠ two control blocks, double-delete
  ```
- **`dynamic_pointer_cast` failure** returns empty `SharedPtr` — always test `if (derived)`.

---

## 10. Comparison with `std::shared_ptr`

**Same:** shared/weak counts, atomic ref updates, `lock()` / `expired()`, aliasing
behavior conceptually, pointer casts.

**Intentionally simplified:** single fused allocation in `make_shared`, custom
deleters, `std::enable_shared_from_this`, aliasing constructor, array support,
`owner_before`, atomic `shared_ptr` overloads.

**vs `unique_ptr`:**

| | `unique_ptr` | `shared_ptr` |
|---|---|---|
| Ownership | Exclusive | Shared |
| Copy | Deleted | Increments count |
| Size | 8 bytes | 16 bytes + control block |
| Count atomics | None | Yes |
| Default choice | **Yes** | When sharing required |

---

## 11. Build & Run

```bash
make run-shared     # build + run the examples
make test-shared    # build + run the unit tests
make all            # build everything in the repo
```

From the repo root directly:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. shared_ptr/shared_ptr_example.cpp -o /tmp/x_shared_ptr
```

---

## 12. See Also

- [`unique_ptr`](../unique_ptr/README.md) — exclusive ownership, zero count overhead
- [`vector`](../vector/README.md) — storing `SharedPtr` in dynamic arrays
- [`list`](../list/README.md) — node graphs that pair `SharedPtr` child / `WeakPtr` parent
