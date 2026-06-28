# LRU Cache

A fixed-capacity key→value cache that, when full, evicts the **Least Recently
Used** entry. Every `get` and `put` "touches" a key and makes it the most
recently used; the entry nobody has touched for longest is the first to go.

The classic O(1) design pairs two structures:

- a **hash map** `key → node` for O(1) lookup, and
- a **doubly linked list** holding nodes in recency order (front = MRU,
  back = LRU) so we can reorder and evict in O(1).

## Picture

```
     MRU (front)                                   LRU (back / evict here)
     ┌───────┐   ┌───────┐   ┌───────┐   ┌───────┐
head │ k=C   │<->│ k=A   │<->│ k=D   │<->│ k=B   │ tail
     │ v=30  │   │ v=10  │   │ v=40  │   │ v=20  │
     └───────┘   └───────┘   └───────┘   └───────┘
   map: C ───────────┘ A ───────┘ D ───────┘ B   (key -> node*)

get(A): map finds the node, unlink it, relink at front  -> A becomes MRU
put(E) when full: drop the tail (B = LRU) from list + map, insert E at front
```

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `get(key)` | O(1) avg | returns `V*` (nullptr on miss); promotes to MRU |
| `put(key, value)` | O(1) avg | insert/update; evicts LRU when full |
| `contains(key)` | O(1) avg | does **not** change recency |
| `erase(key)` | O(1) avg | remove if present |
| `most_recent` / `least_recent` | O(1) | peek the ends |

## Usage

```cpp
#include "lru_cache.hpp"

ds::LRUCache<std::string, int> cache(2);   // capacity 2
cache.put("a", 1);
cache.put("b", 2);
cache.get("a");          // a is now most-recently-used
cache.put("c", 3);       // cache full -> evicts "b" (the LRU)

if (int* v = cache.get("b")) { /* hit */ } else { /* miss: nullptr */ }
```

## Build & run

```bash
make run-lru_cache            # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. lru_cache.cpp -o demo && ./demo
```

## Things to watch

- `get` returns a **pointer** so you can tell "missing" apart from "present but
  equal to a default value". A `nullptr` means a miss.
- `get` and `put` both change recency; use `contains` to probe without touching.
- Capacity must be `> 0` (the constructor throws otherwise) — a zero-capacity
  cache could never store anything.
- This teaching version owns raw nodes and is therefore non-copyable; a
  production cache would add thread-safety and a pluggable eviction policy.
