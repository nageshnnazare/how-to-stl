#ifndef MULTISET_HPP
#define MULTISET_HPP

#include <cstddef>      // for size_t
#include <vector>       // backing sorted contiguous storage
#include <algorithm>    // lower_bound, equal_range
#include <functional>   // for less
#include <initializer_list>

// ============================================================================
//  Multiset<T> -- a hand-rolled std::multiset (sorted vector, duplicates OK)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Multiset is like Set, but equal elements may appear many times. Elements
// are kept in sorted order in a contiguous std::vector<T>. Search is O(log n)
// via binary search; insert is O(n) because the vector must shift elements right
// to keep duplicates adjacent and order intact.
//
// THE TWO FIELDS
// --------------
//     data_  -> sorted vector of elements (duplicates allowed, grouped together)
//     comp_  -> strict weak ordering (default std::less<T>)
//
// MEMORY LAYOUT (example: insert order 3,1,4,1,5 → stored sorted)
// ---------------------------------------------------------------
//
//     Multiset object (stack)          data_ (heap, contiguous)
//     ┌──────────────┐                index:  0   1   2   3   4   5   6
//     │ data_   ●────┼───────────────▶       [1][1][2][3][3][4][5]
//     │ comp_        │                equal keys occupy a contiguous RUN
//     └──────────────┘
//
//     equal_range(3)  →  iterators to [lower, upper) = indices 3..5 (one slot)
//     count(1)        →  distance(lower_bound(1), upper_bound(1)) = 2
//
// WHY SORTED VECTOR (not Red-Black tree like Set)?
// ------------------------------------------------
// Simpler to teach duplicate *ranges*: all copies of a value sit in one slice.
// Trade-off: O(n) insert/erase (element shift) vs Set/Map's O(log n) tree ops.
// Good for small n or read-heavy workloads; std::multiset uses a tree in libc++.
//
// Key characteristics:
// - Sorted order, duplicates allowed and stored contiguously
// - O(log n) find / count / lower_bound; O(n) insert / erase (shift cost)
// - Iterators are vector const_iterators (random access within the sequence)
// ============================================================================

template<typename T, typename Compare = std::less<T>>
class Multiset {
private:
    std::vector<T> data_;  // sorted sequence; equal values form contiguous blocks
    Compare comp_;         // strict weak ordering for sort position and search

public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = typename std::vector<T>::const_iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    /** @brief Default constructor — empty sorted vector. */
    Multiset() = default;

    /**
     * @brief Construct by inserting each element (maintains sort + duplicate runs).
     */
    Multiset(std::initializer_list<T> init) {
        for (const auto& val : init) {
            insert(val);
        }
    }

    /** @brief Iterator to smallest element. */
    iterator begin() const { return data_.begin(); }

    /** @brief One-past-last element (vector end). */
    iterator end() const { return data_.end(); }

    /** @brief True when data_ is empty. */
    bool empty() const { return data_.empty(); }

    /** @brief Number of elements including all duplicates. */
    size_type size() const { return data_.size(); }

    /** @brief Remove all elements; capacity may remain reserved. */
    void clear() { data_.clear(); }

    /**
     * @brief Insert value at sorted position (duplicates allowed).
     *
     *   (1) lower_bound → first slot not less than value (O(log n) binary search).
     *   (2) vector::insert at pos — shifts tail right (O(n) move).
     *
     *     data_: [1][3][5][5]   insert 3:
     *                 ▲ lower_bound(3) at index 1
     *            → [1][3][3][5][5]   (new 3 sits with existing 3s)
     *
     * Returns iterator to the newly inserted copy.
     */
    iterator insert(const T& value) {
        auto pos = std::lower_bound(data_.begin(), data_.end(), value, comp_);
        return data_.insert(pos, value);
    }

    /**
     * @brief Erase ALL elements equal to value (entire duplicate run).
     *
     *   equal_range → [first, second) spanning one key's block:
     *
     *     [1][1][2][5][5][5][9]
     *           └─── equal_range(5) ───┘  → erase 3 copies
     *
     * Returns count erased. O(n) due to vector erase shift.
     */
    size_type erase(const T& value) {
        auto range = std::equal_range(data_.begin(), data_.end(), value, comp_);
        size_type count = std::distance(range.first, range.second);
        data_.erase(range.first, range.second);
        return count;
    }

    /**
     * @brief Find one occurrence — iterator to first equal element or end().
     *
     * lower_bound then verify equivalence (not strictly less in either direction).
     */
    iterator find(const T& value) const {
        auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
        if (it != data_.end() && !comp_(value, *it) && !comp_(*it, value)) {
            return it;
        }
        return data_.end();
    }

    /**
     * @brief Count duplicates — size of equal_range window.
     *
     *     count(5) on [1][5][5][5][7]  →  3
     */
    size_type count(const T& value) const {
        auto range = std::equal_range(data_.begin(), data_.end(), value, comp_);
        return std::distance(range.first, range.second);
    }

    /** @brief True if at least one copy of value exists. */
    bool contains(const T& value) const {
        return find(value) != data_.end();
    }
};

#endif // MULTISET_HPP
