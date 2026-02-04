#ifndef MULTIMAP_HPP
#define MULTIMAP_HPP

#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <initializer_list>

/**
 * @brief Multimap implementation (allows duplicate keys)
 * Uses sorted vector of pairs for simplicity
 */
template<typename Key, typename T, typename Compare = std::less<Key>>
class Multimap {
private:
    using value_type_internal = std::pair<Key, T>;
    std::vector<value_type_internal> data_;
    Compare comp_;
    
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
    
    Multimap() = default;
    
    Multimap(std::initializer_list<value_type> init) {
        for (const auto& p : init) {
            insert(p);
        }
    }
    
    iterator begin() const { return data_.begin(); }
    iterator end() const { return data_.end(); }
    
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    
    void clear() { data_.clear(); }
    
    iterator insert(const value_type& value) {
        KeyCompare key_comp{comp_};
        auto pos = std::lower_bound(data_.begin(), data_.end(), value.first, key_comp);
        return data_.insert(pos, value_type_internal(value.first, value.second));
    }
    
    size_type erase(const Key& key) {
        KeyCompare key_comp{comp_};
        auto range = std::equal_range(data_.begin(), data_.end(), key, key_comp);
        size_type count = std::distance(range.first, range.second);
        data_.erase(range.first, range.second);
        return count;
    }
    
    iterator find(const Key& key) const {
        KeyCompare key_comp{comp_};
        auto it = std::lower_bound(data_.begin(), data_.end(), key, key_comp);
        if (it != data_.end() && !comp_(key, it->first) && !comp_(it->first, key)) {
            return it;
        }
        return data_.end();
    }
    
    size_type count(const Key& key) const {
        KeyCompare key_comp{comp_};
        auto range = std::equal_range(data_.begin(), data_.end(), key, key_comp);
        return std::distance(range.first, range.second);
    }
    
    bool contains(const Key& key) const {
        return find(key) != data_.end();
    }
};

#endif // MULTIMAP_HPP
