#ifndef UNORDERED_SET_HPP
#define UNORDERED_SET_HPP

#include <cstddef>
#include <vector>
#include <list>
#include <functional>
#include <initializer_list>

/**
 * @brief UnorderedSet implementation using hash table (separate chaining)
 * O(1) average case for insert/find/erase
 */
template<typename T, typename Hash = std::hash<T>>
class UnorderedSet {
private:
    std::vector<std::list<T>> buckets_;
    size_t size_;
    Hash hash_;
    static constexpr size_t DEFAULT_BUCKETS = 16;
    
    size_t bucket_index(const T& value) const {
        return hash_(value) % buckets_.size();
    }
    
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
    
    UnorderedSet() : buckets_(DEFAULT_BUCKETS), size_(0) {}
    
    UnorderedSet(std::initializer_list<T> init) : UnorderedSet() {
        for (const auto& val : init) {
            insert(val);
        }
    }
    
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    
    void clear() {
        for (auto& bucket : buckets_) {
            bucket.clear();
        }
        size_ = 0;
    }
    
    bool insert(const T& value) {
        if (contains(value)) return false;
        
        rehash_if_needed();
        size_t idx = bucket_index(value);
        buckets_[idx].push_back(value);
        ++size_;
        return true;
    }
    
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
    
    bool contains(const T& value) const {
        size_t idx = bucket_index(value);
        const auto& bucket = buckets_[idx];
        
        for (const auto& val : bucket) {
            if (val == value) return true;
        }
        return false;
    }
    
    size_type count(const T& value) const {
        return contains(value) ? 1 : 0;
    }
};

#endif // UNORDERED_SET_HPP
