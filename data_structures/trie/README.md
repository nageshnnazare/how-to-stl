# Trie (Prefix Tree)

A tree of characters for storing lowercase English words. Each edge is one
letter; a node marked as end-of-word means the path from the root spells a
complete word. Great for prefix lookup (`starts_with`) and autocomplete.

## Picture

```
Words: "cat", "car", "dog", "do"

root --c-->* --a-->* --t(end)
       |            \--r(end)
       |
       +--d-->* --o(end) --g(end)
```

Children are stored as `children[26]` (`'a'` → 0, `'z'` → 25): O(1) indexing,
fixed memory per node. A `std::map<char, Node*>` would save space for sparse
alphabets but adds pointer overhead per edge.

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `insert` / `contains` / `erase` | O(W) | W = word length |
| `starts_with` | O(W) | walk the prefix path |
| `words_with_prefix` | O(W + matches) | DFS from prefix node |
| `word_count` | O(1) | tracked on insert/erase |

## Usage

```cpp
#include "trie.hpp"

ds::Trie trie;
trie.insert("cat");
trie.insert("car");
trie.starts_with("ca");              // true
trie.contains("ca");                 // false (prefix only)
for (const auto& w : trie.words_with_prefix("ca"))
    std::cout << w << ' ';           // car cat
```

## Build & run

```bash
make run-trie        # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. trie.cpp -o demo && ./demo
```

## Things to watch

- Designed for **lowercase a–z** only; other characters would need bounds checks
  or a different alphabet mapping.
- `children[26]` trades memory for speed; 26 pointers per node even when empty.
- `erase` prunes dead branches when a node has no children and is not an
  end-of-word marker.
