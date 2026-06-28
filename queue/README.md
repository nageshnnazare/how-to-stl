# Queue — FIFO Container Adapter

> A `Queue<T>` exposes **First In, First Out** access on top of an underlying
> sequence (default `std::deque<T>`). New items enter at the **back**; the oldest
> item leaves from the **front**. Like a line at a counter — first arrival is
> first served.

This is a from-scratch reimplementation of `std::queue` built for learning. The
header is [`queue.hpp`](queue.hpp), runnable examples are in
[`queue_example.cpp`](queue_example.cpp), and the test suite is in
[`../tests/queue_test.cpp`](../tests/queue_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Kind | **Container adapter** |
| Default backend | `std::deque<T>` |
| Enqueue | `push` → `push_back` (rear) |
| Dequeue | `pop` → `pop_front` (front) |
| Peek | `front()` (oldest), `back()` (newest) |

**Reach for a Queue when** you schedule fair-first work (BFS, job queues, message
buffers, print spoolers).

**Look elsewhere when** you need LIFO ([`stack`](../stack/README.md)), priority
ordering ([`priority_queue`](../priority_queue/README.md)), or both-end deque ops
([`deque`](../deque/README.md)).

---

## 2. Mental Model

Two ends of the same underlying deque — **opposite** of stack:

```
   Queue                              deque c_
   ┌──────────┐              FRONT (dequeue)     BACK (enqueue)
   │ c_    ●──┼─────────────▶ [10] [20] [30] [40]
   └──────────┘                ▲                 ▲
                            pop()/front()    push()/back()
```

Flow of elements:

```
   arrivals ──push──▶  ... [10][20][30] ... ──pop──▶ service
              (rear)         FIFO order          (front)
```

After `push(10); push(20); push(30);` — repeated `pop()` yields `10`, `20`, `30`.

---

## 3. Internal Representation

```cpp
Container c_;  // default std::deque<T>
```

| Queue op | Underlying | End |
|---|---|---|
| `push(x)` | `push_back(x)` | back |
| `pop()` | `pop_front()` | front |
| `front()` | `front()` | front |
| `back()` | `back()` | back |

**Invariant:** elements in `c_` are logically ordered oldest→newest left→right.

---

## 4. How It Works (Step by Step)

### 4.1 Enqueue (`push`) — append at rear

```
   [10][20]  ──push(30)──▶  [10][20][30]
                              rear ────▲
```

### 4.2 Dequeue (`pop`) — remove from front

```
   [10][20][30]  ──pop()──▶  [20][30]
    ▲
  10 leaves first (FIFO)
```

### 4.3 Peek both ends

```
   front() → 10   (next out)
   back()  → 30   (last in, still waiting)
```

### 4.4 Why not `vector` as default?

`vector::pop_front()` is O(n) — every dequeue shifts the whole array. `deque`
gives O(1) at both ends with chunked storage.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Queue<T>()` | empty |
| `Queue<T>(container)` / move | wrap existing sequence |
| copy / move | copy or steal `c_` |

### Element access
`front()`, `back()` — const and non-const; UB if empty

### Capacity
`empty()`, `size()`

### Modifiers
`push`, `emplace`, `pop`, `swap`

### Non-member
Lexicographic `==`, `!=`, `<`, … and `swap`

---

## 6. Complexity Summary

| Operation | Complexity (deque) | Note |
|---|---|---|
| `push` | O(1) | back |
| `pop` | O(1) | front |
| `front`, `back` | O(1) | |
| `swap` | O(1) | |

With `vector` backend, `pop` degrades to O(n).

---

## 7. Usage

```cpp
#include "queue/queue.hpp"

Queue<int> q;
q.push(10);
q.push(20);
q.push(30);

std::cout << q.front();  // 10
q.pop();
std::cout << q.front();  // 20
```

See [`queue_example.cpp`](queue_example.cpp) for BFS, task scheduling, level-order
traversal, and Josephus simulation.

---

## 8. Design Decisions & Trade-offs

- **FIFO via two ends** — clearest teaching mapping; matches `std::queue`.
- **Default `deque`** — O(1) `pop_front`; list works too but worse cache locality.
- **`back()` exposed** — lets you inspect the newest waiting item without dequeuing.
- **No iteration** — queue abstraction hides the middle of the sequence.

---

## 9. Common Pitfalls

- **`pop` does not return the value** — read `front()` first.
- **Undefined behavior on empty `front`/`pop`.**
- **Using `vector` backend for high dequeue volume** — O(n) per pop kills performance.
- **Confusing `back()` with “next out”** — `front()` is the next to leave.

---

## 10. Comparison with `std::queue`

**Same:** adapter on default `deque`, FIFO API, `push` at back / `pop` at front.

**Aligned with STL:** no iterators, no direct access to underlying `c_` (we keep it
private like `std`).

---

## 11. Build & Run

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. queue/queue_example.cpp -o /tmp/x_queue
/tmp/x_queue

make run-queue    # if defined in Makefile
make test-queue
```

---

## 12. See Also

- [`stack`](../stack/README.md) — single-end LIFO adapter
- [`deque`](../deque/README.md) — default queue backend
- [`priority_queue`](../priority_queue/README.md) — not FIFO; highest priority first
