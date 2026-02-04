#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cstddef>
#include <stdexcept>

/**
 * @brief Fixed-size array wrapper
 * Stack-allocated, size known at compile time
 */
template<typename T, std::size_t N>
struct Array {
    T data_[N];
    
    using value_type = T;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    
    T& operator[](size_type i) { return data_[i]; }
    const T& operator[](size_type i) const { return data_[i]; }
    
    T& at(size_type i) {
        if (i >= N) throw std::out_of_range("Array::at");
        return data_[i];
    }
    
    const T& at(size_type i) const {
        if (i >= N) throw std::out_of_range("Array::at");
        return data_[i];
    }
    
    T& front() { return data_[0]; }
    const T& front() const { return data_[0]; }
    T& back() { return data_[N-1]; }
    const T& back() const { return data_[N-1]; }
    
    iterator begin() { return data_; }
    const_iterator begin() const { return data_; }
    iterator end() { return data_ + N; }
    const_iterator end() const { return data_ + N; }
    
    constexpr size_type size() const { return N; }
    constexpr bool empty() const { return N == 0; }
    
    void fill(const T& value) {
        for (size_type i = 0; i < N; ++i) data_[i] = value;
    }
};

#endif // ARRAY_HPP
