# Singly Linked List

A chain of nodes where each node holds a value and a pointer to the next node.
The list keeps a `head` pointer to the first node (or `nullptr` when empty).
There is no random access — reaching index *i* means walking *i* links.

## Picture

```
head
  |
  v
+---+---+     +---+---+     +---+----+
| a | * | --> | b | * | --> | c |null|
+---+---+     +---+---+     +---+----+
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `push_front` / `pop_front` | O(1) | update head |
| `push_back` | O(N) | walk to tail (add a tail ptr for O(1)) |
| `front` / `size` / `empty` | O(1) | |
| `find` / `remove` | O(N) | linear scan |
| `reverse` | O(N) | rewire next pointers |
| iteration | O(N) | forward only |

## Usage

```cpp
#include "singly_linked_list.hpp"

ds::SinglyLinkedList<int> list;
list.push_back(10);
list.push_front(5);    // 5 -> 10
list.remove(10);
for (int x : list) std::cout << x << ' ';
```

## Build & run

```bash
make run-singly_linked_list        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -Isingly_linked_list singly_linked_list/singly_linked_list.cpp -o demo && ./demo
```

## Things to watch

- `push_back` is O(N) without a tail pointer; a doubly-linked list with head
  and tail makes both ends O(1).
- Iterators are invalidated by any structural change (insert, remove, reverse).
- Each node is a separate heap allocation — more overhead than a contiguous array.
