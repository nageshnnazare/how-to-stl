#ifndef UNORDERED_MAP_HPP
#define UNORDERED_MAP_HPP

#include <cstddef>
#include <vector>
#include <list>
#include <utility>
#include <functional>
#include <initializer_list>
#include <stdexcept>

/**
 * @brief UnorderedMap implementation using hash table (separate chaining)
 * O(1) average case for insert/find/erase
 */
template<typename Key, typename T, typename Hash = std::hash<Key>>
class UnorderedMap {
private:
    using value_type_internal = std::pair<Key, T>;
    std::vector<std::list<value_type_internal>> buckets_;
    size_t size_;
    Hash hash_;
    static constexpr size_t DEFAULT_BUCKETS = 16;
    
    size_t bucket_index(const Key& key) const {
        return hash_(key) % buckets_.size();
    }
    
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
    
    UnorderedMap() : buckets_(DEFAULT_BUCKETS), size_(0) {}
    
    UnorderedMap(std::initializer_list<value_type> init) : UnorderedMap() {
        for (const auto& p : init) {
            insert(p);
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
    
    T& operator[](const Key& key) {
        size_t idx = bucket_index(key);
        auto& bucket = buckets_[idx];
        
        for (auto& pair : bucket) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        
        rehash_if_needed();
        bucket.push_back({key, T()});
        ++size_;
        return bucket.back().second;
    }
    
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
    
    bool insert(const value_type& value) {
        if (contains(value.first)) return false;
        
        rehash_if_needed();
        size_t idx = bucket_index(value.first);
        buckets_[idx].push_back({value.first, value.second});
        ++size_;
        return true;
    }
    
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
    
    bool contains(const Key& key) const {
        size_t idx = bucket_index(key);
        const auto& bucket = buckets_[idx];
        
        for (const auto& pair : bucket) {
            if (pair.first == key) return true;
        }
        return false;
    }
    
    size_type count(const Key& key) const {
        return contains(key) ? 1 : 0;
    }
};

#endif // UNORDERED_MAP_HPP
