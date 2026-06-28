# Hash Table

A key–value map built from an array of *buckets*. Each bucket holds a short
chain (vector) of entries that hashed to the same slot. Collisions are resolved
by scanning that chain — *separate chaining*.

## Picture

```
hash("cat") % 4 == 2        hash("dog") % 4 == 2   (collision!)

bucket:   0      1      2              3
        +----+ +----+ +----------+   +----+
        |    | |    | | cat:3    |   |    |
        +----+ +----+ | dog:7    |   +----+
                      +----------+
                           ^
                      same bucket -> chain
```

When `load_factor = size / bucket_count` exceeds the threshold (default **0.75**),
the table **rehashes**: roughly double the buckets and redistribute every entry.

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `put(k, v)` | O(1) amortized | updates if key exists; may rehash |
| `get(k)` | O(1) average | returns `V*` or `nullptr` |
| `contains(k)` | O(1) average | |
| `erase(k)` | O(1) average | returns `true` if removed |
| `size` / `empty` | O(1) | |
| `load_factor` | O(1) | triggers grow when ≥ `max_load_factor` |

Worst case (all keys collide into one bucket): O(N) per operation.

## Usage

```cpp
#include "hash_table.hpp"

ds::HashTable<std::string, int> ages;
ages.put("alice", 20);
ages.put("bob", 22);

if (const int* p = ages.get("alice"))
    std::cout << *p << '\n';

ages.erase("bob");
```

## Build & run

```bash
make run-hash_table             # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. hash_table.cpp -o demo && ./demo
```

## Things to watch

- `get` returns a **pointer** — `nullptr` means absent; dereferencing without a
  check is undefined behaviour.
- Keys must be hashable (`std::hash<K>`) and comparable with `==`.
- A bad hash function or adversarial keys can force long chains; rehashing
  fixes load but not a uniformly terrible hash.
- This is **not** thread-safe; concurrent `put`/`get` needs external locking.
