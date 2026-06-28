#ifndef UNORDERED_MAP_HPP
#define UNORDERED_MAP_HPP

#include <cstddef>           // for size_t
#include <vector>            // bucket array
#include <list>              // collision chains
#include <utility>           // for std::pair
#include <functional>        // for std::hash
#include <initializer_list>  // for initializer_list
#include <stdexcept>         // for out_of_range

// ============================================================================
//  UnorderedMap<Key,T> -- a hand-rolled std::unordered_map (hash + chaining)
// ============================================================================
//
// WHAT IT IS
// ----------
// An UnorderedMap stores UNIQUE keys mapped to values (Key → T). Like
// UnorderedSet, it is a hash table with separate chaining: each bucket holds a
// linked list of pair<Key,T> nodes. Lookup by key is O(1) average; no key order.
//
// THE THREE FIELDS
// ----------------
//
//     buckets_  -> vector of lists of pair<Key,T> (each node is key + value)
//     size_     -> number of key-value pairs stored
//     hash_     -> callable mapping Key -> size_t (only the KEY is hashed)
//
// Invariant: size_ equals total nodes across all chains; keys are unique.
//
// MEMORY LAYOUT (bucket array + chains of pair nodes)
// -------------------------------------------------
//
//     UnorderedMap object                 buckets_[i] chains hold PAIRS:
//     ┌─────────────────────┐
//     │ buckets_   ●────────┼──▶  [ ]  [("bob",25)]──▶[("eve",31)]  [ ]  ...
//     │ size_      = 3      │         empty    bucket 2 chain
//     │ hash_              │
//     └─────────────────────┘
//
//     bucket 2 chain detail:
//         ┌──────────┐    ┌──────────┐
//         │ bob → 25 │───▶│ eve → 31 │
//         └──────────┘    └──────────┘
//
// HASHING (key only — the value is not hashed)
// --------------------------------------------
//     idx = hash_(key) % buckets_.size()
//
//     key "alice"  ──hash_──▶ h  ──% N──▶ bucket idx  ──▶ scan pairs for .first == "alice"
//
// LOAD FACTOR AND REHASH
// ----------------------
//     load_factor = size_ / buckets_.size()
//     when size_ > buckets_.size() * 2  →  double buckets, re-hash every pair by key
//
//     BEFORE (4 buckets)              AFTER (8 buckets, each pair re-inserted)
//     [("a",1)] [("b",2),("c",3)] ...   pairs scatter into new slots by hash(key)%8
//
// Key characteristics:
// - operator[] inserts default-constructed T if key missing (like std::map)
// - at() throws if key missing (bounds-checked access)
// - insert(pair) rejects duplicate keys
// - Separate chaining: colliding keys share a bucket list
// ============================================================================

template<typename Key, typename T, typename Hash = std::hash<Key>>
class UnorderedMap {
private:
    using value_type_internal = std::pair<Key, T>;
    std::vector<std::list<value_type_internal>> buckets_;  // bucket array of pair chains
    size_t size_;                                          // number of key-value pairs
    Hash hash_;                                            // hashes Key only
    static constexpr size_t DEFAULT_BUCKETS = 16;

    /**
     * @brief Map key to bucket index: hash(key) % bucket_count.
     *
     *     key k  ──hash_──▶ h  ──% buckets_.size()──▶ idx  ──▶ buckets_[idx]
     *
     * Values are never hashed; they ride along in the pair node once the key matches.
     */
    size_t bucket_index(const Key& key) const {
        return hash_(key) % buckets_.size();
    }

    /**
     * @brief Double bucket count when load factor exceeds 2.0; re-hash every pair by key.
     *
     *     for each pair (k,v) in old table:
     *         new_idx = hash_(k) % (2 * old_bucket_count)
     *         new_buckets[new_idx].push_back({k,v})
     *
     * WHY: same as UnorderedSet — shorter chains restore average O(1) lookups.
     * Modulus change forces full re-insertion, not in-place bucket split.
     */
    void rehash_if_needed() {
        if (size_ > buckets_.size() * 2) {
            size_t new_size = buckets_.size() * 2;
            std::vector<std::list<value_type_internal>> new_buckets(new_size);
            for (const auto& bucket : buckets_) {
                for (const auto& pair : bucket) {
                    size_t idx = hash_(pair.first) % new_size;
                    new_buckets[idx].push_back(pair);
                }
            }
            buckets_ = std::move(new_buckets);
        }
    }

public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;

    /** @brief Default constructor — 16 empty bucket chains. */
    UnorderedMap() : buckets_(DEFAULT_BUCKETS), size_(0) {}

    /**
     * @brief Construct from initializer list of pairs; duplicate keys use first wins via insert.
     */
    UnorderedMap(std::initializer_list<value_type> init) : UnorderedMap() {
        for (const auto& p : init) {
            insert(p);
        }
    }

    /** @brief True when no pairs stored. */
    bool empty() const { return size_ == 0; }

    /** @brief Number of key-value pairs. */
    size_type size() const { return size_; }

    /**
     * @brief Remove all pairs from all chains; bucket vector size unchanged.
     */
    void clear() {
        for (auto& bucket : buckets_) {
            bucket.clear();
        }
        size_ = 0;
    }

    /**
     * @brief Subscript: return reference to mapped value; insert {key, T()} if absent.
     *
     *     (1) idx = bucket_index(key)
     *     (2) scan chain for pair.first == key  →  if found, return pair.second
     *     (3) rehash_if_needed()
     *     (4) push_back({key, T()}); ++size_; return back().second
     *
     *     buckets_[idx]:  [("a",1)]──▶[("b",2)]
     *     operator[]("c") when missing:
     *         ... push {c, T()}  →  [("a",1)]──▶[("b",2)]──▶[("c",0)]
     *
     * WHY default-insert: matches std::unordered_map — convenient aggregation
     * (map[key] += 1). Inserts invisible to caller when key was missing.
     */
    T& operator[](const Key& key) {
        // (a) Look for an existing entry first.
        for (auto& pair : buckets_[bucket_index(key)]) {
            if (pair.first == key) {
                return pair.second;
            }
        }

        // (b) Missing: grow BEFORE caching a bucket reference. rehash_if_needed()
        //     may replace the whole bucket vector (buckets_ = std::move(...)),
        //     which would dangle any reference/index captured beforehand. So we
        //     recompute the index after a possible rehash, then insert.
        rehash_if_needed();
        size_t idx = bucket_index(key);
        buckets_[idx].push_back({key, T()});
        ++size_;
        return buckets_[idx].back().second;
    }

    /**
     * @brief Bounds-checked access; throws std::out_of_range if key not found.
     *
     * Unlike operator[], never inserts. Use when missing key is a logic error.
     */
    T& at(const Key& key) {
        size_t idx = bucket_index(key);
        auto& bucket = buckets_[idx];

        for (auto& pair : bucket) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        throw std::out_of_range("UnorderedMap::at: key not found");
    }

    /** @brief const overload of at() — same throw semantics. */
    const T& at(const Key& key) const {
        size_t idx = bucket_index(key);
        const auto& bucket = buckets_[idx];

        for (const auto& pair : bucket) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        throw std::out_of_range("UnorderedMap::at: key not found");
    }

    /**
     * @brief Insert key-value pair if key not already present.
     * @return false if key exists (no overwrite).
     *
     *     contains(key)?  yes → return false
     *     else rehash, push_back on chain, ++size_, return true
     *
     * WHY no overwrite on duplicate: map unique-key semantics; use operator[] or
     * erase+insert to change an existing value deliberately.
     */
    bool insert(const value_type& value) {
        if (contains(value.first)) return false;

        rehash_if_needed();
        size_t idx = bucket_index(value.first);
        buckets_[idx].push_back({value.first, value.second});
        ++size_;
        return true;
    }

    /**
     * @brief Remove pair with given key if present; return 0 or 1.
     *
     * Walk bucket chain comparing pair.first to key; list::erase on match.
     */
    size_type erase(const Key& key) {
        size_t idx = bucket_index(key);
        auto& bucket = buckets_[idx];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                --size_;
                return 1;
            }
        }
        return 0;
    }

    /**
     * @brief Test whether key exists in any chain.
     *
     *     idx = hash(key) % N; linear scan for pair.first == key
     */
    bool contains(const Key& key) const {
        size_t idx = bucket_index(key);
        const auto& bucket = buckets_[idx];

        for (const auto& pair : bucket) {
            if (pair.first == key) return true;
        }
        return false;
    }

    /** @brief Map count: 0 or 1 for unique keys. */
    size_type count(const Key& key) const {
        return contains(key) ? 1 : 0;
    }
};

#endif // UNORDERED_MAP_HPP
