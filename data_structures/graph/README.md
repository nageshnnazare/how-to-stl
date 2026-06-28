# Graph (Adjacency List)

Vertices are numbered `0 .. n-1`. Each vertex keeps a vector of outgoing
`Edge { to, weight }` records — an *adjacency list*. Supports directed or
undirected graphs and several classic algorithms.

## Picture

```
vertices: 0, 1, 2, 3

adj[0]:  -> 1(w=1) -> 2(w=4)
adj[1]:  -> 2(w=2) -> 3(w=5)
adj[2]:  -> 3(w=1)
adj[3]:  (empty)

0 --1--> 1 --2--> 2 --1--> 3
|                 ^
+-------4---------+
```

Undirected `add_edge(u, v)` stores both directions. Directed stores only `u -> v`.

## Operations

| Operation | Cost | Notes |
|---|---|---|
| `add_edge(u, v, w)` | O(1) amortized | appends to `adj[u]` |
| `neighbors(u)` | O(1) to access | returns adjacency vector |
| `bfs(src)` | O(V + E) | discovery order |
| `dfs(src)` | O(V + E) | iterative stack |
| `dijkstra(src)` | O((V+E) log V) | non-negative weights only |
| `topological_sort()` | O(V + E) | Kahn's; empty if cycle |

## Usage

```cpp
#include "graph.hpp"

ds::Graph g(5, true);          // 5 vertices, directed
g.add_edge(0, 1, 4);
g.add_edge(0, 2, 1);

auto order = g.bfs(0);
auto dist  = g.dijkstra(0);   // dist[v] == shortest path from 0

ds::Graph dag(4, true);
dag.add_edge(0, 1);
dag.add_edge(1, 2);
auto topo = dag.topological_sort();  // e.g. [0, 1, 2, 3]
```

## Build & run

```bash
make run-graph                  # from data_structures/
# or
g++ -std=c++14 -Wall -Wextra -I.. graph.cpp -o demo && ./demo
```

## Things to watch

- Vertex labels must be in `0 .. n-1`; there is no `add_vertex` — pick `n`
  up front.
- `dijkstra` assumes **non-negative** weights; negative edges need Bellman–Ford.
- `topological_sort` is for **directed acyclic** graphs. A cycle yields an
  **empty** vector (not a partial order).
- `dfs` order depends on neighbor push order; compare to BFS when teaching
  layer-by-layer vs depth-first exploration.
