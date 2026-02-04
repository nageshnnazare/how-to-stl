#ifndef MULTISET_HPP
#define MULTISET_HPP

#include <cstddef>
#include <vector>
#include <algorithm>
#include <functional>
#include <initializer_list>

/**
 * @brief Multiset implementation (allows duplicates)
 * Uses sorted vector for simplicity - O(n) insert, O(log n) search
 */
template<typename T, typename Compare = std::less<T>>
class Multiset {
private:
    std::vector<T> data_;
    Compare comp_;
    
public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = typename std::vector<T>::const_iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    
    Multiset() = default;
    
    Multiset(std::initializer_list<T> init) {
        for (const auto& val : init) {
            insert(val);
        }
    }
    
    iterator begin() const { return data_.begin(); }
    iterator end() const { return data_.end(); }
    
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    
    void clear() { data_.clear(); }
    
    iterator insert(const T& value) {
        auto pos = std::lower_bound(data_.begin(), data_.end(), value, comp_);
        return data_.insert(pos, value);
    }
    
    size_type erase(const T& value) {
        auto range = std::equal_range(data_.begin(), data_.end(), value, comp_);
        size_type count = std::distance(range.first, range.second);
        data_.erase(range.first, range.second);
        return count;
    }
    
    iterator find(const T& value) const {
        auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
        if (it != data_.end() && !comp_(value, *it) && !comp_(*it, value)) {
            return it;
        }
        return data_.end();
    }
    
    size_type count(const T& value) const {
        auto range = std::equal_range(data_.begin(), data_.end(), value, comp_);
        return std::distance(range.first, range.second);
    }
    
    bool contains(const T& value) const {
        return find(value) != data_.end();
    }
};

#endif // MULTISET_HPP
