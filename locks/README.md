# Locks — Thread Synchronization Primitives

> Five hand-rolled locks from spinlocks to recursive reader/writer guards. Each
> one shows the atomics, waiting, and RAII patterns behind `std::mutex` and
> `std::shared_mutex` — spin for micro-critical sections, CAS pointers for
> lock-free stacks, RW locks when reads dominate.

This is a from-scratch reimplementation of common synchronization patterns built
for learning. The header is [`locks.hpp`](locks.hpp), runnable examples are in
[`locks_example.cpp`](locks_example.cpp), and the test suite is in
[`../tests/locks_test.cpp`](../tests/locks_test.cpp).

---

## 1. What It Is

| Primitive | Exclusivity | Blocks? | Reentrant? | Best for |
|---|---|---|---|---|
| `MutexLock` | exclusive | spin + yield | ❌ | tiny critical sections |
| `PointerLock<T>` | lock-free CAS | retry loops | N/A | lock-free stack head |
| `ReadWriteLock` | shared read / exclusive write | condition_variable | ❌ | read-heavy caches |
| `RecursiveRWLock` | RW + per-thread depth | condition_variable | ✅ | nested calls |
| `RecursiveRWPointerLock<T>` | RW around `T*` | via recursive RW | ✅ | guarded shared pointer |

**Reach for these types when** you want to see exactly how acquire/release,
test-and-set, and reader counts work — not when you need production fairness or
`std::shared_mutex` platform tuning.

**Look elsewhere when** critical sections are long (prefer blocking `std::mutex`),
fair scheduling matters, or you need standard Thread Sanitizer annotations.

---

## 2. Mental Model

### Spinlock contention

```
   Thread A                         Thread B
      │ lock(): TAS → OK               │ lock(): TAS → busy
      │ [ counter++ ]                  │   yield → spin → retry
      │ unlock(): clear                │
      │                                │ lock(): TAS → OK
```

### Read-write lock

```
   readers_ >= 0  →  N readers in parallel
   readers_ = WRITER_FLAG  →  one writer, everyone else waits on cv_
```

### RAII guard (all mutex-like types)

```
   { Guard g(lock);     // lock()
       ...               // critical section
   }                     // ~Guard → unlock()  even on throw
```

---

## 3. Internal Representation

### MutexLock

```cpp
std::atomic_flag flag_ = ATOMIC_FLAG_INIT;  // test_and_set / clear
```

### PointerLock\<T\>

```cpp
std::atomic<T*>      ptr_;
std::atomic<uint64_t> version_;   // incremented on successful mutation
```

### ReadWriteLock

```cpp
std::atomic<int32_t> readers_;   // count, or WRITER_FLAG (-1000000)
std::mutex           mutex_;     // for condition_variable
std::condition_variable cv_;
```

### RecursiveRWLock

```cpp
std::mutex mutex_;
std::condition_variable cv_;
std::thread::id writer_id_;
int32_t write_count_;
int32_t reader_count_;
std::unordered_map<std::thread::id, int32_t> read_counts_;
```

### RecursiveRWPointerLock\<T\>

```cpp
T* ptr_;                         // guarded data (lock_ required for access)
RecursiveRWLock lock_;
std::atomic<uint64_t> version_;
```

---

## 4. How It Works (Step by Step)

### 4.1 MutexLock — test-and-set spin loop

```
   lock():
       while flag_.test_and_set(acquire):   // returns previous value
           yield()                          // previous was SET → spin

   unlock():
       flag_.clear(release)                 // publish critical section end
```

`memory_order_acquire` on lock prevents reads in the critical section from
hoisting before the lock. `release` on unlock prevents writes from leaking after.

### 4.2 PointerLock — CAS push on a lock-free stack

```
   push(node):
       loop:
           old = head.load()
           node->next = old
           if CAS(head, old, node): version++; return
           // else retry — another thread won the race
```

Version increments teach ABA detection; production code often packs pointer+tag
in 128-bit CAS.

### 4.3 ReadWriteLock — reader and writer paths

**Reader:**

```
   unique_lock(mutex_)
   wait until readers_ >= 0        // no active writer
   readers_.fetch_add(1)
```

**Writer:**

```
   wait until readers_ == 0
   readers_ = WRITER_FLAG
```

**Last reader unlock:**

```
   if fetch_sub(1) == 1: notify_all()   // wake blocked writer
```

### 4.4 RecursiveRWLock — nested acquire same thread

```
   Thread T already holds write (writer_id_=T, write_count_=2)
   T calls lock_write() again → write_count_ becomes 3 (no wait)

   Different thread U calls lock_write() → waits on cv_ until T drops to 0
```

### 4.5 RecursiveRWPointerLock — RAII read/write facades

```
   auto r = account.read();    // ReadGuard lives inside ReadAccess
   double b = r->balance;      // shared with other readers

   auto w = account.write();   // WriteGuard inside WriteAccess
   w->deposit(10);
   w.set(new_account);         // ptr swap + version++
```

---

## 5. API Reference

### MutexLock
| Call | Effect |
|---|---|
| `lock()` | spin until acquired |
| `try_lock()` | non-blocking attempt |
| `unlock()` | release |
| `Guard(m)` | RAII lock |

### PointerLock\<T\>
| Call | Effect |
|---|---|
| `load(order)` | atomic read of ptr_ |
| `store(ptr)` | store + version++ |
| `compare_exchange_weak/strong` | CAS + version on success |
| `version()` | observation counter |

### ReadWriteLock
| Call | Effect |
|---|---|
| `lock_read` / `unlock_read` | shared access |
| `lock_write` / `unlock_write` | exclusive access |
| `try_lock_read/write` | non-blocking |
| `ReadGuard` / `WriteGuard` | RAII |

### RecursiveRWLock
| Call | Effect |
|---|---|
| `lock_read` / `unlock_read` | reentrant shared |
| `lock_write` / `unlock_write` | reentrant exclusive |
| `ReadGuard` / `WriteGuard` | RAII |

### RecursiveRWPointerLock\<T\>
| Call | Effect |
|---|---|
| `read()` | `ReadAccess` with shared lock |
| `write()` | `WriteAccess` with exclusive lock |
| `version()` | pointer change generation |

---

## 6. Complexity Summary

| Operation | MutexLock | PointerLock CAS | ReadWriteLock | RecursiveRWLock |
|---|---|---|---|---|
| uncontended lock | O(1) | O(1) | O(1) + possible wait | O(1) + map touch |
| contended lock | O(spin) | O(retries) | O(park/wake) | O(park/wake) |
| unlock | O(1) | N/A | O(1); maybe notify | O(1); maybe notify |

---

## 7. Usage

```cpp
#include "locks/locks.hpp"

// Spinlock + RAII
MutexLock m;
int counter = 0;
{
    MutexLock::Guard g(m);
    ++counter;
}

// Lock-free stack head
PointerLock<Node> head;
// ... CAS push/pop loop in example ...

// Read-heavy config
ReadWriteLock rw;
{
    ReadWriteLock::ReadGuard g(rw);
    read_config();
}
{
    ReadWriteLock::WriteGuard g(rw);
    update_config();
}

// Nested recursive write
RecursiveRWLock rr;
RecursiveRWLock::WriteGuard outer(rr);
RecursiveRWLock::WriteGuard inner(rr);  // same thread — OK
```

See [`locks_example.cpp`](locks_example.cpp) for counter stress tests, lock-free
stack, database simulation, nested recursive locks, and `MutexLock` vs `std::mutex`
timing.

---

## 8. Design Decisions & Trade-offs

- **Spin + yield for MutexLock.** Fast when hold time is nanoseconds; wastes CPU
  when contention is high or sections are long — use `std::mutex` then.
- **Separate version atomics in PointerLock.** Simpler than 128-bit tagged pointers
  on all platforms; not a complete hazard-pointer implementation.
- **Writer sentinel in ReadWriteLock.** One atomic `readers_` field instead of
  separate reader/writer flags; writers can starve under constant readers.
- **RecursiveRWLock map.** Per-thread read depth costs heap lookups; fine for teaching,
  heavy for millions of threads.
- **RAII guards everywhere.** Mirrors `std::lock_guard` — unlock on every exit path.

---

## 9. Common Pitfalls

- **Manual lock without RAII.** An early `return` or exception skips `unlock()` → deadlock.
- **Long work under spinlock.** Holding `MutexLock` across I/O burns CPU for all waiters.
- **Write lock for reads.** Blocks other readers unnecessarily — use `ReadGuard`.
- **Lock ordering inversion.** Thread 1: `A` then `B`; Thread 2: `B` then `A` → deadlock.
- **ABA without versioning.** Raw `atomic<T*>` CAS can succeed spuriously after complex
  pop/push cycles — `PointerLock` increments `version_` on each change.

---

## 10. Comparison with `std::mutex` / `std::shared_mutex`

**Same ideas:** mutual exclusion, shared/exclusive phases, RAII guards.

**Intentionally different:** `MutexLock` spins instead of parking; `ReadWriteLock` uses
a reader-preference policy without OS-specific rwlock; no `timed_mutex`, no `shared_lock`
typedef aliases.

**When std wins:** long critical sections, fairness, integration with `condition_variable`
against standard mutex types, TSan annotations.

---

## 11. Build & Run

```bash
make run-locks          # build + run the examples
make test-locks         # build + run the unit tests
make all                # build everything in the repo
```

Manual compile from repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. locks/locks_example.cpp -o /tmp/x_locks
```

---

## 12. See Also

- [`thread_pool`](../thread_pool/README.md) — mutex + condition_variable on task queue
- [`arena_allocator`](../arena_allocator/README.md) — mutex on arena map
- [`allocator`](../allocator/README.md) — single-thread allocators (no locks)
