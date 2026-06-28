#ifndef DS_GRAPH_HPP
#define DS_GRAPH_HPP

// ============================================================================
//  Graph -- adjacency list for vertices 0 .. n-1
// ============================================================================
//
// THE IDEA
// --------
// A graph is a set of vertices and edges. We store it as an *adjacency list*:
// for each vertex u, adj_[u] lists every outgoing edge (neighbor v, weight w).
//
//     vertices: 0, 1, 2, 3
//
//     adj[0]:  -> 1(w=1) -> 2(w=4)
//     adj[1]:  -> 2(w=2) -> 3(w=5)
//     adj[2]:  -> 3(w=1)
//     adj[3]:  (empty)
//
//     0 --1--> 1 --2--> 2 --1--> 3
//     |                 ^
//     +-------4---------+
//
// UNDIRECTED vs DIRECTED
// ----------------------
// If `directed_` is false, add_edge(u, v) also inserts the reverse edge (v, u)
// with the same weight. If true, the edge goes only u -> v.
//
// ALGORITHMS INCLUDED
// -------------------
//   bfs(src)            visit order (and implicit level by discovery time)
//   dfs(src)            depth-first visit order (iterative stack)
//   dijkstra(src)       shortest distances from src (non-negative weights)
//   topological_sort()  Kahn's algorithm on a DAG; empty vector if a cycle exists
//
// COMPLEXITY (V = vertices, E = edges)
//     add_edge / neighbors .... O(1) amortized (append to a vector)
//     bfs / dfs ............... O(V + E)
//     dijkstra ................ O((V + E) log V) with a binary heap
//     topological_sort ........ O(V + E)
// ============================================================================

#include <limits>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

namespace ds {

struct Edge {
    int to;
    int weight;
};

class Graph {
public:
    // `n` vertices labelled 0 .. n-1. `directed` controls edge direction.
    explicit Graph(int n, bool directed = false)
        : directed_(directed), adj_(static_cast<std::size_t>(n)) {}

    int vertex_count() const { return static_cast<int>(adj_.size()); }
    bool is_directed() const { return directed_; }

    // Add an edge u -> v with optional weight (default 1).
    // Undirected graphs also add v -> u.
    void add_edge(int u, int v, int weight = 1) {
        adj_[static_cast<std::size_t>(u)].push_back({v, weight});
        if (!directed_)
            adj_[static_cast<std::size_t>(v)].push_back({u, weight});
    }

    // Outgoing edges from `u`.
    const std::vector<Edge>& neighbors(int u) const {
        return adj_[static_cast<std::size_t>(u)];
    }

    // ---- traversals --------------------------------------------------------

    // Breadth-first search from `src`. Returns vertices in discovery order.
    // Vertices at the same BFS layer appear consecutively when the graph is
    // unweighted and edges are explored in adjacency-list order.
    std::vector<int> bfs(int src) const {
        const int n = vertex_count();
        std::vector<bool> seen(static_cast<std::size_t>(n), false);
        std::vector<int> order;
        order.reserve(static_cast<std::size_t>(n));

        std::queue<int> q;
        seen[static_cast<std::size_t>(src)] = true;
        q.push(src);

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (const Edge& e : neighbors(u)) {
                if (!seen[static_cast<std::size_t>(e.to)]) {
                    seen[static_cast<std::size_t>(e.to)] = true;
                    q.push(e.to);
                }
            }
        }
        return order;
    }

    // Depth-first search from `src` (iterative). Returns finish-time order.
    std::vector<int> dfs(int src) const {
        const int n = vertex_count();
        std::vector<bool> seen(static_cast<std::size_t>(n), false);
        std::vector<int> order;
        order.reserve(static_cast<std::size_t>(n));

        std::stack<int> st;
        st.push(src);
        while (!st.empty()) {
            int u = st.top();
            st.pop();
            if (seen[static_cast<std::size_t>(u)]) continue;
            seen[static_cast<std::size_t>(u)] = true;
            order.push_back(u);
            // Push neighbors in reverse so lower-index neighbors are popped first.
            const auto& adj = neighbors(u);
            for (int i = static_cast<int>(adj.size()) - 1; i >= 0; --i)
                st.push(adj[static_cast<std::size_t>(i)].to);
        }
        return order;
    }

    // ---- shortest paths ----------------------------------------------------

    // Dijkstra from `src`. Returns distance to every vertex; unreachable
    // vertices get INT_MAX. Requires non-negative edge weights.
    std::vector<int> dijkstra(int src) const {
        const int n = vertex_count();
        const int INF = std::numeric_limits<int>::max();
        std::vector<int> dist(static_cast<std::size_t>(n), INF);
        dist[static_cast<std::size_t>(src)] = 0;

        // min-heap on (distance, vertex)
        using P = std::pair<int, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.push({0, src});

        while (!pq.empty()) {
            int d = pq.top().first;
            int u = pq.top().second;
            pq.pop();
            if (d != dist[static_cast<std::size_t>(u)]) continue;  // stale entry

            for (const Edge& e : neighbors(u)) {
                if (e.weight < 0) continue;  // guard: Dijkstra needs non-neg
                if (dist[static_cast<std::size_t>(u)] == INF) continue;
                int nd = dist[static_cast<std::size_t>(u)] + e.weight;
                if (nd < dist[static_cast<std::size_t>(e.to)]) {
                    dist[static_cast<std::size_t>(e.to)] = nd;
                    pq.push({nd, e.to});
                }
            }
        }
        return dist;
    }

    // ---- topological sort --------------------------------------------------

    // Kahn's algorithm (BFS on in-degrees). Valid only for DAGs.
    // If a directed cycle exists, not every vertex can be placed — we return
    // an *empty* vector to signal failure (no valid topological order).
    std::vector<int> topological_sort() const {
        if (!directed_)
            return {};  // undefined for undirected graphs

        const int n = vertex_count();
        std::vector<int> indeg(static_cast<std::size_t>(n), 0);
        for (int u = 0; u < n; ++u)
            for (const Edge& e : neighbors(u))
                ++indeg[static_cast<std::size_t>(e.to)];

        std::queue<int> q;
        for (int v = 0; v < n; ++v)
            if (indeg[static_cast<std::size_t>(v)] == 0) q.push(v);

        std::vector<int> order;
        order.reserve(static_cast<std::size_t>(n));

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (const Edge& e : neighbors(u)) {
                int& d = indeg[static_cast<std::size_t>(e.to)];
                if (--d == 0) q.push(e.to);
            }
        }

        if (static_cast<int>(order.size()) != n)
            return {};  // cycle detected
        return order;
    }

private:
    bool directed_;
    std::vector<std::vector<Edge>> adj_;
};

}  // namespace ds

#endif  // DS_GRAPH_HPP
