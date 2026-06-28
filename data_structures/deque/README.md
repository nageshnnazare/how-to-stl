# Deque

A **double-ended queue**: push and pop at both the front and the back in O(1).
This version uses a **circular buffer** with `head` and `tail` indices that
wrap around, and doubles capacity when full.

## Picture

```
capacity=8, size=4

index:  0    1    2    3    4    5    6    7
      +----+----+----+----+----+----+----+----+
      |    | 20 | 30 | 40 |    |    | 10 |    |
      +----+----+----+----+----+----+----+----+
             ^head          ^tail
             front=20       back=40
```

`push_front` moves `head` left (with wrap); `push_back` writes at `tail`
and advances it.

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `push_front` / `push_back` | O(1) amortized | grows when full |
| `pop_front` / `pop_back` | O(1) | |
| `front` / `back` / `operator[]` | O(1) | `[]` is logical index from front |
| `size` / `empty` | O(1) | |

## Usage

```cpp
#include "deque.hpp"

ds::Deque<int> d;
d.push_back(20);
d.push_front(10);   // 10, 20
d.pop_back();
for (std::size_t i = 0; i < d.size(); ++i) std::cout << d[i] << ' ';
```

## Build & run

```bash
make run-deque        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -Ideque deque/deque.cpp -o demo && ./demo
```

## Things to watch

- `operator[]` uses logical indices (0 = front), not raw buffer positions.
- `front()` / `back()` on an empty deque are undefined — check `empty()` first.
- `std::deque` typically uses multiple fixed-size blocks; this teaching
  version uses one ring buffer for simplicity.
