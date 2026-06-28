# how-to-stl — The C++ Standard Library, Rebuilt from Scratch to Learn How It Works

This repository reimplements **24 C++ standard-library components** — every major
container, both smart pointers, and four systems-level building blocks — in clean,
heavily-commented, header-only C++. It is a *teaching* codebase: every header
opens with an ASCII memory-layout diagram, every clever algorithm (Red-Black
rotations, hash rehashing, heap sift, Small String Optimization, ref-counting…)
is explained step by step, and every component has its own deep-dive README.

> **New here? Start with the [cross-cutting concepts guide](docs/README.md).** It
> explains the one memory pattern that underpins almost every container, plus a
> "which container should I use?" decision tree and a full complexity matrix.

---

## Catalog

Each component is self-contained in its own folder: `<name>/<name>.hpp` (the
implementation), `<name>/<name>_example.cpp` (a runnable tour), and
`<name>/README.md` (the deep dive). Tests live in [`tests/`](tests/README.md).

### Smart pointers
| Component | What it teaches | Deep dive |
|---|---|---|
| `unique_ptr` | exclusive ownership, move-only, RAII, custom deleters, EBO | [README](unique_ptr/README.md) |
| `shared_ptr` | reference counting, control blocks, `weak_ptr`, atomics | [README](shared_ptr/README.md) |

### Sequence containers
| Component | What it teaches | Deep dive |
|---|---|---|
| `array` | fixed-size, stack-allocated, zero overhead | [README](array/README.md) |
| `vector` | contiguous storage, geometric growth, amortized analysis | [README](vector/README.md) |
| `string` | Small String Optimization (inline vs heap buffer) | [README](string/README.md) |
| `deque` | chunked storage, O(1) at both ends | [README](deque/README.md) |
| `list` | doubly-linked nodes, O(1) splice | [README](list/README.md) |

### Associative containers — ordered (Red-Black tree / sorted)
| Component | What it teaches | Deep dive |
|---|---|---|
| `set` | Red-Black tree, rotations, insert/delete fixups | [README](set/README.md) |
| `map` | Red-Black tree storing key→value nodes | [README](map/README.md) |
| `multiset` | sorted storage with duplicates, `equal_range` | [README](multiset/README.md) |
| `multimap` | sorted key→value with duplicate keys | [README](multimap/README.md) |

### Associative containers — unordered (hash table)
| Component | What it teaches | Deep dive |
|---|---|---|
| `unordered_set` | separate chaining, load factor, rehashing | [README](unordered_set/README.md) |
| `unordered_map` | hashed key→value, collision chains | [README](unordered_map/README.md) |

### Container adapters
| Component | What it teaches | Deep dive |
|---|---|---|
| `stack` | LIFO adapter over an underlying container | [README](stack/README.md) |
| `queue` | FIFO adapter | [README](queue/README.md) |
| `priority_queue` | binary heap, sift-up / sift-down | [README](priority_queue/README.md) |

### Utility types
| Component | What it teaches | Deep dive |
|---|---|---|
| `pair` | two-field aggregate, structured bindings | [README](pair/README.md) |
| `tuple` | variadic templates, compile-time `get<I>` | [README](tuple/README.md) |
| `optional` | aligned storage, engaged/disengaged states | [README](optional/README.md) |
| `bitset` | word-packed bits, bit/word index math | [README](bitset/README.md) |

### Systems-level building blocks
| Component | What it teaches | Deep dive |
|---|---|---|
| `allocator` | pool / free-list allocation | [README](allocator/README.md) |
| `arena_allocator` | bump-pointer (linear) allocation | [README](arena_allocator/README.md) |
| `locks` | spinlocks, RAII guards, reader/writer locks | [README](locks/README.md) |
| `thread_pool` | worker threads, task queue, futures | [README](thread_pool/README.md) |

---

## Companion: data structures from scratch

Alongside the STL re-implementations above, [`data_structures/`](data_structures/README.md)
holds the **undergraduate-style** versions — the from-scratch dynamic array,
linked lists, stack/queue/deque, BST, AVL tree, binary heap, trie, hash table,
graph (BFS/DFS/Dijkstra/topological sort), union-find, and an LRU cache you'd
write in a CS course to learn the mechanics. Each comes with an ASCII-diagrammed header, a
runnable self-checking demo, and a short README.

```bash
cd data_structures && make run     # build + run every demo
```

See the [data_structures catalog](data_structures/README.md) for the full list.

---

## Quick start

Everything is header-only, so you can drop a folder into your project and
`#include` it. To build and run the bundled examples and tests, use the Makefile:

```bash
make all            # build every example and test binary into build/
make test           # build and run all test suites
make run-vector     # build + run a single example (any module name works)
make test-set       # build + run a single test suite
make clean          # remove build artifacts
make help           # list every target
```

Requirements: a C++14 compiler (`g++`/`clang++`). The systems modules use
`-pthread`, which the Makefile passes automatically.

### Use a component directly

```cpp
#include "vector/vector.hpp"
#include "unique_ptr/unique_ptr.hpp"

Vector<int> v = {1, 2, 3};
v.push_back(4);

auto p = makeUnique<int>(42);
```

---

## How to read this repo

The documentation is layered so you can go as deep as you like:

1. **[Cross-cutting concepts](docs/README.md)** — the shared memory pattern, RAII,
   the container decision tree, and the complexity matrix. Read this first.
2. **Per-component `README.md`** — a 12-section deep dive each: mental model,
   internal representation, step-by-step ASCII walkthroughs of the key
   algorithms, complexity, pitfalls, and a comparison with the real `std::` type.
3. **`<name>.hpp`** — the implementation itself, where the same diagrams and
   explanations live right next to the code they describe.
4. **`<name>_example.cpp`** — a runnable, commented tour of the API.

A good beginner path is `array → vector → string → list → deque`, then the smart
pointers, then the tree- and hash-based containers. The
[learning path in the concepts guide](docs/README.md#6-suggested-learning-path)
spells out the full order.

---

## Repository layout

```
how-to-stl/
├── README.md                 ← you are here (project hub)
├── docs/README.md            ← cross-cutting concepts & decision guide
├── Makefile                  ← build/run/test every module
├── <component>/
│   ├── <component>.hpp        ← implementation (header-only, diagrammed)
│   ├── <component>_example.cpp← runnable example tour
│   └── README.md              ← 12-section deep dive
├── tests/                    ← one test suite per component
└── data_structures/          ← companion: classic DS&A from scratch
    ├── README.md             ← catalog + complexity cheat sheet
    ├── Makefile              ← build/run every demo
    └── <structure>/          ← <structure>.hpp + <structure>.cpp + README.md
```

---

## A note on scope

These implementations are written for **clarity over completeness**. They mirror
the real standard library's data structures, complexity, and exception behaviour,
but intentionally omit production concerns that would obscure the lesson — custom
allocator plumbing, every overload, locale handling, and so on. Each component's
README has a "Comparison with `std::`" section spelling out exactly what was left
out and why. For real code, use the real standard library; to *understand* the
real standard library, read these.

---

## License

See [`LICENSE`](LICENSE).
