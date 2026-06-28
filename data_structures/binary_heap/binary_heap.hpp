#ifndef DS_BINARY_HEAP_HPP
#define DS_BINARY_HEAP_HPP

// ============================================================================
//  BinaryHeap<T, Compare> -- array-backed binary heap
// ============================================================================
//
// THE IDEA
// --------
// A binary heap is a nearly-complete binary tree stored in a flat array.
// Index arithmetic maps parent/child without pointers:
//
//     parent(i)       = (i - 1) / 2
//     left_child(i)   = 2*i + 1
//     right_child(i)  = 2*i + 2
//
// ARRAY <-> TREE (max-heap example, larger values nearer the root)
//
//     indices:  0   1   2   3   4   5   6
//     array:   [90][80][70][30][50][60][40]
//
//     tree:              (90)
//                     /        \
//                  (80)        (70)
//                 /   \       /
//              (30) (50)  (60)
//              /
//           (40)
//
// DEFAULT: MAX-HEAP with Compare = std::less<T>
// ---------------------------------------------
// std::less means compare(a,b) is true when a < b. We sift so the parent is
// *not less than* its children — i.e. the largest element sits at index 0.
// For a min-heap, use Compare = std::greater<T>.
//
// BUILD (heapify)
// ---------------
// Floyd's method: sift_down from index (n/2 - 1) down to 0 — O(N) total.
//
// COMPLEXITY
//     push / pop .............. O(log N)
//     top ..................... O(1)
//     build (heapify) ......... O(N)
// ============================================================================

#include <cstddef>      // std::size_t
#include <functional>   // std::less
#include <utility>      // std::move, std::swap
#include <vector>       // std::vector

namespace ds {

template <typename T, typename Compare = std::less<T>>
class BinaryHeap {
public:
    BinaryHeap() = default;

    explicit BinaryHeap(const std::vector<T>& values) : data_(values) {
        build_heap();
    }

    // ---- capacity ----------------------------------------------------------

    std::size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    // Largest element (max-heap with default Compare).
    const T& top() const { return data_.front(); }

    // ---- modifiers ---------------------------------------------------------

    void push(const T& value) {
        data_.push_back(value);
        sift_up(data_.size() - 1);
    }

    // Remove the root and restore the heap property.
    void pop() {
        if (data_.empty()) return;
        data_[0] = std::move(data_.back());
        data_.pop_back();
        if (!data_.empty()) sift_down(0);
    }

    // Turn an arbitrary vector into a heap in O(N).
    void build_heap() {
        if (data_.empty()) return;
        for (std::size_t i = data_.size() / 2; i-- > 0;)
            sift_down(i);
    }

    // Build a max-heap from `values` (does not mutate the argument).
    static BinaryHeap build(const std::vector<T>& values) {
        return BinaryHeap(values);
    }

    const std::vector<T>& data() const { return data_; }

private:
    std::vector<T> data_;
    Compare cmp_{};

    static std::size_t parent(std::size_t i) { return (i - 1) / 2; }
    static std::size_t left_child(std::size_t i) { return 2 * i + 1; }
    static std::size_t right_child(std::size_t i) { return 2 * i + 2; }

    // Bubble index `i` toward the root while it outranks its parent.
    void sift_up(std::size_t i) {
        while (i > 0) {
            std::size_t p = parent(i);
            if (!cmp_(data_[p], data_[i])) break;  // parent >= child (max-heap)
            std::swap(data_[p], data_[i]);
            i = p;
        }
    }

    // Push index `i` toward the leaves while a child outranks it.
    void sift_down(std::size_t i) {
        const std::size_t n = data_.size();
        while (true) {
            std::size_t best = i;
            std::size_t l = left_child(i);
            std::size_t r = right_child(i);
            if (l < n && cmp_(data_[best], data_[l])) best = l;
            if (r < n && cmp_(data_[best], data_[r])) best = r;
            if (best == i) break;
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};

}  // namespace ds

#endif  // DS_BINARY_HEAP_HPP
