#ifndef UNORDERED_SET_HPP
#define UNORDERED_SET_HPP

#include <cstddef>           // for size_t
#include <vector>            // bucket array
#include <list>              // collision chains
#include <functional>        // for std::hash
#include <initializer_list>  // for initializer_list

// ============================================================================
//  UnorderedSet<T> -- a hand-rolled std::unordered_set (hash table, chaining)
// ============================================================================
//
// WHAT IT IS
// ----------
// An UnorderedSet stores UNIQUE values with no particular order. Lookup is
// driven by a hash function: we map each value to a bucket index in O(1), then
// scan only the short linked list in that bucket. Average-case insert/find/erase
// are O(1); worst case is O(n) when every value lands in the same bucket.
//
// THE THREE FIELDS
// ----------------
//
//     buckets_  -> vector of lists; index i is bucket i (a collision chain)
//     size_     -> total number of stored elements (unique count)
//     hash_     -> callable that maps T -> size_t hash code
//
// Invariant: size_ equals the sum of all bucket list lengths.
//
// MEMORY LAYOUT (8 buckets, size_ = 5; collisions in buckets 1 and 6)
// -----------------------------------------------------------------------
//
//     UnorderedSet object (stack)          buckets_ (vector of 8 lists)
//     ┌─────────────────────┐              index:  0    1      2   3   4   5   6      7
//     │ buckets_   ●────────┼─────────────▶       [ ]──▶42──▶99  [ ] [ ] [ ] [ ]──▶7──▶31  [ ]
//     │ size_      = 5      │                     empty  chain   empty ...        chain   empty
//     │ hash_      (fn)    │
//     └─────────────────────┘
//
// Each bucket is a std::list<T>. Empty buckets cost one empty list head (cheap).
// Colliding values share a bucket and are chained in insertion order.
//
// HASHING AND BUCKET INDEX
// ------------------------
//     idx = hash_(value) % buckets_.size()
//
//     value "hello"  ──hash_──▶ 0x8f3a2c1b...  ──% 8──▶ bucket 3
//                                                      │
//                                                      ▼
//                                              buckets_[3] ──▶ "hello"
//
// LOAD FACTOR AND REHASH
// ----------------------
//     load_factor = size_ / buckets_.size()
//
// When size_ > buckets_.size() * 2 (load factor exceeds 2.0), we DOUBLE the
// bucket count and re-insert every element — each value is re-hashed with the
// NEW modulus, so chains redistribute:
//
//     BEFORE (4 buckets, size_=9, load=2.25 → trigger rehash)
//     buckets_:  [a,b]  [c]  [ ]  [d,e,f,g,h,i]   (crowded bucket 3)
//
//     AFTER  (8 buckets — every element re-hashed % 8)
//     buckets_:  [a] [b,c] [ ] [d] [e] [f] [g] [h,i]   chains shorter on average
//
// Key characteristics:
// - Separate chaining: collisions never overwrite, they extend a list
// - No iteration API in this teaching build (std adds iterators)
// - Duplicates rejected: insert returns false if value already present
// - Rehash is O(n) but rare enough to keep insert amortized O(1) average
// ============================================================================

template<typename T, typename Hash = std::hash<T>>
class UnorderedSet {
private:
    std::vector<std::list<T>> buckets_;  // bucket array; each slot is one collision chain
    size_t size_;                        // number of unique elements stored
    Hash hash_;                          // maps a T to a size_t hash code
    static constexpr size_t DEFAULT_BUCKETS = 16;  // initial bucket count at construction

    /**
     * @brief Map a value to a bucket index via hash modulo bucket count.
     *
     *     value v
     *        │
     *        ▼
     *     hash_(v)  ──% buckets_.size()──▶  idx  ──▶  buckets_[idx]
     *
     * WHY modulo: spreads hash codes across [0, bucket_count). The quality of
     * hash_ and the bucket count together determine how evenly elements distribute.
     */
    size_t bucket_index(const T& value) const {
        return hash_(value) % buckets_.size();
    }

    /**
     * @brief Grow the bucket table when load factor exceeds 2.0.
     *
     * Trigger: size_ > buckets_.size() * 2  (i.e. load_factor > 2.0).
     *
     *     STEP 1 — allocate new_buckets with 2× capacity (all empty lists)
     *         old: 4 buckets                    new: 8 buckets
     *              [●─▶x,y] [●─▶z] [ ] [ ]           [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
     *
     *     STEP 2 — walk every chain in old buckets_, for each value v:
     *              idx = hash_(v) % new_size
     *              new_buckets[idx].push_back(v)
     *
     *     STEP 3 — buckets_ = move(new_buckets); size_ unchanged
     *
     * WHY rehash: long chains make find/insert O(chain length). Doubling buckets
     * halves the expected chain length on average, restoring O(1) average ops.
     * Every element MUST be re-hashed because the modulus changed.
     */
    void rehash_if_needed() {
        if (size_ > buckets_.size() * 2) {
            size_t new_size = buckets_.size() * 2;
            std::vector<std::list<T>> new_buckets(new_size);
            for (const auto& bucket : buckets_) {
                for (const auto& val : bucket) {
                    size_t idx = hash_(val) % new_size;
                    new_buckets[idx].push_back(val);
                }
            }
            buckets_ = std::move(new_buckets);
        }
    }

public:
    using value_type = T;
    using size_type = std::size_t;

    /**
     * @brief Default constructor — 16 empty buckets, zero elements.
     */
    UnorderedSet() : buckets_(DEFAULT_BUCKETS), size_(0) {}

    /**
     * @brief Construct from an initializer list; duplicates in the list are ignored.
     *
     * Each element is passed to insert(), which skips values already present.
     */
    UnorderedSet(std::initializer_list<T> init) : UnorderedSet() {
        for (const auto& val : init) {
            insert(val);
        }
    }

    /** @brief True when size_ == 0. */
    bool empty() const { return size_ == 0; }

    /** @brief Number of unique elements stored. */
    size_type size() const { return size_; }

    /**
     * @brief Remove all elements; bucket count stays unchanged.
     *
     * Clears every list in buckets_ and resets size_ to 0. The vector of buckets
     * is not shrunk — capacity is retained for fast re-fill without reallocation.
     */
    void clear() {
        for (auto& bucket : buckets_) {
            bucket.clear();
        }
        size_ = 0;
    }

    /**
     * @brief Insert value if not already present.
     * @return true if inserted, false if duplicate (set semantics).
     *
     *     (1) contains(value)?  ──yes──▶ return false
     *     (2) rehash_if_needed()        (may double buckets_)
     *     (3) idx = bucket_index(value)
     *     (4) buckets_[idx].push_back(value); ++size_
     *
     *     bucket[idx] before:  [a]──▶[b]
     *     after push_back(v):  [a]──▶[b]──▶[v]
     *
     * WHY check contains first: a set must reject duplicates; also avoids
     * inflating size_ and wasting chain nodes.
     */
    bool insert(const T& value) {
        if (contains(value)) return false;

        rehash_if_needed();
        size_t idx = bucket_index(value);
        buckets_[idx].push_back(value);
        ++size_;
        return true;
    }

    /**
     * @brief Remove one occurrence of value if present.
     * @return 1 if erased, 0 if not found (matches std::unordered_set::erase).
     *
     *     idx = bucket_index(value)
     *     walk buckets_[idx]:
     *         [x]──▶[value]──▶[y]
     *                    │
     *              list::erase(it)
     *                    ▼
     *         [x]──▶[y]     --size_
     *
     * Linear in chain length for that bucket; O(1) average when chains are short.
     */
    size_type erase(const T& value) {
        size_t idx = bucket_index(value);
        auto& bucket = buckets_[idx];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (*it == value) {
                bucket.erase(it);
                --size_;
                return 1;
            }
        }
        return 0;
    }

    /**
     * @brief Test membership by hashing to a bucket and scanning its chain.
     *
     *     idx = hash_(value) % N
     *     for v in buckets_[idx]:
     *         if v == value: return true
     *     return false
     *
     * Equality uses operator== on T; hash consistency requires equal objects
     * hash to the same bucket (same hash code is ideal but not required if
     * == finds the match in the chain).
     */
    bool contains(const T& value) const {
        size_t idx = bucket_index(value);
        const auto& bucket = buckets_[idx];

        for (const auto& val : bucket) {
            if (val == value) return true;
        }
        return false;
    }

    /**
     * @brief Set count of value: 0 or 1 (unique container semantics).
     */
    size_type count(const T& value) const {
        return contains(value) ? 1 : 0;
    }
};

#endif // UNORDERED_SET_HPP
