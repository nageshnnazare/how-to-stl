# Stack — LIFO Container Adapter

> A `Stack<T>` exposes **Last In, First Out** access on top of an underlying
> sequence (default `std::deque<T>`). You only ever touch the **top** — push adds
> there, pop removes there, `top()` reads there. Everything else is hidden.

This is a from-scratch reimplementation of `std::stack` built for learning. The
header is [`stack.hpp`](stack.hpp), runnable examples are in
[`stack_example.cpp`](stack_example.cpp), and the test suite is in
[`../tests/stack_test.cpp`](../tests/stack_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Kind | **Container adapter** (not a standalone allocator) |
| Default backend | `std::deque<T>` |
| Access | **Top only** (maps to `back()` of underlying sequence) |
| Order | LIFO — last pushed is first popped |
| Iteration | **None** (by design) |

**Reach for a Stack when** you need undo history, DFS, bracket matching, expression
evaluation, or any algorithm that walks backward through recent choices.

**Look elsewhere when** you need FIFO ([`queue`](../queue/README.md)), priority
([`priority_queue`](../priority_queue/README.md)), or random access ([`vector`](../vector/README.md)).

---

## 2. Mental Model

The adapter owns one field `c_`. The **top** of the stack is the **back** of `c_`:

```
   Stack                         underlying deque c_
   ┌──────────┐                 front                    back = TOP
   │ c_    ●──┼────────────────▶ [10] [20] [30] [40]
   └──────────┘                                      ▲
                                              push / pop / top
```

Vertical LIFO picture:

```
        push 40 ──▶  ┌─────┐
        push 30 ──▶  │ 40  │  ← top
        push 20 ──▶  │ 30  │
        push 10 ──▶  │ 20  │
                       │ 10  │
                       └─────┘
        pop() → 40, then 30, then 20 …
```

---

## 3. Internal Representation

```cpp
Container c_;  // default std::deque<T>
```

**Mapping (fixed by this implementation):**

| Stack op | Underlying call | End |
|---|---|---|
| `push(x)` | `c_.push_back(x)` | back (top) |
| `pop()` | `c_.pop_back()` | back |
| `top()` | `c_.back()` | back |

No extra metadata — stack size is `c_.size()`.

---

## 4. How It Works (Step by Step)

### 4.1 Push — grow at top (back)

```
   push(30) on [10, 20]:

   [10][20]  ──push_back(30)──▶  [10][20][30]
                                    top ────▲
```

O(1) amortized with deque or vector backend.

### 4.2 Pop — shrink at top (does not return value)

```
   pop() on [10][20][30]:

   [10][20][30]  ──pop_back()──▶  [10][20]
                    top was 30
```

Call `top()` before `pop()` if you need the value. Undefined if empty.

### 4.3 Top — read without mutation

```
   top() → reference to c_.back()
```

### 4.4 Custom backend (`Stack<T, std::vector<T>>`)

Same API; **top** is still `vector::back()`. Vector may reallocate on growth
(invalidating references); deque avoids that — why deque is the default.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Stack<T>()` | empty deque |
| `Stack<T>(container)` | copy-wrap existing sequence |
| `Stack<T>(std::move(container))` | move-wrap |
| copy / move ctor, assignment | deep copy / steal `c_` |

### Element access
| Call | Notes |
|---|---|
| `top()` | mutable / const; UB if empty |

### Capacity
`empty()`, `size()`

### Modifiers
`push`, `emplace`, `pop`, `swap`

### Non-member
`==`, `!=`, `<`, `<=`, `>`, `>=` (lexicographic on `c_`), free `swap`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `push` | O(1) amortized | deque/vector back insert |
| `pop` | O(1) | back erase |
| `top` | O(1) | |
| `empty`, `size` | O(1) | |
| `swap` | O(1) | swaps underlying containers |

---

## 7. Usage

```cpp
#include "stack/stack.hpp"

Stack<int> s;
s.push(10);
s.push(20);
s.push(30);

std::cout << s.top();  // 30
s.pop();
std::cout << s.top();  // 20

// Vector backend (still LIFO at back)
Stack<int, std::vector<int>> sv;
sv.push(1);
```

See [`stack_example.cpp`](stack_example.cpp) for parentheses checking, postfix
evaluation, DFS, undo/redo, and min-stack patterns.

---

## 8. Design Decisions & Trade-offs

- **Adapter, not a linked list stack.** Reuses battle-tested `deque` for O(1) ends
  and cache-friendly blocks — same design as libstdc++/libc++ `std::stack`.
- **Top = back.** Matches STL; all mutations at one end.
- **No `pop` return value.** STL separates `top` and `pop` for exception safety
  when `T` is expensive to copy.
- **Comparison operators** delegate to underlying container — useful for tests.

---

## 9. Common Pitfalls

- **Pop on empty** — undefined behavior; check `empty()` first.
- **Stale references after `push`** — if using `vector` backend, reallocation may
  invalidate references to elements; deque is safer.
- **Using stack when you need FIFO** — you will process items in reverse order.
- **`top()` on empty** — UB, same as `std::stack`.

---

## 10. Comparison with `std::stack`

**Same:** adapter pattern, default `deque`, LIFO API, `push`/`pop`/`top`/`emplace`.

**Same spirit:** no iterators, no size guarantees beyond underlying container.

This teaching build adds **full comparison operators**; some STLs provide only `==`
in C++20 and earlier.

---

## 11. Build & Run

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. stack/stack_example.cpp -o /tmp/x_stack
/tmp/x_stack

# If your Makefile defines them:
make run-stack
make test-stack
```

---

## 12. See Also

- [`queue`](../queue/README.md) — FIFO adapter (opposite ends)
- [`deque`](../deque/README.md) — default stack backend
- [`vector`](../vector/README.md) — alternative stack backend
- [`priority_queue`](../priority_queue/README.md) — heap adapter, not LIFO
