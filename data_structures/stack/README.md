# Stack

A **last-in, first-out** (LIFO) container: the most recently pushed element
is the first one popped. This version is backed by a growing array (like
`std::stack` often is under the hood).

The same interface can also be built on a dynamic array wrapper or a linked
list — the LIFO behaviour is what defines a stack, not the backing store.

## Picture

```
push 3, push 7, push 2:

    +---+
top | 2 |  <-- pop returns 2
    +---+
    | 7 |
    +---+
    | 3 |
    +---+
bottom
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `push` | O(1) amortized | doubles capacity when full |
| `pop` / `top` | O(1) | |
| `size` / `empty` | O(1) | |

## Usage

```cpp
#include "stack.hpp"

ds::Stack<int> s;
s.push(10);
s.push(20);
s.top();    // 20
s.pop();    // removes 20
```

## Build & run

```bash
make run-stack        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -Istack stack/stack.cpp -o demo && ./demo
```

## Things to watch

- `top()` on an empty stack is undefined — check `empty()` first.
- `pop()` does not return the value; read `top()` before popping if you need it.
- A linked-list stack avoids reallocation but pays per-node allocation overhead.
