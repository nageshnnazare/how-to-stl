#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#include <stdexcept>
#include <utility>

/**
 * @brief Optional value (may or may not contain a value)
 * C++17 feature for safer null handling
 */
template<typename T>
class Optional {
private:
    alignas(T) unsigned char storage_[sizeof(T)];
    bool has_value_;
    
    T* ptr() { return reinterpret_cast<T*>(storage_); }
    const T* ptr() const { return reinterpret_cast<const T*>(storage_); }
    
public:
    Optional() : has_value_(false) {}
    
    Optional(const T& value) : has_value_(true) {
        new (storage_) T(value);
    }
    
    Optional(T&& value) : has_value_(true) {
        new (storage_) T(std::move(value));
    }
    
    ~Optional() {
        if (has_value_) ptr()->~T();
    }
    
    Optional(const Optional& other) : has_value_(other.has_value_) {
        if (has_value_) new (storage_) T(*other.ptr());
    }
    
    Optional(Optional&& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (storage_) T(std::move(*other.ptr()));
            other.reset();
        }
    }
    
    Optional& operator=(const Optional& other) {
        if (this != &other) {
            if (has_value_) ptr()->~T();
            has_value_ = other.has_value_;
            if (has_value_) new (storage_) T(*other.ptr());
        }
        return *this;
    }
    
    bool has_value() const { return has_value_; }
    explicit operator bool() const { return has_value_; }
    
    T& value() {
        if (!has_value_) throw std::runtime_error("Optional::value: no value");
        return *ptr();
    }
    
    const T& value() const {
        if (!has_value_) throw std::runtime_error("Optional::value: no value");
        return *ptr();
    }
    
    T value_or(const T& default_value) const {
        return has_value_ ? *ptr() : default_value;
    }
    
    T& operator*() { return *ptr(); }
    const T& operator*() const { return *ptr(); }
    T* operator->() { return ptr(); }
    const T* operator->() const { return ptr(); }
    
    void reset() {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }
};

#endif // OPTIONAL_HPP
