# Doubly Linked List

Like a singly-linked list, but each node also points to its *previous* node.
The list keeps both `head` and `tail`, so inserts and removals at either end
are O(1).

## Picture

```
nullptr <--+---+---+     +---+---+     +---+---+
           |prev| a |next|<->|prev| b |next|<->|prev| c |next|--> nullptr
           +---+---+     +---+---+     +---+---+
head --------^                         tail --^
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `push_front` / `push_back` | O(1) | splice at head or tail |
| `pop_front` / `pop_back` | O(1) | unlink head or tail |
| `front` / `back` / `size` / `empty` | O(1) | |
| iteration (forward) | O(N) | walk via `next` |

## Usage

```cpp
#include "doubly_linked_list.hpp"

ds::DoublyLinkedList<int> list;
list.push_back(20);
list.push_front(10);   // 10 <-> 20
list.pop_back();
for (int x : list) std::cout << x << ' ';
```

## Build & run

```bash
make run-doubly_linked_list        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -Idoubly_linked_list doubly_linked_list/doubly_linked_list.cpp -o demo && ./demo
```

## Things to watch

- Two pointers per node (prev + next) — more memory than a singly-linked list.
- Always update *both* `head` and `tail` when the list becomes empty.
- Bidirectional traversal is possible, but this implementation only exposes
  forward iteration.
