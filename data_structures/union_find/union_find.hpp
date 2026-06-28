#ifndef DS_UNION_FIND_HPP
#define DS_UNION_FIND_HPP

// ============================================================================
//  DisjointSet (Union-Find) -- disjoint-set forest with two optimizations
// ============================================================================
//
// THE IDEA
// --------
// Maintain a collection of disjoint sets over n items labelled 0 .. n-1.
// Each set is a tree stored backward: parent_[x] points to x's parent (or to
// x itself if x is the root / representative).
//
// BEFORE unite(4, 5):          AFTER unite(4, 5) (by rank):
//
//     0   1   2   3   4   5         0   1   2   3   4
//     o   o   o   o   o   o         o   o   o   o   o
//                                     \           /
//                                      5         (4 is root)
//
// PATH COMPRESSION (during find)
// --------------------------------
// find(x) walks up to the root, then rewires every node on the path to point
// directly at the root — flattening the tree for future finds.
//
//     find(5):  5 -> 4 -> 2 -> 2        then compress:
//               parent[5]=2, parent[4]=2
//
// UNION BY RANK
// -------------
// When merging two trees, hang the shorter tree under the taller root (rank
// approximates height). Ties increment the winner's rank.
//
// NEAR-O(1) AMORTIZED
// -------------------
// With both tricks, a sequence of M operations (find + unite) costs
// O(M * alpha(N)) where alpha is the inverse Ackermann function — so slowly
// growing it is effectively constant for any realistic N.
//
// COMPLEXITY (amortized, with both optimizations)
//     find / unite / connected ... O(alpha(N))  ~ O(1) in practice
//     count ...................... O(1)
// ============================================================================

#include <vector>

namespace ds {

class DisjointSet {
public:
    // Create `n` singleton sets {0}, {1}, ..., {n-1}.
    explicit DisjointSet(int n)
        : parent_(static_cast<std::size_t>(n)),
          rank_(static_cast<std::size_t>(n), 0),
          sets_(n) {
        for (int i = 0; i < n; ++i)
            parent_[static_cast<std::size_t>(i)] = i;
    }

    int size() const { return static_cast<int>(parent_.size()); }

    // Number of disjoint sets remaining.
    int count() const { return sets_; }

    // Representative of the set containing `x` (with path compression).
    int find(int x) {
        if (parent_[static_cast<std::size_t>(x)] != x)
            parent_[static_cast<std::size_t>(x)] =
                find(parent_[static_cast<std::size_t>(x)]);
        return parent_[static_cast<std::size_t>(x)];
    }

    // Const find without compression (read-only queries).
    int find(int x) const {
        while (parent_[static_cast<std::size_t>(x)] != x)
            x = parent_[static_cast<std::size_t>(x)];
        return x;
    }

    // Merge the sets containing `x` and `y` (union by rank).
    void unite(int x, int y) {
        int rx = find(x);
        int ry = find(y);
        if (rx == ry) return;

        if (rank_[static_cast<std::size_t>(rx)] < rank_[static_cast<std::size_t>(ry)])
            std::swap(rx, ry);

        parent_[static_cast<std::size_t>(ry)] = rx;
        if (rank_[static_cast<std::size_t>(rx)] == rank_[static_cast<std::size_t>(ry)])
            ++rank_[static_cast<std::size_t>(rx)];

        --sets_;
    }

    // True when `x` and `y` belong to the same set.
    bool connected(int x, int y) { return find(x) == find(y); }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
    int sets_;
};

}  // namespace ds

#endif  // DS_UNION_FIND_HPP
