# Arena Allocator — Per-Thread Bump Memory Pools

> A `ThreadArenaAllocator` gives every OS thread its own bump arena so hot-path
> allocations are pointer arithmetic with no mutex. Individual frees are
> deliberate no-ops; reset the arena (or use `ScopedArena`) when a request,
> frame, or compile pass ends and reclaim everything at once.

This is a from-scratch reimplementation of per-thread arena allocation built for
learning. The header is [`arena_allocator.hpp`](arena_allocator.hpp), runnable
examples are in [`arena_allocator_example.cpp`](arena_allocator_example.cpp),
and the test suite is in
[`../tests/arena_allocator_test.cpp`](../tests/arena_allocator_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Allocation model | Bump pointer per thread |
| Hot-path lock | **None** (after arena exists) |
| Individual free | **No** — bulk `reset()` only |
| Thread safety | Safe across threads (separate arenas) |
| Default arena size | 1 MiB per thread |
| Alignment | Rounded on every `allocate()` |

**Reach for an arena when** many threads allocate short-lived temporaries and
malloc contention shows up in profiles — web requests, game frames, compiler
front-ends.

**Look elsewhere when** objects have independent lifetimes (see
[`allocator`](../allocator/README.md) pool/free-list), or you need one shared
heap with arbitrary frees.

---

## 2. Mental Model

Each thread owns a private slab; a global manager only creates arenas on first use:

```
   ThreadArenaAllocator                    Thread 1          Thread 2
   ┌──────────────────────┐               ┌─────────┐       ┌─────────┐
   │ mutex_ + arenas_ map │──creates──▶   │ Arena 1 │       │ Arena 2 │
   │ arena_size_ = 1MB  │               │ bump ──▶│       │ bump ──▶│
   └──────────────────────┘               └─────────┘       └─────────┘
        ▲ lock only on                           │ lock-free allocate()
        │ first touch per thread                 ▼
```

- Bytes `[0, offset_)` in an arena are **handed out**.
- Bytes `[offset_, size_)` are **available** until the next bump.
- `reset()` sets `offset_ = 0` — instant reclaim of the whole slab.

---

## 3. Internal Representation

### Arena (one thread's slab)

```cpp
char*                 buffer_;   // malloc'd slab base
size_t                size_;     // fixed capacity
std::atomic<size_t>   offset_;   // bump pointer (relaxed stores on allocate)
```

**Invariant:** `0 <= offset_ <= size_` when used from the owning thread.

### ThreadArenaAllocator (manager)

```cpp
size_t                              arena_size_;  // bytes per new Arena
mutable std::mutex                  mutex_;       // protects arenas_ map
std::unordered_map<std::thread::id, Arena*> arenas_;
```

### Helpers

| Type | Role |
|---|---|
| `TypedArenaAllocator<T>` | typed `allocate(n)` / `construct` / `destroy` |
| `ScopedArena` | RAII `reset_thread_arena()` on scope exit |
| `ArenaVector<T>` | growable vector backed by arena bumps |

---

## 4. How It Works (Step by Step)

### 4.1 Bump allocation with alignment

```
   offset_=48, request 32 bytes, align=16

   (1) pad: 48 % 16 == 0  →  aligned_offset = 48
   (2) new_offset = 48 + 32 = 80  ≤ size_  → OK
   (3) return buffer_+48; store offset_=80

   slab:  [████████ used ████████│ NEW 32B │░░ free ░░]
```

Alignment padding is mandatory: returning misaligned addresses breaks `alignof(T)`
for placement new and SIMD loads.

### 4.2 First allocation on a new thread

```
   Thread T calls allocate(64):

   lock(mutex_)
   if T not in arenas_:
       arenas_[T] = new Arena(arena_size_)   // e.g. 1 MiB malloc once
   unlock
   arenas_[T]->allocate(64)   // no lock — bump only
```

Subsequent allocations on thread T still call `get_thread_arena()` (map lookup
under mutex). A production version would cache `Arena*` in `thread_local` storage.

### 4.3 Bulk reclaim — reset vs ScopedArena

```
   request handler {
       ScopedArena scope(alloc);
       void* buf = scope.allocate(4096);
       parse(buf);
   }  // ~ScopedArena → reset_thread_arena() → offset_ = 0
```

No per-allocation `free()` — the entire request's bumps vanish in one store.

### 4.4 ArenaVector growth

When `size_ == capacity_`, vector doubles by bumping a **new** larger array in the
arena and move-constructing elements. Old array bytes are abandoned until arena
reset (classic arena trade-off: simple growth, no individual block free).

---

## 5. API Reference

### ThreadArenaAllocator
| Call | Effect |
|---|---|
| `ThreadArenaAllocator(arena_size)` | construct manager (lazy arenas) |
| `allocate(size, align)` | bump on current thread's arena |
| `reset_thread_arena()` | `offset_ = 0` for caller's thread |
| `reset_all()` | reset every arena |
| `get_thread_stats()` | used / available / capacity |
| `get_global_stats()` | aggregate across arenas |
| `num_arenas()` | map size |

### Arena
| Call | Effect |
|---|---|
| `allocate(size, align)` | bump; nullptr if full |
| `reset()` | offset → 0 |
| `used()` / `available()` / `capacity()` | introspection |

### ScopedArena
| Call | Effect |
|---|---|
| `ScopedArena(alloc)` | bind manager |
| `allocate` / `create<T>(...)` | forward to manager |
| destructor | `reset_thread_arena()` |

### ArenaVector\<T\>
| Call | Effect |
|---|---|
| `push_back(v)` | grow via new arena bump if needed |
| `operator[]`, `size`, `begin/end` | minimal vector surface |

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `Arena::allocate` | O(1) | pointer bump + alignment pad |
| `Arena::reset` | O(1) | single atomic store |
| First alloc per thread | O(1) + lock | map insert + `new Arena` |
| Later `get_thread_arena` | O(1) amortized | hash map + mutex |
| `ArenaVector::push_back` | O(1) amortized | O(n) when regrow copies elements |
| Individual free | N/A | by design |

---

## 7. Usage

```cpp
#include "arena_allocator/arena_allocator.hpp"

ThreadArenaAllocator alloc(1024 * 128);  // 128 KiB per thread

// Direct bump
int* nums = static_cast<int*>(alloc.allocate(10 * sizeof(int)));

// RAII per request
{
    ScopedArena scope(alloc);
    auto* node = scope.create<ASTNode>(token);
}  // arena reset for this thread

// Arena-backed container
ArenaVector<int> vec(alloc);
for (int i = 0; i < 100; ++i) vec.push_back(i * 2);

alloc.reset_thread_arena();  // or reset_all() between phases
```

See [`arena_allocator_example.cpp`](arena_allocator_example.cpp) for multi-threaded
demos, scoped reset, `ArenaVector`, and malloc comparison.

---

## 8. Design Decisions & Trade-offs

- **Per-thread slabs vs one locked bump.** Eliminates cross-thread false sharing
  and malloc lock contention; costs `arena_size_` reserved per active thread.
- **Atomic offset without CAS loop.** Single-writer assumption (owning thread);
  not safe for multiple threads bumping the same `Arena`.
- **No individual free.** Keeps allocate at ~3–5 instructions; callers batch lifetimes.
- **Lazy arena creation.** Threads that never allocate pay zero arena memory.
- **ArenaVector abandons old buffers.** Simpler than compating; acceptable when a
  `reset()` follows soon after.

---

## 9. Common Pitfalls

- **Arena exhaustion.** `allocate()` returns nullptr when `offset_ + size > size_`;
  size arenas for peak per-thread usage or call `reset()` more often.
- **Forgotten reset.** Without `ScopedArena` or explicit reset, bumps accumulate
  until the arena is full even if objects are logically dead.
- **Nested ScopedArena.** Inner scope reset wipes outer scope's allocations too —
  one scope per thread per logical unit of work.
- **Cross-thread pointer use.** Memory allocated on thread A's arena should be
  used on thread A unless you guarantee no concurrent bump/reset.
- **Destructors before reset.** `reset()` does not call `~T()`; destroy objects or
  use RAII wrappers that destroy in `~ScopedArena` before reset.

---

## 10. Comparison with `malloc` and `LinearAllocator`

**vs malloc:** arenas win on thread-local hot paths; malloc wins on arbitrary
lifetimes and global fairness.

**vs `LinearAllocator` in [`allocator`](../allocator/README.md):** same bump
semantics, but `ThreadArenaAllocator` adds per-thread isolation and a mutex-protected
registry so worker threads do not share one offset.

**Not `std::pmr::monotonic_buffer_resource`:** similar bump idea, but this code is
explicit about thread mapping and teaching diagrams.

---

## 11. Build & Run

```bash
make run-arena-allocator     # build + run the examples
make test-arena-allocator    # build + run the unit tests
make all                     # build everything in the repo
```

Manual compile from repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. \
    arena_allocator/arena_allocator_example.cpp -o /tmp/x_arena_allocator
```

---

## 12. See Also

- [`allocator`](../allocator/README.md) — single-thread linear, pool, stack, free-list
- [`thread_pool`](../thread_pool/README.md) — workers that pair well with per-thread arenas
- [`locks`](../locks/README.md) — mutex protecting the arena map
- [`vector`](../vector/README.md) — `ArenaVector` is a minimal arena-backed cousin
