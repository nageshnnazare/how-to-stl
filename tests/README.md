# Tests

One self-contained test suite per component — 24 in total. Each file is an
ordinary C++ program with a tiny inline harness (no external framework): it runs
a series of numbered checks, prints `PASSED`/`FAILED` for each, and exits
non-zero on the first failure so `make test` stops loudly.

## Running

```bash
make test            # build + run every suite (stops at first failure)
make test-vector     # build + run a single suite (any module name)
make test-set
make build-tests     # just compile every suite into build/
```

Every suite is also compiled in the default `make all`.

## Layout

```
tests/
└── <component>_test.cpp     # the suite for <component>/<component>.hpp
```

Each suite `#include`s the header under test directly (the implementations are
header-only) and is built with:

```
g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. tests/<x>_test.cpp -o build/<x>_test
```

## Coverage

| Suite | Checks | Focus |
|---|---|---|
| `unique_ptr_test` | 30+ | move-only, reset/release, arrays, custom deleters, polymorphism |
| `shared_ptr_test` | many | ref-counting, `weak_ptr`, cycles, aliasing, thread-safety |
| `array_test` | 12 | fixed size, `fill`, bounds, iteration |
| `vector_test` | 39 | growth, insert/erase, capacity, move, comparisons |
| `string_test` | many | SSO, append/insert/replace, find, comparisons |
| `deque_test` | 14 | both-end push/pop, **chunk growth (1000+)**, random access, resize, swap |
| `list_test` | 15 | push/pop ends, iteration, copy |
| `set_test` | 15 | ordered iteration, RB balancing, find/erase |
| `map_test` | 13 | `operator[]`, `at` throw, insert pair, ordered iteration, **500-key stress** |
| `multiset_test` | 12 | duplicate runs, `count`, erase-whole-run |
| `multimap_test` | 10 | duplicate keys, key-ordered iteration, erase block |
| `unordered_set_test` | 11 | insert/contains/erase, **rehash (1000)** |
| `unordered_map_test` | 11 | `operator[]`, `at` throw, **rehash (1000)** |
| `stack_test` | 15 | LIFO order, underflow safety |
| `queue_test` | 15 | FIFO order |
| `priority_queue_test` | 20 | heap order, sift up/down |
| `pair_test` | 14 | construction, comparisons, `make_pair`, nesting |
| `tuple_test` | 10 | `get<I>`, heterogeneous storage |
| `optional_test` | 13 | engaged/disengaged, `value_or`, throw on empty |
| `bitset_test` | 17 | set/reset/flip, bitwise ops, multi-word |
| `allocator_test` | 18 | linear / pool / stack / free-list, coalescing |
| `arena_allocator_test` | 45 | bump alloc, **alignment**, scoped reset, concurrency |
| `locks_test` | 23 | spinlock, RAII guards, reader/writer |
| `thread_pool_test` | 29 | submit/futures, parallel-for, shutdown |

## Memory & UB checking

The suites are clean under sanitizers; this is the recommended way to verify a
change:

```bash
# AddressSanitizer + UndefinedBehaviorSanitizer
g++ -std=c++14 -g -fsanitize=address,undefined -pthread -I. tests/deque_test.cpp -o /tmp/t && /tmp/t

# Valgrind (Linux)
valgrind --leak-check=full ./build/vector_test
```

> The stress tests in `deque_test`, `map_test`, and the `unordered_*` suites
> exist specifically to exercise reallocation/rehashing paths under ASan — they
> caught real bugs (a deque map-reallocation overflow, an `unordered_map`
> dangling-reference-on-rehash, and an arena alignment defect) during the
> revamp.

## Adding a test

1. Open `tests/<component>_test.cpp`.
2. Add a numbered block following the existing pattern:
   ```cpp
   std::cout << "Test N: what it checks... ";
   { /* arrange + act */ if (!expected) { std::cout << "FAILED\n"; return 1; } }
   std::cout << "PASSED\n"; ++passed;
   ```
3. Bump the `total` counter at the top.
4. `make test-<component>`.

For a brand-new component, also add its name to `MODULES` in the root
[`Makefile`](../Makefile) — it then gets `run-`/`test-` targets automatically.
