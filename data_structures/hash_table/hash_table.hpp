#ifndef DS_HASH_TABLE_HPP
#define DS_HASH_TABLE_HPP

// ============================================================================
//  HashTable<K, V> -- separate chaining (buckets + linked lists of entries)
// ============================================================================
//
// THE IDEA
// --------
// A hash table maps keys to values in *expected* O(1) time. We hash each key
// to an integer, reduce it modulo the number of buckets, and store the entry
// in that bucket. When two keys land in the same bucket we have a *collision*;
// separate chaining resolves it by keeping a short list of entries per bucket.
//
//     hash("cat") % 4 == 2        hash("dog") % 4 == 2  (collision!)
//
//     bucket:   0      1      2              3
//             +----+ +----+ +----------+   +----+
//             |    | |    | | cat:3    |   |    |
//             +----+ +----+ | dog:7    |   +----+
//                           +----------+
//                                ^
//                           same bucket -> chain
//
// LOAD FACTOR AND REHASHING
// -------------------------
// load_factor = size / bucket_count. As more entries pack in, chains grow
// longer and lookups slow down. When load_factor exceeds max_load_factor_
// (default 0.75), we *rehash*: allocate roughly twice as many buckets and
// redistribute every entry into its new bucket. Amortized cost stays O(1).
//
// COMPLEXITY (average case, good hash function)
//     put / get / contains / erase ... O(1) amortized
//     worst case (all keys collide) ... O(N) per operation
// ============================================================================

#include <cstddef>      // std::size_t
#include <functional>   // std::hash
#include <vector>

namespace ds {

template <typename K, typename V>
class HashTable {
public:
    using key_type = K;
    using mapped_type = V;

    // Start with `initial_buckets` empty chains (default 8).
    explicit HashTable(std::size_t initial_buckets = 8, double max_load = 0.75)
        : buckets_(initial_buckets > 0 ? initial_buckets : 8),
          size_(0),
          max_load_factor_(max_load) {}

    // ---- size / capacity ---------------------------------------------------

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    std::size_t bucket_count() const { return buckets_.size(); }
    double load_factor() const {
        return buckets_.empty() ? 0.0 : static_cast<double>(size_) / buckets_.size();
    }
    double max_load_factor() const { return max_load_factor_; }

    // ---- lookup ------------------------------------------------------------

    // Returns true if `key` is present.
    bool contains(const K& key) const {
        return find_entry(key) != nullptr;
    }

    // Returns a pointer to the stored value, or nullptr if absent.
    // (Callers who need a hard error can check for nullptr and throw.)
    const V* get(const K& key) const {
        const Entry* e = find_entry(key);
        return e ? &e->value : nullptr;
    }

    V* get(const K& key) {
        Entry* e = find_entry(key);
        return e ? &e->value : nullptr;
    }

    // ---- modifiers ---------------------------------------------------------

    // Insert or update: if `key` exists, overwrite its value; else add it.
    // Rehashes automatically when load factor is too high.
    void put(const K& key, const V& value) {
        Entry* e = find_entry(key);
        if (e) {
            e->value = value;
            return;
        }
        if (needs_grow()) grow();
        buckets_[bucket_index(key)].push_back({key, value});
        ++size_;
    }

    // Remove `key` if present. Returns true when something was erased.
    bool erase(const K& key) {
        auto& chain = buckets_[bucket_index(key)];
        for (auto it = chain.begin(); it != chain.end(); ++it) {
            if (it->key == key) {
                chain.erase(it);
                --size_;
                return true;
            }
        }
        return false;
    }

    void clear() {
        for (auto& chain : buckets_) chain.clear();
        size_ = 0;
    }

private:
    struct Entry {
        K key;
        V value;
    };

    std::vector<std::vector<Entry>> buckets_;
    std::size_t size_;
    double max_load_factor_;

    std::size_t bucket_index(const K& key) const {
        return std::hash<K>{}(key) % buckets_.size();
    }

    bool needs_grow() const {
        return load_factor() >= max_load_factor_;
    }

    // Double bucket count and redistribute every entry.
    void grow() {
        std::size_t new_count = buckets_.size() * 2;
        std::vector<std::vector<Entry>> fresh(new_count);
        for (auto& chain : buckets_) {
            for (auto& e : chain) {
                std::size_t idx = std::hash<K>{}(e.key) % new_count;
                fresh[idx].push_back(std::move(e));
            }
        }
        buckets_ = std::move(fresh);
    }

    Entry* find_entry(const K& key) {
        auto& chain = buckets_[bucket_index(key)];
        for (auto& e : chain)
            if (e.key == key) return &e;
        return nullptr;
    }

    const Entry* find_entry(const K& key) const {
        const auto& chain = buckets_[bucket_index(key)];
        for (const auto& e : chain)
            if (e.key == key) return &e;
        return nullptr;
    }
};

}  // namespace ds

#endif  // DS_HASH_TABLE_HPP
