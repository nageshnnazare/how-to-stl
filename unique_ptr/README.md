# UniquePtr — Exclusive-Ownership Smart Pointer

> A `UniquePtr<T>` is a move-only RAII wrapper around one heap object. Exactly one
> `UniquePtr` owns the object at any time; when that owner is destroyed, reset, or
> replaced, the deleter runs automatically. Ownership transfers only through
> `std::move` — copies are deleted at compile time.

This is a from-scratch reimplementation of `std::unique_ptr` built for learning. The
header is [`unique_ptr.hpp`](unique_ptr.hpp), runnable examples are in
[`unique_ptr_example.cpp`](unique_ptr_example.cpp), and the test suite is in
[`../tests/unique_ptr_test.cpp`](../tests/unique_ptr_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Ownership | **Exclusive** — one owner at a time |
| Copyable | **No** (copy ctor / copy assign deleted) |
| Moveable | **Yes** — `std::move` transfers ownership |
| Overhead | **8 bytes** on 64-bit with stateless deleter (Empty Base Optimization) |
| Thread-safe | **No** for concurrent mutation of the same instance |

**Reach for a UniquePtr when** ownership is clear and singular: factory returns,
Pimpl members, sole container elements, or any `new` that must not leak on exceptions.

**Look elsewhere when** multiple parts of the program need shared lifetime
(see [`shared_ptr`](../shared_ptr/README.md)) or the object lives on the stack /
in a fixed-size container (see [`array`](../array/README.md), [`vector`](../vector/README.md)).

---

## 2. Mental Model

A `UniquePtr` is a tiny stack object holding a raw pointer (and optionally a deleter).
It does **not** allocate memory for itself — only the managed object lives on the heap.

```
   UniquePtr object (stack)              Heap
   ┌───────────────────┐               ┌─────────────────┐
   │ ptr_       ●──────┼──────────────▶│   T object      │
   │ deleter_   (0 B*) │               │   members...    │
   └───────────────────┘               └─────────────────┘
   * Empty Base Optimization: stateless DefaultDeleter adds no size
```

**The invariant:** at most one `UniquePtr` has a non-null `ptr_` for a given heap
object. Move transfers that responsibility; copy is forbidden.

### Class hierarchy (what this repo implements)

```
UniquePtr<T, Deleter>
├── Primary template (single objects)
│   ├── Members: T* ptr_, Deleter deleter_
│   ├── operator*(), operator->()
│   ├── Converting move: UniquePtr<Derived> → UniquePtr<Base>
│   └── Deleter calls: delete
│
└── Specialization UniquePtr<T[], Deleter>
    ├── Members: T* ptr_, Deleter deleter_
    ├── operator[] only (no * or ->)
    ├── No pointer-type conversions
    └── Deleter calls: delete[]
```

---

## 3. Internal Representation

```cpp
T*       ptr_;      // raw pointer to the managed object (nullptr when empty)
Deleter  deleter_;  // callable that destroys ptr_ (default: delete or delete[])
```

**Invariant:** if `ptr_ != nullptr`, this `UniquePtr` is the sole owner. After a move,
the source's `ptr_` is always `nullptr`.

### Memory layout — single object

```
Stack:                          Heap:
┌─────────────────┐            ┌─────────────────┐
│   UniquePtr     │            │     Object      │
│  ┌───────────┐  │            │                 │
│  │ ptr_ ─────┼──┼───────────>│ member1         │
│  ├───────────┤  │            │ member2         │
│  │ deleter_  │  │            │ ...             │
│  │ (size: 0*)│  │            └─────────────────┘
│  └───────────┘  │
└─────────────────┘
Total: 8 bytes (64-bit) with stateless deleter via EBO
```

### Memory layout — array specialization

```
Stack:                          Heap:
┌─────────────────┐            ┌───┬───┬───┬───┐
│ UniquePtr<T[]>  │            │[0]│[1]│[2]│...│
│  ptr_ ──────────┼───────────>└───┴───┴───┴───┘
│  deleter_       │            Access: arr[i] → *(ptr_ + i)
└─────────────────┘
```

### The four primitive operations

| Step | What | Code in `unique_ptr.hpp` |
|---|---|---|
| Acquire ownership | Store raw pointer in ctor | `UniquePtr(T* p)` |
| Release on scope end | `deleter_(ptr_)` if non-null | `~UniquePtr()` |
| Transfer ownership | Steal pointer, null source | move ctor / `release()` |
| Replace managed object | Delete old, store new | `reset(p)` |

---

## 4. How It Works (Step by Step)

### 4.1 RAII lifetime — construction → usage → destruction

```
1. Construction                 2. Usage                    3. Destruction
┌──────────────┐               ┌──────────────┐             ┌──────────────┐
│ new Object() │               │ ptr->method()│             │    ~Ptr()    │
└──────┬───────┘               └──────┬───────┘             └──────┬───────┘
       │                              │                            │
       v                              v                            v
┌──────────────┐               ┌──────────────┐            ┌──────────────┐
│  Wrap in     │               │ Dereference  │            │if (ptr_)     │
│ UniquePtr    │               │  ptr_ to     │            │  deleter_()  │
│              │               │ access object│            │              │
└──────────────┘               └──────────────┘            └──────────────┘
  Object owned                 Object still owned           Object deleted
  by unique_ptr                by unique_ptr                Memory freed
```

**Why RAII:** if `doWork()` throws mid-function, the destructor still runs — no
manual `delete` in every exit path.

### 4.2 Move semantics — transferring exclusive ownership

```
Before:  ptr1 ──> [Object]     ptr2 ──> nullptr

Move:    ptr2 = std::move(ptr1)

After:   ptr1 ──> nullptr      ptr2 ──> [Object]
```

Implementation (move constructor):

1. Copy `other.ptr_` into `this->ptr_`.
2. Move `other.deleter_` (stateful deleters keep their state).
3. Set `other.ptr_ = nullptr` — **critical**; prevents double-delete.

**Why move-only:** copying would create two owners → double-delete. Deleting copy
operations turns that bug into a compile error.

### 4.3 `reset` — replace or clear the managed object

```
   reset(new T) on ptr owning [Old]:

   (1) save old_ptr = ptr_
   (2) ptr_ = p                      // point at [New] first
   (3) if (old_ptr) deleter_(old_ptr) // then destroy [Old]

   Order matters: self-reset (ptr.reset(ptr.get())) stays safe.
```

### 4.4 `release` — give up ownership without deleting

```
   Owning state ──release()──> ptr_ = nullptr, return raw pointer
                                      │
                                      v
                               Caller must delete manually
```

### 4.5 Deleter dispatch (compile-time)

```
                    UniquePtr<T, Deleter>
                            │
              ┌─────────────┴─────────────┐
              ▼                           ▼
    T is single object              T is array (T[])
              │                           │
              ▼                           ▼
   DefaultDeleter<T>::                DefaultDeleter<T[]>::
    operator()(T*)                     operator()(T*)
              │                           │
              ▼                           ▼
        delete ptr;                  delete[] ptr;
```

Custom deleter example (`FILE`):

```
UniquePtr<FILE, FileDeleter>
         │ ~UniquePtr()
         ▼
  deleter_(ptr_) → FileDeleter::operator()(FILE*) → fclose(f)
```

### 4.6 Polymorphic conversion (Derived → Base)

```
    Base                         UniquePtr<Derived> d = makeUnique<Derived>();
      ▲                                    │
      │                                    │ std::move
   Derived                                 ▼
                                  UniquePtr<Base> b
                                           │ ~UniquePtr
                                           ▼
                                  delete (Base*) → ~Derived() then ~Base()
```

Array covariance is **not** provided — `Derived[]` → `Base[]` would call the wrong
`delete[]` element size.

### 4.7 `makeUnique` — why it is exception-safe

**Unsafe (evaluation order unspecified):**

```cpp
func(UniquePtr<A>(new A()), UniquePtr<B>(new B()));
// If B's new throws after A's new, A leaks — wrappers not constructed yet.
```

**Safe:**

```cpp
func(makeUnique<A>(), makeUnique<B>());
// Each allocation is immediately wrapped; destructor cleans up on throw.
```

```
try {
    UniquePtr<A> a = makeUnique<A>();  ← A allocated + wrapped
    UniquePtr<B> b = makeUnique<B>();  ← B allocated + wrapped
    doWork();                          ← may throw
}
catch (...) {
    // Both destructors run — no leak
}
```

### 4.8 Operation state machine

```
                    ┌──────────────┐
                    │   nullptr    │  ← default constructed
                    │  (no object) │
                    └──────┬───────┘
                           │ reset(new T)
                    ┌──────▼───────┐
         release()  │              │  reset()
        ┌───────────┤   Owning     ├─────────┐
        │           │   (has obj)  │         │
        │           └──────┬───────┘         │
        │                  │ std::move       │
        ▼                  ▼                 ▼
┌──────────────┐    ┌──────────────┐  ┌──────────────┐
│   Manual     │    │   Moved-from │  │   nullptr    │
│   delete     │    │   (nullptr)  │  │  (no object) │
│   required   │    └──────────────┘  └──────────────┘
└──────────────┘
```

### 4.9 Function parameter patterns

**Sink (take ownership by value):**

```
Caller  UniquePtr owns ──std::move──>  Function UniquePtr owns ──> ~ deletes
Caller left empty
```

**Borrow (const reference):**

```
Caller owns ──const UniquePtr&──> Function reads only ──> Caller still owns
```

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `UniquePtr<T>()` | empty (`ptr_ == nullptr`) |
| `UniquePtr<T>(nullptr_t)` | empty |
| `UniquePtr<T>(T* p)` | take ownership of `p` (**explicit**) |
| `UniquePtr<T>(T* p, Deleter d)` | take ownership with custom deleter |
| move ctor | steal pointer, null source |
| converting move ctor | `UniquePtr<U>&&` → `UniquePtr<T>` if `U*` → `T*` |
| copy ctor / copy assign | **deleted** |

### Modifiers
| Call | Effect |
|---|---|
| `release()` | return raw pointer, become empty (no delete) |
| `reset(p = nullptr)` | delete current, optionally manage `p` |
| `swap(other)` | exchange pointer and deleter |
| move assign | reset to other's object, null other |

### Observers
`get()`, `get_deleter()`, `explicit operator bool()`, `operator*`, `operator->`

### Array specialization (`UniquePtr<T[]>`)
`operator[](size_t)` — no `*` or `->`; uses `delete[]` via `DefaultDeleter<T[]>`

### Non-member
`==`, `!=` (with other `UniquePtr` and with `nullptr`), `swap`, `makeUnique`, `makeUniqueArray`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| Construction | O(1) | pointer store only |
| Destruction | O(1) | one deleter call |
| Move ctor / move assign | O(1) | pointer + deleter move |
| `reset` / `release` | O(1) | `reset` may call deleter once |
| `get` / `operator*` / `operator->` | O(1) | same as raw pointer |
| Memory (stateless deleter) | 8 bytes | identical to `T*` |

```
Operation           Raw Pointer    unique_ptr     Overhead
─────────────────────────────────────────────────────────
Construction        O(1)           O(1)           0%
Destruction         O(1)           O(1)           0%
Move                O(1)           O(1)           0%
Dereference         O(1)           O(1)           0%
Memory              8 bytes        8 bytes*       0%
                                   *with EBO for stateless deleters
```

---

## 7. Usage

```cpp
#include "unique_ptr/unique_ptr.hpp"

// Preferred: exception-safe factory style
auto ptr = makeUnique<Resource>("db", 1);
ptr->use();

// Transfer ownership into a sink function
void consume(UniquePtr<Resource> p) { /* p deleted on return */ }
consume(std::move(ptr));   // ptr is now empty

// Arrays — note T[], not T
auto arr = makeUniqueArray<int>(10);
arr[0] = 42;

// Custom deleter for C APIs
UniquePtr<FILE, FileDeleter> f(fopen("data.txt", "r"), FileDeleter{});
```

See [`unique_ptr_example.cpp`](unique_ptr_example.cpp) for move semantics, polymorphism,
custom deleters, reset/release, and comparison operators.

---

## 8. Design Decisions & Trade-offs

- **Deleted copies.** Enforces exclusive ownership at compile time; use `shared_ptr`
  when sharing is intentional.
- **Separate `T[]` specialization.** `delete` vs `delete[]` is not interchangeable;
  specialization also drops unsafe array conversions.
- **Deleter as second template parameter.** Type-encoded cleanup (`UniquePtr<FILE,
  FileDeleter>`); stateless deleters cost zero bytes via Empty Base Optimization.
- **`explicit` pointer constructor.** Prevents `func(new T)` from silently wrapping
  a raw pointer.
- **`noexcept` move operations.** Enables STL containers to move `UniquePtr` instead
  of copying during reallocation.
- **Simplified vs `std::unique_ptr`.** No compressed-pair layout, reference deleters,
  or allocator support — kept minimal for teaching.

---

## 9. Common Pitfalls

- **Double ownership from the same raw pointer.**
  ```cpp
  T* raw = new T;
  UniquePtr<T> a(raw);
  UniquePtr<T> b(raw);  // ☠ double-delete when either is destroyed
  ```
- **Wrong type for arrays.**
  ```cpp
  UniquePtr<int> bad(new int[10]);   // ☠ calls delete, not delete[]
  UniquePtr<int[]> good(new int[10]); // ✓
  ```
- **Dangling `get()`.**
  ```cpp
  T* raw = ptr.get();
  ptr.reset();   // or ptr goes out of scope
  raw->foo();    // ☠ dangling
  ```
- **Forgetting `std::move` into by-value parameters.**
  ```cpp
  void sink(UniquePtr<T> p);
  sink(ptr);            // compile error (good!)
  sink(std::move(ptr)); // correct
  ```
- **Returning `get()` from a function.**
  ```cpp
  T* leak() {
      auto p = makeUnique<T>();
      return p.get();  // ☠ object destroyed at end of leak()
  }
  ```

### Guarantees vs non-guarantees

```
╔═══════════════════════════════════════════════════╗
║  GUARANTEES                                       ║
╠═══════════════════════════════════════════════════╣
║  ✓ Object deleted exactly once                    ║
║  ✓ Deleted when last owner destroyed / reset      ║
║  ✓ No double-deletion (with correct usage)        ║
║  ✓ Exception-safe cleanup via destructor          ║
║  ✓ Zero runtime overhead vs raw pointer (opt)     ║
╠═══════════════════════════════════════════════════╣
║  DOES NOT GUARANTEE                               ║
╠═══════════════════════════════════════════════════╣
║  ✗ Thread-safety on the same instance             ║
║  ✗ Non-null dereference safety (UB if null)        ║
║  ✗ Safety if external code deletes get() pointer  ║
╚═══════════════════════════════════════════════════╝
```

---

## 10. Comparison with `std::unique_ptr`

**Same:** exclusive ownership, move-only semantics, `T[]` specialization, custom
deleters, `make_unique` pattern, `nullptr` comparisons.

**Intentionally simplified:** compressed-pair storage, reference-wrapper deleters,
allocator-aware deleters, broader converting-constructor SFINAE, `constexpr` in
C++20 modes.

**vs raw pointer:**

```
Raw pointer: manual new/delete, copyable, leak-prone, unclear ownership
unique_ptr:  automatic cleanup, move-only, exception-safe, clear ownership
```

---

## 11. Build & Run

```bash
make run-unique     # build + run the examples
make test-unique    # build + run the unit tests
make all            # build everything in the repo
```

From the repo root directly:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. unique_ptr/unique_ptr_example.cpp -o /tmp/x_unique_ptr
```

---

## 12. See Also

- [`shared_ptr`](../shared_ptr/README.md) — shared ownership and reference counting
- [`vector`](../vector/README.md) — `vector<UniquePtr<T>>` for owning collections
- [`allocator`](../allocator/README.md) — what `new` / `delete` do under the hood
