#ifndef STRING_HPP
#define STRING_HPP

#include <cstring>      // for strlen, strcpy, strcmp, etc.
#include <iostream>     // for ostream, istream
#include <algorithm>    // for min, max, swap
#include <stdexcept>    // for out_of_range, length_error
#include <iterator>     // for iterator traits
#include <cstddef>      // for size_t, ptrdiff_t

/**
 * @brief Custom implementation of std::string
 * 
 * String is a dynamic character array that manages its own memory.
 * 
 * Key characteristics:
 * - Dynamic memory allocation
 * - Automatic memory management (RAII)
 * - Copy and move semantics
 * - Rich set of string operations
 * - Iterator support
 * - SSO (Small String Optimization) for short strings
 */

class String {
public:
    // Type aliases (similar to std::string)
    using value_type = char;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = char&;
    using const_reference = const char&;
    using pointer = char*;
    using const_pointer = const char*;
    using iterator = char*;
    using const_iterator = const char*;

    // Special value for "not found"
    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    // Small String Optimization (SSO) threshold
    static constexpr size_type SSO_CAPACITY = 15;
    
    // Data members
    char* data_;        // Pointer to string data
    size_type size_;    // Current size (not including null terminator)
    size_type capacity_; // Allocated capacity (not including null terminator)
    
    // Small buffer for SSO
    char sso_buffer_[SSO_CAPACITY + 1];
    
    /**
     * @brief Check if string is using SSO
     */
    bool is_sso() const {
        return data_ == sso_buffer_;
    }
    
    /**
     * @brief Ensure capacity is at least n
     */
    void ensure_capacity(size_type n) {
        if (n > capacity_) {
            reserve(n);
        }
    }

public:
    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /**
     * @brief Default constructor - creates empty string
     */
    String() : data_(sso_buffer_), size_(0), capacity_(SSO_CAPACITY) {
        sso_buffer_[0] = '\0';
    }

    /**
     * @brief Construct from C-string
     */
    String(const char* s) : String() {
        if (s) {
            size_type len = std::strlen(s);
            if (len > SSO_CAPACITY) {
                capacity_ = len;
                data_ = new char[capacity_ + 1];
            }
            size_ = len;
            std::strcpy(data_, s);
        }
    }

    /**
     * @brief Construct with n copies of character c
     */
    String(size_type n, char c) : String() {
        if (n > SSO_CAPACITY) {
            capacity_ = n;
            data_ = new char[capacity_ + 1];
        }
        size_ = n;
        std::fill(data_, data_ + n, c);
        data_[size_] = '\0';
    }

    /**
     * @brief Construct from substring of another String
     */
    String(const String& other, size_type pos, size_type len = npos) : String() {
        if (pos > other.size_) {
            throw std::out_of_range("String: position out of range");
        }
        
        size_type actual_len = std::min(len, other.size_ - pos);
        if (actual_len > SSO_CAPACITY) {
            capacity_ = actual_len;
            data_ = new char[capacity_ + 1];
        }
        size_ = actual_len;
        std::memcpy(data_, other.data_ + pos, actual_len);
        data_[size_] = '\0';
    }

    /**
     * @brief Copy constructor
     */
    String(const String& other) : String() {
        if (other.size_ > SSO_CAPACITY) {
            capacity_ = other.size_;
            data_ = new char[capacity_ + 1];
        }
        size_ = other.size_;
        std::strcpy(data_, other.data_);
    }

    /**
     * @brief Move constructor
     */
    String(String&& other) noexcept : String() {
        if (other.is_sso()) {
            // Source uses SSO, must copy
            size_ = other.size_;
            std::strcpy(sso_buffer_, other.sso_buffer_);
        } else {
            // Steal the heap allocation
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            // Reset other to SSO
            other.data_ = other.sso_buffer_;
            other.size_ = 0;
            other.capacity_ = SSO_CAPACITY;
            other.sso_buffer_[0] = '\0';
        }
    }

    // ============================================================================
    // DESTRUCTOR
    // ============================================================================

    /**
     * @brief Destructor - frees allocated memory
     */
    ~String() {
        if (!is_sso()) {
            delete[] data_;
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Copy assignment
     */
    String& operator=(const String& other) {
        if (this != &other) {
            String temp(other);
            swap(temp);
        }
        return *this;
    }

    /**
     * @brief Move assignment
     */
    String& operator=(String&& other) noexcept {
        if (this != &other) {
            // Clean up current data
            if (!is_sso()) {
                delete[] data_;
            }
            
            if (other.is_sso()) {
                // Source uses SSO, must copy
                data_ = sso_buffer_;
                size_ = other.size_;
                capacity_ = SSO_CAPACITY;
                std::strcpy(sso_buffer_, other.sso_buffer_);
            } else {
                // Steal the heap allocation
                data_ = other.data_;
                size_ = other.size_;
                capacity_ = other.capacity_;
                
                // Reset other to SSO
                other.data_ = other.sso_buffer_;
                other.size_ = 0;
                other.capacity_ = SSO_CAPACITY;
                other.sso_buffer_[0] = '\0';
            }
        }
        return *this;
    }

    /**
     * @brief Assignment from C-string
     */
    String& operator=(const char* s) {
        String temp(s);
        swap(temp);
        return *this;
    }

    /**
     * @brief Assignment from single character
     */
    String& operator=(char c) {
        String temp(1, c);
        swap(temp);
        return *this;
    }

    // ============================================================================
    // ELEMENT ACCESS
    // ============================================================================

    /**
     * @brief Access character at position (with bounds checking)
     */
    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("String::at: position out of range");
        }
        return data_[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("String::at: position out of range");
        }
        return data_[pos];
    }

    /**
     * @brief Access character at position (no bounds checking)
     */
    reference operator[](size_type pos) {
        return data_[pos];
    }

    const_reference operator[](size_type pos) const {
        return data_[pos];
    }

    /**
     * @brief Access first character
     */
    reference front() {
        return data_[0];
    }

    const_reference front() const {
        return data_[0];
    }

    /**
     * @brief Access last character
     */
    reference back() {
        return data_[size_ - 1];
    }

    const_reference back() const {
        return data_[size_ - 1];
    }

    /**
     * @brief Get C-string
     */
    const char* c_str() const {
        return data_;
    }

    const char* data() const {
        return data_;
    }

    // ============================================================================
    // ITERATORS
    // ============================================================================

    iterator begin() { return data_; }
    const_iterator begin() const { return data_; }
    const_iterator cbegin() const { return data_; }

    iterator end() { return data_ + size_; }
    const_iterator end() const { return data_ + size_; }
    const_iterator cend() const { return data_ + size_; }

    // ============================================================================
    // CAPACITY
    // ============================================================================

    /**
     * @brief Check if string is empty
     */
    bool empty() const {
        return size_ == 0;
    }

    /**
     * @brief Get string size
     */
    size_type size() const {
        return size_;
    }

    size_type length() const {
        return size_;
    }

    /**
     * @brief Get current capacity
     */
    size_type capacity() const {
        return capacity_;
    }

    /**
     * @brief Reserve memory for at least n characters
     */
    void reserve(size_type n) {
        if (n <= capacity_) {
            return;
        }
        
        // Allocate new buffer
        char* new_data = new char[n + 1];
        std::strcpy(new_data, data_);
        
        // Delete old buffer if not SSO
        if (!is_sso()) {
            delete[] data_;
        }
        
        data_ = new_data;
        capacity_ = n;
    }

    /**
     * @brief Reduce capacity to fit size
     */
    void shrink_to_fit() {
        if (capacity_ == size_ || is_sso()) {
            return;
        }
        
        if (size_ <= SSO_CAPACITY) {
            // Can move to SSO
            std::strcpy(sso_buffer_, data_);
            delete[] data_;
            data_ = sso_buffer_;
            capacity_ = SSO_CAPACITY;
        } else {
            // Allocate exact size
            char* new_data = new char[size_ + 1];
            std::strcpy(new_data, data_);
            delete[] data_;
            data_ = new_data;
            capacity_ = size_;
        }
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Clear string contents
     */
    void clear() {
        size_ = 0;
        data_[0] = '\0';
    }

    /**
     * @brief Append string
     */
    String& append(const String& str) {
        return append(str.data_, str.size_);
    }

    String& append(const char* s) {
        if (s) {
            return append(s, std::strlen(s));
        }
        return *this;
    }

    String& append(const char* s, size_type n) {
        if (n == 0) return *this;
        
        ensure_capacity(size_ + n);
        std::memcpy(data_ + size_, s, n);
        size_ += n;
        data_[size_] = '\0';
        return *this;
    }

    String& append(size_type n, char c) {
        if (n == 0) return *this;
        
        ensure_capacity(size_ + n);
        std::fill(data_ + size_, data_ + size_ + n, c);
        size_ += n;
        data_[size_] = '\0';
        return *this;
    }

    /**
     * @brief Append operators
     */
    String& operator+=(const String& str) {
        return append(str);
    }

    String& operator+=(const char* s) {
        return append(s);
    }

    String& operator+=(char c) {
        return append(1, c);
    }

    /**
     * @brief Push back single character
     */
    void push_back(char c) {
        append(1, c);
    }

    /**
     * @brief Pop back last character
     */
    void pop_back() {
        if (size_ > 0) {
            --size_;
            data_[size_] = '\0';
        }
    }

    /**
     * @brief Insert string at position
     */
    String& insert(size_type pos, const String& str) {
        return insert(pos, str.data_, str.size_);
    }

    String& insert(size_type pos, const char* s) {
        if (s) {
            return insert(pos, s, std::strlen(s));
        }
        return *this;
    }

    String& insert(size_type pos, const char* s, size_type n) {
        if (pos > size_) {
            throw std::out_of_range("String::insert: position out of range");
        }
        
        if (n == 0) return *this;
        
        ensure_capacity(size_ + n);
        
        // Move existing characters
        std::memmove(data_ + pos + n, data_ + pos, size_ - pos + 1);
        
        // Copy new characters
        std::memcpy(data_ + pos, s, n);
        size_ += n;
        
        return *this;
    }

    /**
     * @brief Erase characters
     */
    String& erase(size_type pos = 0, size_type len = npos) {
        if (pos > size_) {
            throw std::out_of_range("String::erase: position out of range");
        }
        
        size_type actual_len = std::min(len, size_ - pos);
        std::memmove(data_ + pos, data_ + pos + actual_len, size_ - pos - actual_len + 1);
        size_ -= actual_len;
        
        return *this;
    }

    /**
     * @brief Replace substring
     */
    String& replace(size_type pos, size_type len, const String& str) {
        return replace(pos, len, str.data_, str.size_);
    }

    String& replace(size_type pos, size_type len, const char* s, size_type n) {
        if (pos > size_) {
            throw std::out_of_range("String::replace: position out of range");
        }
        
        size_type actual_len = std::min(len, size_ - pos);
        
        if (n == actual_len) {
            // Same size, just copy
            std::memcpy(data_ + pos, s, n);
        } else if (n < actual_len) {
            // Replacement is smaller
            std::memcpy(data_ + pos, s, n);
            std::memmove(data_ + pos + n, data_ + pos + actual_len, 
                        size_ - pos - actual_len + 1);
            size_ -= (actual_len - n);
        } else {
            // Replacement is larger
            ensure_capacity(size_ + (n - actual_len));
            std::memmove(data_ + pos + n, data_ + pos + actual_len,
                        size_ - pos - actual_len + 1);
            std::memcpy(data_ + pos, s, n);
            size_ += (n - actual_len);
        }
        
        return *this;
    }

    /**
     * @brief Resize string
     */
    void resize(size_type n, char c = '\0') {
        if (n < size_) {
            size_ = n;
            data_[size_] = '\0';
        } else if (n > size_) {
            ensure_capacity(n);
            std::fill(data_ + size_, data_ + n, c);
            size_ = n;
            data_[size_] = '\0';
        }
    }

    /**
     * @brief Swap with another string
     */
    void swap(String& other) noexcept {
        // Handle SSO cases
        bool this_sso = is_sso();
        bool other_sso = other.is_sso();
        
        if (this_sso && other_sso) {
            // Both use SSO - swap buffers
            char temp[SSO_CAPACITY + 1];
            std::strcpy(temp, sso_buffer_);
            std::strcpy(sso_buffer_, other.sso_buffer_);
            std::strcpy(other.sso_buffer_, temp);
        } else if (!this_sso && !other_sso) {
            // Both use heap - swap pointers
            std::swap(data_, other.data_);
            std::swap(capacity_, other.capacity_);
        } else if (this_sso && !other_sso) {
            // This is SSO, other is heap
            char temp[SSO_CAPACITY + 1];
            std::strcpy(temp, sso_buffer_);
            data_ = other.data_;
            capacity_ = other.capacity_;
            other.data_ = other.sso_buffer_;
            other.capacity_ = SSO_CAPACITY;
            std::strcpy(other.sso_buffer_, temp);
        } else {
            // This is heap, other is SSO
            char temp[SSO_CAPACITY + 1];
            std::strcpy(temp, other.sso_buffer_);
            other.data_ = data_;
            other.capacity_ = capacity_;
            data_ = sso_buffer_;
            capacity_ = SSO_CAPACITY;
            std::strcpy(sso_buffer_, temp);
        }
        
        std::swap(size_, other.size_);
    }

    // ============================================================================
    // STRING OPERATIONS
    // ============================================================================

    /**
     * @brief Find substring
     */
    size_type find(const String& str, size_type pos = 0) const {
        return find(str.data_, pos, str.size_);
    }

    size_type find(const char* s, size_type pos = 0) const {
        if (s) {
            return find(s, pos, std::strlen(s));
        }
        return npos;
    }

    size_type find(const char* s, size_type pos, size_type n) const {
        if (n == 0) return pos <= size_ ? pos : npos;
        if (pos + n > size_) return npos;
        
        const char* found = std::strstr(data_ + pos, s);
        if (found && std::strlen(found) >= n) {
            return found - data_;
        }
        return npos;
    }

    size_type find(char c, size_type pos = 0) const {
        if (pos >= size_) return npos;
        
        const char* found = std::strchr(data_ + pos, c);
        return found ? (found - data_) : npos;
    }

    /**
     * @brief Find last occurrence
     */
    size_type rfind(char c, size_type pos = npos) const {
        if (size_ == 0) return npos;
        
        size_type actual_pos = std::min(pos, size_ - 1);
        for (size_type i = actual_pos + 1; i > 0; --i) {
            if (data_[i - 1] == c) {
                return i - 1;
            }
        }
        return npos;
    }

    /**
     * @brief Get substring
     */
    String substr(size_type pos = 0, size_type len = npos) const {
        return String(*this, pos, len);
    }

    /**
     * @brief Compare with another string
     */
    int compare(const String& str) const {
        return std::strcmp(data_, str.data_);
    }

    int compare(const char* s) const {
        return std::strcmp(data_, s);
    }

    // ============================================================================
    // NON-MEMBER FUNCTIONS (as friends)
    // ============================================================================

    friend String operator+(const String& lhs, const String& rhs);
    friend String operator+(const String& lhs, const char* rhs);
    friend String operator+(const char* lhs, const String& rhs);
    friend String operator+(const String& lhs, char rhs);
    friend String operator+(char lhs, const String& rhs);

    friend bool operator==(const String& lhs, const String& rhs);
    friend bool operator==(const String& lhs, const char* rhs);
    friend bool operator==(const char* lhs, const String& rhs);

    friend bool operator!=(const String& lhs, const String& rhs);
    friend bool operator<(const String& lhs, const String& rhs);
    friend bool operator<=(const String& lhs, const String& rhs);
    friend bool operator>(const String& lhs, const String& rhs);
    friend bool operator>=(const String& lhs, const String& rhs);

    friend std::ostream& operator<<(std::ostream& os, const String& str);
    friend std::istream& operator>>(std::istream& is, String& str);
};

// ============================================================================
// NON-MEMBER FUNCTION IMPLEMENTATIONS
// ============================================================================

inline String operator+(const String& lhs, const String& rhs) {
    String result;
    result.reserve(lhs.size_ + rhs.size_);
    result.append(lhs);
    result.append(rhs);
    return result;
}

inline String operator+(const String& lhs, const char* rhs) {
    String result(lhs);
    result.append(rhs);
    return result;
}

inline String operator+(const char* lhs, const String& rhs) {
    String result(lhs);
    result.append(rhs);
    return result;
}

inline String operator+(const String& lhs, char rhs) {
    String result(lhs);
    result.push_back(rhs);
    return result;
}

inline String operator+(char lhs, const String& rhs) {
    String result(1, lhs);
    result.append(rhs);
    return result;
}

inline bool operator==(const String& lhs, const String& rhs) {
    return lhs.size_ == rhs.size_ && std::strcmp(lhs.data_, rhs.data_) == 0;
}

inline bool operator==(const String& lhs, const char* rhs) {
    return std::strcmp(lhs.data_, rhs) == 0;
}

inline bool operator==(const char* lhs, const String& rhs) {
    return std::strcmp(lhs, rhs.data_) == 0;
}

inline bool operator!=(const String& lhs, const String& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(const String& lhs, const String& rhs) {
    return std::strcmp(lhs.data_, rhs.data_) < 0;
}

inline bool operator<=(const String& lhs, const String& rhs) {
    return !(rhs < lhs);
}

inline bool operator>(const String& lhs, const String& rhs) {
    return rhs < lhs;
}

inline bool operator>=(const String& lhs, const String& rhs) {
    return !(lhs < rhs);
}

inline std::ostream& operator<<(std::ostream& os, const String& str) {
    return os << str.data_;
}

inline std::istream& operator>>(std::istream& is, String& str) {
    str.clear();
    char c;
    
    // Skip whitespace
    while (is.get(c) && std::isspace(c));
    
    if (is) {
        is.putback(c);
        while (is.get(c) && !std::isspace(c)) {
            str.push_back(c);
        }
    }
    
    return is;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

inline void swap(String& lhs, String& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // STRING_HPP

