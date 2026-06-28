#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <cstddef>      // for size_t
#include <vector>       // default heap storage (flat array)
#include <functional>   // for std::less
#include <stdexcept>    // for out_of_range
#include <utility>      // for move, swap, forward

// ============================================================================
//  PriorityQueue<T> -- a hand-rolled std::priority_queue (binary heap)
// ============================================================================
//
// WHAT IT IS
// ----------
// A PriorityQueue is a container adapter over Container `c_` (default
// std::vector<T>) arranged as a **binary heap**. The "top" is always the
// best element per Compare `comp_` (default std::less → **max-heap**: largest
// at top). push/pop are O(log n); top is O(1).
//
// THE TWO FIELDS
// --------------
//
//     c_     -> flat array holding the complete binary tree level-order
//     comp_  -> strict weak ordering; comp_(a,b) means "a is lower priority than b"
//
// TREE VIEW vs ARRAY VIEW (max-heap, values 1..7)
// -------------------------------------------------
//
//     Tree (conceptual)              c_ (index 0 = root = top())
//            50                           index:  0   1   2   3   4   5   6
//           /  \                          c_:    [50][30][40][10][20][35][25]
//         30    40
//        / \   / \
//      10  20 35 25
//
//     Index math (0-based):
//         parent(i) = (i - 1) / 2
//         left(i)   = 2 * i + 1
//         right(i)  = 2 * i + 2
//
// HEAP PROPERTY (max-heap with std::less)
// ---------------------------------------
// For every node i:  comp_(c_[i], c_[child]) is false when child exists —
// i.e. parent is NOT less than either child → parent is ≥ children.
//
// PUSH — sift-up (heapify_up)
// ---------------------------
// Append at end (next leaf), bubble up while parent is lower priority:
//
//     push(45): append → sift-up swaps with parent while comp_(parent, new)
//
//         step 0:        50              c_: [50,30,40,10,20,35,25,45]
//                         / \
//                       ...  ...
//     new 45 at index 7; parent(7)=3 → compare 45 vs 10 → swap ...
//
// POP — sift-down (heapify_down)
// ------------------------------
// Move last element to root, shrink array, sink root while a child wins:
//
//     (1) save top  (2) c_[0] = move(c_.back())  (3) pop_back
//     (4) heapify_down(0) — pick larger child, swap, repeat
//
// BUILD_HEAP — Floyd's O(n) heapify
// ---------------------------------
// Range/container ctor calls build_heap(): sift-down from last non-leaf down to 0.
//
// Key characteristics:
// - Complete binary tree in a vector — no child pointers
// - Default max-heap; std::greater → min-heap
// - push O(log n), pop O(log n), top O(1)
// - Not stable — equal keys may reorder on push/pop
// ============================================================================

template<typename T, typename Container = std::vector<T>, typename Compare = std::less<T>>
class PriorityQueue {
private:
    Container c_;    // heap elements in level-order; c_[0] is top
    Compare comp_;   // returns true if first arg is LOWER priority than second

    /**
     * @brief Parent index in the implicit complete binary tree.
     *
     *     parent(5) = (5-1)/2 = 2
     *
     *         index 2
     *        /       \
     *       5         6
     */
    size_t parent(size_t i) const {
        return (i - 1) / 2;
    }

    /**
     * @brief Left child index; may be >= size() (no left child).
     *
     *     left(2) = 2*2+1 = 5
     */
    size_t left(size_t i) const {
        return 2 * i + 1;
    }

    /**
     * @brief Right child index; may be >= size() (no right child).
     */
    size_t right(size_t i) const {
        return 2 * i + 2;
    }

    /**
     * @brief Sift-up: restore heap after appending at index i (push path).
     *
     *     while i > 0 and comp_(parent, i):  swap parent ↔ i;  i = parent(i)
     *
     * Example (max-heap, comp_=less, new 45 at leaf index 7):
     *
     *         50                         50
     *        /  \                       /  \
     *      30    40        →          30    40
     *     / \   / \                  / \   / \
     *   10 20 35 25                 10 20 35 25
     *   /
     *  45  (bubbles up while > parent)
     *
     * WHY upward: only the leaf→root path may violate heap; swapping fixes
     * one level at a time in O(log n) steps.
     */
    void heapify_up(size_t i) {
        while (i > 0 && comp_(c_[parent(i)], c_[i])) {
            std::swap(c_[i], c_[parent(i)]);
            i = parent(i);
        }
    }

    /**
     * @brief Sift-down: restore heap after replacing root (pop path).
     *
     *     at index i, pick best child (per comp_) if any beats c_[i];
     *     swap and recurse into child index.
     *
     * Example after pop moved last element 25 to root:
     *
     *         25                         40
     *        /  \                       /  \
     *      30    40        →          30    35
     *     / \   /                    / \
     *   10 20 35                    10 20 25
     *
     * WHY downward: after root replacement, violation flows down one branch;
     * at most tree height swaps.
     */
    void heapify_down(size_t i) {
        size_t largest = i;
        size_t l = left(i);
        size_t r = right(i);

        if (l < c_.size() && comp_(c_[largest], c_[l])) {
            largest = l;
        }
        if (r < c_.size() && comp_(c_[largest], c_[r])) {
            largest = r;
        }

        if (largest != i) {
            std::swap(c_[i], c_[largest]);
            heapify_down(largest);
        }
    }

    /**
     * @brief Floyd's build-heap: O(n) sift-down from last non-leaf to root.
     *
     *     last index = n-1  →  start i = parent(n-1)  →  down to 0:
     *         heapify_down(i)
     *
     *     unsorted [4,1,3,2,16,9,10,14,8,7]
     *         → single bottom-up pass fixes all subtrees
     *
     * WHY O(n) not O(n log n): most nodes are near leaves and sift short distances;
     * used when bulk-loading from an existing container/range.
     */
    void build_heap() {
        if (c_.size() <= 1) return;

        for (int i = parent(c_.size() - 1); i >= 0; --i) {
            heapify_down(static_cast<size_t>(i));
        }
    }

public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using container_type = Container;
    using value_compare = Compare;

    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /** @brief Empty max-heap (default std::less → largest on top). */
    PriorityQueue() : c_(), comp_() {}

    /** @brief Empty heap with custom comparator (e.g. std::greater for min-heap). */
    explicit PriorityQueue(const Compare& compare) : c_(), comp_(compare) {}

    /**
     * @brief Wrap existing container and heapify in O(n) via Floyd.
     *
     * c_ may be arbitrary order; build_heap() establishes heap property in place.
     */
    PriorityQueue(const Compare& compare, const Container& cont)
        : c_(cont), comp_(compare) {
        build_heap();
    }

    /** @brief Move-container variant — steals storage then build_heap(). */
    PriorityQueue(const Compare& compare, Container&& cont)
        : c_(std::move(cont)), comp_(compare) {
        build_heap();
    }

    /**
     * @brief Range constructor — copy [first,last) into c_, then build_heap().
     */
    template<typename InputIt>
    PriorityQueue(InputIt first, InputIt last, const Compare& compare = Compare())
        : c_(first, last), comp_(compare) {
        build_heap();
    }

    /** @brief Copy — duplicates both array and comparator. */
    PriorityQueue(const PriorityQueue& other) : c_(other.c_), comp_(other.comp_) {}

    /** @brief Move — O(1) steal of vector and comparator. */
    PriorityQueue(PriorityQueue&& other) noexcept
        : c_(std::move(other.c_)), comp_(std::move(other.comp_)) {}

    ~PriorityQueue() = default;

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /** @brief Copy assignment. */
    PriorityQueue& operator=(const PriorityQueue& other) {
        if (this != &other) {
            c_ = other.c_;
            comp_ = other.comp_;
        }
        return *this;
    }

    /** @brief Move assignment. */
    PriorityQueue& operator=(PriorityQueue&& other) noexcept {
        if (this != &other) {
            c_ = std::move(other.c_);
            comp_ = std::move(other.comp_);
        }
        return *this;
    }

    // ============================================================================
    // ELEMENT ACCESS
    // ============================================================================

    /**
     * @brief Highest-priority element at c_[0] (heap root).
     *
     *     c_: [TOP][ ... children in level-order ... ]
     *
     * @throws std::out_of_range if empty (stricter than std::priority_queue).
     */
    const_reference top() const {
        if (empty()) {
            throw std::out_of_range("PriorityQueue::top: queue is empty");
        }
        return c_.front();
    }

    // ============================================================================
    // CAPACITY
    // ============================================================================

    /** @brief True when heap has no elements. */
    bool empty() const noexcept {
        return c_.empty();
    }

    /** @brief Number of elements in the heap. */
    size_type size() const noexcept {
        return c_.size();
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Insert copy at end, then sift-up to restore heap — O(log n).
     *
     *     push_back → heapify_up(size-1)
     */
    void push(const value_type& value) {
        c_.push_back(value);
        heapify_up(c_.size() - 1);
    }

    /** @brief Insert by move, then sift-up. */
    void push(value_type&& value) {
        c_.push_back(std::move(value));
        heapify_up(c_.size() - 1);
    }

    /**
     * @brief Construct in-place at end, then sift-up.
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        c_.emplace_back(std::forward<Args>(args)...);
        heapify_up(c_.size() - 1);
    }

    /**
     * @brief Remove top: move last to root, pop_back, sift-down — O(log n).
     *
     *     (1) if empty → throw
     *     (2) c_[0] = move(c_.back())
     *     (3) pop_back()
     *     (4) if !empty → heapify_down(0)
     *
     * WHY move last to root: keeps heap complete; sift-down fixes ordering.
     *
     * @throws std::out_of_range if empty.
     */
    void pop() {
        if (empty()) {
            throw std::out_of_range("PriorityQueue::pop: queue is empty");
        }

        c_.front() = std::move(c_.back());
        c_.pop_back();

        if (!empty()) {
            heapify_down(0);
        }
    }

    /** @brief Swap underlying container and comparators. */
    void swap(PriorityQueue& other) noexcept {
        std::swap(c_, other.c_);
        std::swap(comp_, other.comp_);
    }

    // ============================================================================
    // COMPARISON (for testing/debugging)
    // ============================================================================

    /**
     * @brief Verify heap property at every node (testing aid).
     *
     * For each i: neither child should beat c_[i] per comp_.
     */
    bool is_heap() const {
        for (size_t i = 0; i < c_.size(); ++i) {
            size_t l = left(i);
            size_t r = right(i);

            if (l < c_.size() && comp_(c_[i], c_[l])) {
                return false;
            }
            if (r < c_.size() && comp_(c_[i], c_[r])) {
                return false;
            }
        }
        return true;
    }
};

/** @brief Non-member swap. */
template<typename T, typename Container, typename Compare>
void swap(PriorityQueue<T, Container, Compare>& lhs,
          PriorityQueue<T, Container, Compare>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // PRIORITY_QUEUE_HPP
