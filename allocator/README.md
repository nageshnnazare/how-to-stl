# Allocator — Custom Memory Allocation Strategies

> Four hand-rolled allocators that trade generality for speed. Each one matches a
> known allocation pattern — bump forward, fixed-size pools, LIFO scopes, or
> variable-size free lists — so hot paths avoid malloc's global bookkeeping and
> locking. Pick the strategy that fits your lifetime and size constraints.

This is a from-scratch reimplementation of specialized allocation techniques built
for learning. The header is [`allocator.hpp`](allocator.hpp), runnable examples
are in [`allocator_example.cpp`](allocator_example.cpp), and the test suite is in
[`../tests/allocator_test.cpp`](../tests/allocator_test.cpp).

---

## 1. What It Is

| Property | LinearAllocator | PoolAllocator | StackAllocator | FreeListAllocator |
|---|---|---|---|---|
| Allocation speed | **O(1)** bump | **O(1)** pop | **O(1)** bump + header | **O(n)** first-fit |
| Individual free | ❌ (bulk reset) | ✅ O(1) push | ❌ (markers) | ✅ + coalesce |
| Variable sizes | ✅ | ❌ fixed block | ✅ | ✅ |
| Fragmentation | None | None | Low | Can occur |
| Thread-safe | ❌ | ❌ | ❌ | ❌ |

**Reach for these allocators when** you know your allocation pattern ahead of time
and malloc's generality is costing measurable time — game frames, object pools,
parser scratch, or embedded slabs.

**Look elsewhere when** you need thread-safe general allocation (see
[`arena_allocator`](../arena_allocator/README.md)), arbitrary cross-thread frees,
or the full flexibility of the system heap.

---

## 2. Mental Model

Think of four different ways to carve a pre-allocated slab:

```
   LINEAR (bump)                 POOL (free list)
   ┌──used──┬─free─┐             free_list_ ──▶ [B2]──▶ [B0]──▶ null
   │ A │ B  │      │             slab: [B0][B1 in use][B2][B3 in use]
   └────────┴──────┘             allocate() = pop; deallocate() = push

   STACK (markers)               FREE LIST (variable)
   marker ──▶ rewind here        free_list_ ──▶ [120B]──▶ [64B]──▶ ...
   [hdr|data][hdr|data]...       allocate first-fit; dealloc merges neighbors
```

- **Linear**: one pointer moves right; `reset()` snaps it back to zero.
- **Pool**: every block is the same size; free blocks form a linked list threaded
  through their first bytes.
- **Stack**: like linear, but markers bookmark `offset_` for scope unwind.
- **Free list**: variable blocks with headers; coalescing merges adjacent holes.

---

## 3. Internal Representation

### LinearAllocator

```cpp
char*   buffer_;   // base of malloc'd slab
size_t  size_;     // total capacity
size_t  offset_;   // bump pointer: bytes [0, offset_) are "in use"
```

**Invariant:** `0 <= offset_ <= size_`.

### PoolAllocator

```cpp
char*      buffer_;       // slab holding num_blocks_ * block_size_ bytes
size_t     block_size_;   // every block is exactly this wide (>= sizeof(FreeNode))
size_t     num_blocks_;   // fixed capacity
FreeNode*  free_list_;    // head of intrusive free list (next pointer in free blocks)
```

**Invariant:** every free block's first bytes overlay a `FreeNode { next }`.

### StackAllocator

```cpp
char*   buffer_;
size_t  size_;
size_t  offset_;   // also exposed as Marker via get_marker()
// Each allocation preceded by Header { size, padding } in the slab
```

### FreeListAllocator

```cpp
char*    buffer_;
size_t   size_;
Header*  free_list_;   // intrusive list sorted by address after deallocate
// Header { size, next } — user pointer is sizeof(Header) past the node
```

---

## 4. How It Works (Step by Step)

### 4.1 LinearAllocator — bump and reset

```
   START   offset_=0   [                                    ]  size_=256

   allocate(40, align=16):
           pad to 16-byte boundary, return ptr, offset_ += 40+pad
           [████████ used ████████|          free           ]

   reset():
           offset_ = 0   — entire slab reusable, O(1)
```

No per-object free: all allocations in a frame share one lifetime. That is why
game engines reset a frame allocator once per tick instead of calling `free`
thousands of times.

### 4.2 PoolAllocator — pop and push the free list

Initialization links every block onto `free_list_`:

```
   slab:  [B0][B1][B2][B3]
   init:  free_list_ ──▶ B3 ──▶ B2 ──▶ B1 ──▶ B0 ──▶ null
```

`allocate()`:

```
   free_list_ ──▶ [B2]──▶ [B0]──▶ null
                 pop B2
   return B2;  free_list_ ──▶ [B0]──▶ null
```

`deallocate(B2)`:

```
   B2->next = free_list_;  free_list_ = B2;
   free_list_ ──▶ [B2]──▶ [B0]──▶ null
```

Contrast with malloc: no size search, no global lock, no syscall — three pointer
writes.

### 4.3 StackAllocator — markers for scope unwind

```
   marker = get_marker()     // offset_ = 80
   allocate(...)             // offset_ = 140
   allocate(...)             // offset_ = 200

   free_to_marker(marker)      // offset_ = 80  (last two allocs abandoned)
```

Individual `deallocate(ptr)` is a teaching stub; production LIFO stacks use
markers or walk headers backward.

### 4.4 FreeListAllocator — first-fit, split, coalesce

**Allocate 64 bytes from a 200-byte free block:**

```
   BEFORE:  free_list_ ──▶ [200 bytes]

   split:   [ in-use 64 ][ free remainder ~120 ] ──▶ free_list_
   return pointer after Header of the 64-byte piece
```

**Deallocate and coalesce adjacent holes:**

```
   [ free 100 ][ in-use 50 ][ free 80 ]  dealloc middle

   insert sorted by address → adjacent free blocks merge:

   [ free 100+hdr+50+hdr+80 ]  — one large hole for future big requests
```

---

## 5. API Reference

### LinearAllocator
| Call | Effect |
|---|---|
| `LinearAllocator(size)` | malloc one slab |
| `allocate(size, align)` | bump pointer; nullptr if full |
| `deallocate(ptr)` | no-op by design |
| `reset()` | `offset_ = 0`, reclaim all |
| `used()` / `available()` / `capacity()` | introspection |

### PoolAllocator
| Call | Effect |
|---|---|
| `PoolAllocator(block_size, count)` | carve slab, build free list |
| `allocate()` | pop block; nullptr if exhausted |
| `deallocate(ptr)` | push block back |
| `block_size()` / `num_blocks()` | pool geometry |

### StackAllocator
| Call | Effect |
|---|---|
| `StackAllocator(size)` | malloc slab |
| `allocate(size, align)` | bump with inline Header |
| `get_marker()` | snapshot `offset_` |
| `free_to_marker(m)` | rewind to marker |
| `reset()` | rewind to start |

### FreeListAllocator
| Call | Effect |
|---|---|
| `FreeListAllocator(size)` | one free block covering slab |
| `allocate(size)` | first-fit; may split |
| `deallocate(ptr)` | insert sorted; coalesce |
| `capacity()` | slab size |

---

## 6. Complexity Summary

| Operation | Linear | Pool | Stack | Free List |
|---|---|---|---|---|
| allocate | O(1) | O(1) | O(1) | O(n) free blocks |
| deallocate | N/A | O(1) | marker O(1) | O(n) insert + coalesce |
| bulk free | O(1) reset | — | O(1) marker | — |
| construct pool | O(1) | O(n) blocks | O(1) | O(1) |

---

## 7. Usage

```cpp
#include "allocator/allocator.hpp"

// Frame temps — reset once per tick
LinearAllocator frame(1024 * 1024);
float* particles = static_cast<float*>(frame.allocate(1000 * sizeof(float)));
frame.reset();

// Object pool — placement new + manual destroy
PoolAllocator pool(sizeof(Particle), 10000);
Particle* p = static_cast<Particle*>(pool.allocate());
new (p) Particle();
p->~Particle();
pool.deallocate(p);

// Scoped stack
StackAllocator stack(64 * 1024);
auto m = stack.get_marker();
int* data = static_cast<int*>(stack.allocate(100 * sizeof(int)));
stack.free_to_marker(m);

// General-purpose slab
FreeListAllocator heap(1024 * 1024);
void* a = heap.allocate(100);
void* b = heap.allocate(500);
heap.deallocate(a);  // any order
```

See [`allocator_example.cpp`](allocator_example.cpp) for frame simulation, pool
reuse, marker scopes, coalescing demos, and a malloc benchmark.

---

## 8. Design Decisions & Trade-offs

- **Intrusive metadata.** Pool free nodes and free-list headers live inside the
  slab, not in a separate global table — fewer cache misses, but blocks must be
  large enough to hold link pointers when free.
- **Pool block_size clamped to `sizeof(FreeNode)`.** Guarantees every free block
  can store a `next` pointer.
- **Free-list split threshold (`sizeof(Header) + 16`).** Avoids slivers too small
  to be useful on a future allocation.
- **No thread safety.** Simpler teaching code; concurrent use needs external
  locks or per-thread arenas.
- **Stack `deallocate` stub.** Markers are the supported bulk-free path; full LIFO
  per-pointer free would need backward header walks.

---

## 9. Common Pitfalls

- **Forgetting `reset()` on a linear allocator.** `offset_` monotonically grows;
  without reset you exhaust the slab even if objects are logically dead.
- **Pool size mismatch.** `PoolAllocator` cannot serve requests larger than
  `block_size_`; use `FreeListAllocator` or malloc instead.
- **Skipping destructors before reuse.** `reset()` and `deallocate()` reclaim raw
  bytes only — call `~T()` on live objects first.
- **Ignoring nullptr.** Every allocator returns nullptr on exhaustion; production
  code must handle OOM.
- **Using stack allocator out of LIFO order.** `free_to_marker` assumes later
  allocations have shorter lifetimes than the marker.

---

## 10. Comparison with `malloc` / `std::allocator`

**Same idea:** separate raw allocation from object construction (placement new).

**Intentionally simpler:** no thread caches, no mmap arenas, no security cookies,
no `std::allocator_traits` interface.

**When malloc still wins:** variable lifetimes, many sizes, cross-thread frees,
and unknown allocation patterns at compile time.

---

## 11. Build & Run

```bash
make run-allocator      # build + run the examples
make test-allocator     # build + run the unit tests
make all                # build everything in the repo
```

Manual compile from repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. allocator/allocator_example.cpp -o /tmp/x_allocator
```

---

## 12. See Also

- [`arena_allocator`](../arena_allocator/README.md) — per-thread bump allocators
- [`vector`](../vector/README.md) — contiguous container using `::operator new`
- [`thread_pool`](../thread_pool/README.md) — parallelism that benefits from fast
  per-task allocation
- [`locks`](../locks/README.md) — mutexes if you must share one allocator
