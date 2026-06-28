# Thread Pool — Task-Based Parallelism

> A `ThreadPool` keeps N worker threads alive, blocking on a shared task queue until
> work arrives. Call `submit()` to enqueue a job and get a `std::future`; workers
> pop tasks under a mutex, run them outside the lock, and increment statistics.
> Shutdown sets a stop flag and joins every worker after the queue drains.

This is a from-scratch reimplementation of a classic thread pool built for learning.
The header is [`thread_pool.hpp`](thread_pool.hpp), runnable examples are in
[`thread_pool_example.cpp`](thread_pool_example.cpp), and the test suite is in
[`../tests/thread_pool_test.cpp`](../tests/thread_pool_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Workers | Fixed count (default `hardware_concurrency()`) |
| Queue | FIFO `std::queue<std::function<void()>>` |
| Synchronization | `mutex_` + `condition_variable` |
| Results | `submit()` → `std::future` via `packaged_task` |
| Fire-and-forget | `execute()` (no future) |
| Shutdown | `stop_` flag + `notify_all` + `join` |
| Extras | `ParallelFor`, `TaskQueue`, `WorkStealingThreadPool` |

**Reach for a thread pool when** you have many medium-grained tasks and creating
a thread per task would thrash the OS scheduler.

**Look elsewhere when** tasks are tiny (< few μs), work is I/O-bound (async I/O),
or you only need one-off parallelism (`std::async` may suffice).

---

## 2. Mental Model

```
   Producers (any thread)              ThreadPool                    Workers
        │                                  │                            │
        ├─ submit(f) ──▶ lock ──▶ push ─────┼── notify_one ─────────────▶│ W0
        │                                  │ tasks_[..]                 ├─ pop, unlock
        ├─ execute(g) ──▶ push ────────────┤                            ├─ run g()
        │                                  │                     notify ─▶│ W1..WN
        └─ future.get() ◀── packaged_task sets result ────────────────────┘
```

- **Enqueue** happens under `mutex_` (fast).
- **Execute** happens without `mutex_` (slow user code must not block peers).
- **`condition_`** parks idle workers until `stop_ || !tasks_.empty()`.

---

## 3. Internal Representation

```cpp
std::vector<std::thread>              workers_;           // OS threads
std::queue<std::function<void()>>     tasks_;             // pending FIFO work
std::mutex                            mutex_;             // queue + wait predicate
std::condition_variable               condition_;         // sleep / wake
std::atomic<bool>                     stop_;              // shutdown requested
std::atomic<size_t>                   active_workers_;    // in-flight tasks
std::atomic<size_t>                   completed_tasks_;   // lifetime counter
```

**Invariant:** only `worker_thread()` pops `tasks_`; producers only push.

### WorkStealingThreadPool (alternate)

```cpp
struct Worker {
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::thread thread;
};
// submit round-robins; idle workers try_to_lock and steal from others
```

---

## 4. How It Works (Step by Step)

### 4.1 Worker lifecycle

```
   worker loop:
       lock(mutex_)
       wait(cv) until stop_ || !tasks_.empty()

       if stop_ && tasks_.empty():  EXIT thread

       task = move(tasks_.front()); pop
       unlock

       active_workers_++
       try { task(); } catch(...) { swallow — future holds exception }
       active_workers_--
       completed_tasks_++
```

Swallowing in the worker keeps the pool alive; `packaged_task` stores exceptions
into the `future` for `submit()` callers.

### 4.2 submit() with futures

```
   (1) packaged_task<ReturnType> bound to f(args...)
   (2) future = task.get_future()
   (3) enqueue lambda: [task]{ (*task)(); }
   (4) notify_one()

   caller thread                         worker thread
        │                                      │
        ├─ returns future immediately          ├─ runs lambda
        ├─ future.get() blocks ────────────────┼─ sets value/exception
```

`shared_ptr` on the packaged_task keeps it alive until the worker runs.

### 4.3 Graceful shutdown

```
   shutdown():
       stop_ = true
       notify_all()     // wake every sleeper

   each worker:
       wake → queue empty && stop_ → return from worker_thread()

   join all workers_; clear vector
```

Destructor calls `shutdown()` — do not destroy the pool while tasks still need
the pool object unless they are finished.

### 4.4 wait() for quiescence

```
   spin:
       lock; if tasks_.empty() && active_workers_==0: done
       unlock; yield
```

Useful after `execute()` bursts; does not stop new `submit()` from other threads.

### 4.5 Work stealing (optional pool)

```
   Worker i idle:
       try own queue
       else for j != i: try_lock(workers_[j]) and steal front task
       else yield
```

Less global queue contention; more complex; destructor does not drain tasks.

---

## 5. API Reference

### ThreadPool
| Call | Effect |
|---|---|
| `ThreadPool(n)` | spawn n workers |
| `submit(f, args...)` | enqueue; return `future` |
| `execute(f)` | enqueue without future |
| `wait()` | until queue empty and no active workers |
| `shutdown()` | stop flag + join |
| `num_workers()` | worker count |
| `queued_tasks()` | snapshot queue size |
| `active_workers()` | in-flight count |
| `completed_tasks()` | finished count |
| `is_stopped()` | shutdown flag |

### ParallelFor
| Call | Effect |
|---|---|
| `execute(start, end, func)` | chunk range across `submit` |

### TaskQueue\<T\>
| Call | Effect |
|---|---|
| `push(v, priority)` | higher priority first |
| `pop(v)` | block; false on stop+empty |
| `stop()` | wake waiters |

### WorkStealingThreadPool
| Call | Effect |
|---|---|
| `submit(f)` | round-robin to worker queue |
| `num_workers()` | worker count |

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `submit` / `execute` | O(1) | queue push + notify |
| worker pop | O(1) | FIFO pop |
| task execution | depends on user | runs outside mutex |
| `wait` | O(ready spins) | polls queue + active count |
| `shutdown` | O(workers) | join each thread |
| `ParallelFor` | O(workers) submits | + user func cost |

---

## 7. Usage

```cpp
#include "thread_pool/thread_pool.hpp"

ThreadPool pool(4);

auto fut = pool.submit([](int a, int b) { return a + b; }, 10, 20);
int sum = fut.get();

pool.execute([&]() { /* fire and forget */ });
pool.wait();

ParallelFor pfor(pool);
std::vector<int> data(1000);
pfor.execute(0, data.size(), [&](size_t i) { data[i] = i * i; });

pool.shutdown();  // optional — destructor does this
```

See [`thread_pool_example.cpp`](thread_pool_example.cpp) for futures, parallel sum,
`ParallelFor`, and fire-and-forget counters.

---

## 8. Design Decisions & Trade-offs

- **Single FIFO queue.** Simple and correct; can contend under many producers.
  `WorkStealingThreadPool` trades complexity for per-worker queues.
- **Execute outside mutex.** Required for throughput and re-entrant `submit()`.
- **Exception in worker swallowed.** Pool survives; `submit` futures still receive
  `exception_ptr` via `packaged_task`.
- **`notify_one` on enqueue.** Wakes one sleeper; thundering herd avoided vs `notify_all`.
- **No task cancellation.** Once enqueued, task runs unless process exits.
- **C++14 `std::result_of`.** Matches codebase standard; C++17 would use `invoke_result`.

---

## 9. Common Pitfalls

- **Capturing references in tasks.** `submit([&x](){...})` dangles if `x` leaves scope
  before `future.get()` — capture by value or `shared_ptr`.
- **Deadlock via nested wait.** Task waits on a future that only the same pool can
  run → starvation if all workers blocked (use separate pool or don't block inside tasks).
- **Destroying pool with pending work.** Destructor shuts down and joins, but objects
  captured by tasks must outlive execution.
- **`wait()` vs `shutdown()`.** `wait()` is quiescence snapshot; does not prevent new
  submissions from other threads.
- **Tiny tasks.** Submission overhead (~μs) dominates if body is nanoseconds — batch work.

---

## 10. Comparison with `std::async` / `std::thread`

**vs `std::async`:** pool reuses threads and caps concurrency; `async` may spawn new
threads unpredictably (implementation-defined).

**vs manual `std::thread` per task:** pool amortizes thread creation and limits load.

**Not a full executor:** no priorities in main `ThreadPool`, no continuation chains,
no integration with coroutines.

---

## 11. Build & Run

```bash
make run-thread-pool       # build + run the examples
make test-thread-pool      # build + run the unit tests
make all                   # build everything in the repo
```

Manual compile from repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. \
    thread_pool/thread_pool_example.cpp -o /tmp/x_thread_pool
```

---

## 12. See Also

- [`locks`](../locks/README.md) — mutex and condition_variable primitives used here
- [`arena_allocator`](../arena_allocator/README.md) — fast per-thread alloc for task scratch
- [`allocator`](../allocator/README.md) — general allocation strategies
- [`vector`](../vector/README.md) — containers often processed via `ParallelFor`
