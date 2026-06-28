# Queue

A **first-in, first-out** (FIFO) container: enqueue adds at the back,
dequeue removes from the front. This version uses a **circular buffer** —
a fixed array with `head` and `tail` indices that wrap around at the end.

## Picture

```
capacity=8, size=4

index:  0    1    2    3    4    5    6    7
      +----+----+----+----+----+----+----+----+
      |    | 20 | 30 | 40 |    |    | 10 |    |
      +----+----+----+----+----+----+----+----+
             ^head          ^tail
             dequeue here   enqueue here
```

When `size == capacity`, the buffer **doubles** and the ring is unwrapped
into a contiguous layout.

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `enqueue` | O(1) amortized | grows when full |
| `dequeue` / `front` | O(1) | |
| `size` / `empty` | O(1) | |

## Usage

```cpp
#include "queue.hpp"

ds::Queue<int> q;
q.enqueue(10);
q.enqueue(20);
q.front();    // 10
q.dequeue();  // removes 10
```

## Build & run

```bash
make run-queue        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -Iqueue queue/queue.cpp -o demo && ./demo
```

## Things to watch

- `front()` on an empty queue is undefined — check `empty()` first.
- After growth, the ring is linearized (head reset to 0) to keep indexing simple.
- A linked-list queue avoids wraparound but allocates per element.
