# data_structures

Classic, undergraduate-style data structures and algorithms — the versions you
write in a CS course to *understand the mechanics*, not the STL-grade generic
containers in the rest of this repo.

Each structure is:

- **header-only** and templated where it makes sense, living in `namespace ds`;
- **heavily commented** with an ASCII diagram and a complexity table at the top;
- paired with a **runnable demo** (`<name>.cpp`) that prints a guided tour and
  then runs `assert`-based self-checks (`All self-checks passed.`);
- documented by a short **README** (idea, picture, operations, pitfalls).

> Looking for the production-style, STL-compatible versions? See the component
> folders at the [repo root](../README.md) (`vector/`, `map/`, `deque/`, …).
> This folder is the "from scratch, for learning" companion.

## Catalog

| Structure | Folder | Backing idea | Highlights |
|---|---|---|---|
| Dynamic array | [`dynamic_array`](dynamic_array/) | contiguous buffer, doubling | amortized O(1) `push_back` |
| Singly linked list | [`singly_linked_list`](singly_linked_list/) | nodes + `head` | O(1) push_front, `reverse()` |
| Doubly linked list | [`doubly_linked_list`](doubly_linked_list/) | nodes + `head`/`tail` | O(1) at both ends |
| Stack | [`stack`](stack/) | growing array | LIFO: push / pop / top |
| Queue | [`queue`](queue/) | circular buffer | FIFO with wraparound |
| Deque | [`deque`](deque/) | circular buffer | push/pop at both ends |
| Binary search tree | [`binary_search_tree`](binary_search_tree/) | ordered tree | insert / erase / in-order |
| AVL tree | [`avl_tree`](avl_tree/) | self-balancing BST | LL/RR/LR/RL rotations |
| Binary heap | [`binary_heap`](binary_heap/) | array-as-tree | priority queue + heapsort |
| Trie | [`trie`](trie/) | 26-way prefix tree | `insert` / `starts_with` |
| Hash table | [`hash_table`](hash_table/) | separate chaining | load factor + rehash |
| Graph | [`graph`](graph/) | adjacency list | BFS / DFS / Dijkstra / topo-sort |
| Union-Find | [`union_find`](union_find/) | disjoint-set forest | path compression + union by rank |
| LRU cache | [`lru_cache`](lru_cache/) | hash map + doubly linked list | O(1) get/put, evicts least-recently-used |

## Quick start

```bash
cd data_structures

make            # build every demo into build/
make run        # build + run every demo (each ends in "All self-checks passed.")
make run-graph  # build + run a single demo (any structure name)
make clean
make help
```

Or compile one by hand:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -O2 -I. graph/graph.cpp -o demo && ./demo
```

## Complexity cheat sheet

| Structure | Access | Search | Insert | Delete | Notes |
|---|---|---|---|---|---|
| Dynamic array | O(1) | O(n) | O(1)* / O(n) | O(1)* / O(n) | *amortized at end; O(n) in middle |
| Singly linked list | O(n) | O(n) | O(1) front | O(1) front | no random access |
| Doubly linked list | O(n) | O(n) | O(1) ends | O(1) ends | two pointers per node |
| Stack | O(1) top | — | O(1) | O(1) | LIFO only |
| Queue | O(1) ends | — | O(1) | O(1) | FIFO only |
| Deque | O(1) ends | — | O(1) ends | O(1) ends | both ends |
| BST (balanced) | — | O(log n) | O(log n) | O(log n) | O(n) worst if skewed |
| AVL tree | — | O(log n) | O(log n) | O(log n) | height stays ~1.44 log n |
| Binary heap | O(1) top | O(n) | O(log n) | O(log n) | only the extreme is cheap |
| Trie | — | O(L) | O(L) | O(L) | L = key length |
| Hash table | — | O(1) avg | O(1) avg | O(1) avg | O(n) worst (all collide) |
| Union-Find | — | — | O(α(n)) | — | α = inverse Ackermann ≈ 1 |
| LRU cache | O(1) avg | O(1) avg | O(1) avg | O(1) avg | map + list; evicts LRU when full |

## Suggested reading order

1. `dynamic_array` → `singly_linked_list` → `doubly_linked_list` (storage basics)
2. `stack` → `queue` → `deque` (restricted-access linear structures)
3. `binary_search_tree` → `avl_tree` → `binary_heap` (trees & ordering)
4. `trie` → `hash_table` (keyed lookup)
5. `graph` → `union_find` (relationships & connectivity)
6. `lru_cache` (combining a hash map + doubly linked list into one design)
