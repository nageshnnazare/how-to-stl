#ifndef MULTIMAP_HPP
#define MULTIMAP_HPP

#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <initializer_list>

// ============================================================================
//  Multimap<Key,T> -- hand-rolled std::multimap (sorted vector, dup keys OK)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Multimap maps keys to values but allows the same key many times (e.g. all
// scores for "Alice"). Pairs are stored in a vector sorted by KEY only; entries
// with equal keys form a contiguous block (order among equal keys = insert order
// among that key's run).
//
// THE TWO FIELDS
// --------------
//     data_  -> vector<pair<Key,T>> sorted by pair.first (key)
//     comp_  -> compares keys (default std::less<Key>)
//
// NODE / SLOT LAYOUT (each vector element is one key-value pair)
// ---------------------------------------------------------------
//
//     ┌─────────────────────────┐
//     │  first:  "Alice"        │  ◀── sort key
//     │  second: 90             │  ◀── mapped value (ignored for ordering)
//     └─────────────────────────┘
//
// CONTIGUOUS STORAGE (keys sorted, duplicate keys grouped)
// --------------------------------------------------------
//
//     index:     0         1         2         3
//     data_:  [Alice,90][Alice,95][Alice,88][Bob,85]
//              └──────── equal_range("Alice") ────────┘
//
//     lower_bound("Alice") → index 0
//     upper_bound("Alice") → index 3 (first slot with key > Alice)
//     equal_range("Alice") → [0, 3)  — three entries
//
// INSERT: lower_bound by key, vector::insert (O(n) shift).
// ERASE:  equal_range by key, erase whole block.
//
// vs Map (Red-Black tree): Multimap trades O(log n) insert for simpler duplicate-
// key ranges and teaching clarity; same trade-off as Multiset vs Set.
// ============================================================================

template<typename Key, typename T, typename Compare = std::less<Key>>
class Multimap {
private:
    using value_type_internal = std::pair<Key, T>;
    std::vector<value_type_internal> data_;  // sorted by key; duplicate keys adjacent
    Compare comp_;                           // key ordering only

    /**
     * @brief Three-way compare helpers for mixed pair/Key lower_bound arguments.
     *
     * Enables std::lower_bound(data_, key) without wrapping keys in dummy pairs.
     */
    struct KeyCompare {
        Compare comp;
        bool operator()(const value_type_internal& a, const value_type_internal& b) const {
            return comp(a.first, b.first);
        }
        bool operator()(const value_type_internal& a, const Key& b) const {
            return comp(a.first, b);
        }
        bool operator()(const Key& a, const value_type_internal& b) const {
            return comp(a, b.first);
        }
    };

public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;
    using iterator = typename std::vector<value_type_internal>::const_iterator;
    using const_iterator = typename std::vector<value_type_internal>::const_iterator;

    /** @brief Default constructor — empty sorted vector. */
    Multimap() = default;

    /** @brief Insert each pair in sorted key order. */
    Multimap(std::initializer_list<value_type> init) {
        for (const auto& p : init) {
            insert(p);
        }
    }

    /** @brief Smallest key (first pair). */
    iterator begin() const { return data_.begin(); }

    /** @brief One past largest key's last duplicate. */
    iterator end() const { return data_.end(); }

    /** @brief True when empty. */
    bool empty() const { return data_.empty(); }

    /** @brief Total pairs including duplicate keys. */
    size_type size() const { return data_.size(); }

    /** @brief Remove all pairs. */
    void clear() { data_.clear(); }

    /**
     * @brief Insert pair at lower_bound(key) — duplicates keys allowed.
     *
     *     scores.insert({Alice, 90}); scores.insert({Alice, 95});
     *
     *     data_: [Alice,90] → [Alice,90][Alice,95]  (same key block)
     *
     * O(log n) search + O(n) vector shift.
     */
    iterator insert(const value_type& value) {
        KeyCompare key_comp{comp_};
        auto pos = std::lower_bound(data_.begin(), data_.end(), value.first, key_comp);
        return data_.insert(pos, value_type_internal(value.first, value.second));
    }

    /**
     * @brief Erase every pair with the given key (whole equal_range block).
     *
     *     erase("Alice") removes all (Alice, *) entries in one slice erase.
     */
    size_type erase(const Key& key) {
        KeyCompare key_comp{comp_};
        auto range = std::equal_range(data_.begin(), data_.end(), key, key_comp);
        size_type count = std::distance(range.first, range.second);
        data_.erase(range.first, range.second);
        return count;
    }

    /**
     * @brief Iterator to first pair with key, or end() if none.
     */
    iterator find(const Key& key) const {
        KeyCompare key_comp{comp_};
        auto it = std::lower_bound(data_.begin(), data_.end(), key, key_comp);
        if (it != data_.end() && !comp_(key, it->first) && !comp_(it->first, key)) {
            return it;
        }
        return data_.end();
    }

    /**
     * @brief Number of pairs with this key — width of equal_range window.
     */
    size_type count(const Key& key) const {
        KeyCompare key_comp{comp_};
        auto range = std::equal_range(data_.begin(), data_.end(), key, key_comp);
        return std::distance(range.first, range.second);
    }

    /** @brief True if key appears at least once. */
    bool contains(const Key& key) const {
        return find(key) != data_.end();
    }
};

#endif // MULTIMAP_HPP
