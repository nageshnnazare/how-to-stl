#ifndef STRING_HPP
#define STRING_HPP

#include <cstring>      // for strlen, strcpy, strcmp, etc.
#include <iostream>     // for ostream, istream
#include <algorithm>    // for min, max, swap
#include <stdexcept>    // for out_of_range, length_error
#include <iterator>     // for iterator traits
#include <cstddef>      // for size_t, ptrdiff_t

// ============================================================================
//  String -- a hand-rolled std::string (dynamic char sequence + SSO)
// ============================================================================
//
// WHAT IT IS
// ----------
// A String is a growable, null-terminated sequence of char. It behaves like
// std::string: you can append, insert, search, and compare, while the class
// manages memory for you (RAII). Unlike a raw char*, length and capacity are
// tracked explicitly and a trailing '\0' is always maintained.
//
// THE FOUR FIELDS
// ---------------
//
//     data_        -> pointer to the active character buffer (SSO or heap)
//     size_        -> number of characters (NOT counting the '\0')
//     capacity_    -> chars the buffer can hold before regrowing (excl. '\0')
//     sso_buffer_  -> inline stack buffer used when size_ <= SSO_CAPACITY (15)
//
// SSO DETECTION: is_sso() returns true when data_ == sso_buffer_.
// Invariant: 0 <= size_ <= capacity_, and data_[size_] == '\0' always.
//
// MEMORY LAYOUT — SSO STATE (size_ = 5, capacity_ = 15)
// -------------------------------------------------------
//
//     String object (on stack)              Characters live INSIDE the object
//     ┌──────────────────────────┐          (no heap allocation)
//     │ data_      ●─────────────┼──┐
//     │ size_      = 5           │  │
//     │ capacity_  = 15          │  │       sso_buffer_[16]
//     │ sso_buffer_┌─────────────┼──┘       ┌───┬───┬───┬───┬───┬───┬─────┐
//     │            │ H │ e │ l │ l │ o │\0 │ ? │ ? │ ...               │
//     └────────────┴───┴───┴───┴───┴───┴───┴───┴───┴─────────────────────┘
//                  0   1   2   3   4   5   6   7        (unused SSO slack)
//                  └──── size_ = 5 live chars ────┘└─ spare capacity ─┘
//
// MEMORY LAYOUT — HEAP STATE (size_ = 30, capacity_ = 30)
// ---------------------------------------------------------
//
//     String object (on stack)              Heap block (capacity_ + 1 bytes)
//     ┌──────────────────────────┐          ┌───┬───┬── ... ──┬───┬───┐
//     │ data_      ●─────────────┼─────────▶│ L │ o │ n │ g │...│\0 │
//     │ size_      = 30          │          └───┴───┴───┴───┴───┴───┘
//     │ capacity_  = 30          │            0   1   2       29  30
//     │ sso_buffer_[16] (idle)   │          sso_buffer_ is NOT used; data_
//     └──────────────────────────┘          points past it to the heap block
//
// SSO → HEAP TRANSITION (why reserve/append matter)
// -------------------------------------------------
// When size_ would exceed SSO_CAPACITY (15), we allocate a heap buffer,
// copy the characters across, and repoint data_ away from sso_buffer_.
//
//     before (SSO):  data_ ─▶ sso_buffer_  "Hello\0"     (15 cap, no heap)
//     append 20 chars triggers ensure_capacity:
//     after (heap):  data_ ─▶ [heap] "Hello + 20 more chars...\0"
//                    sso_buffer_ sits unused in the object
//
// HEAP → SSO (shrink_to_fit when size_ drops to ≤ 15)
// ---------------------------------------------------
// shrink_to_fit() can copy back into sso_buffer_ and delete[] the heap block.
//
// Key characteristics:
// - SSO avoids heap for strings ≤ 15 characters (common case)
// - Null-terminated C-string compatibility via c_str() / data()
// - Move from heap strings steals the pointer in O(1); SSO moves copy inline
// - swap() handles all four SSO/heap combinations without leaking
// ============================================================================

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
    static constexpr size_type SSO_CAPACITY = 15;  // max chars stored inline (no heap)

    char* data_;              // active buffer: points at sso_buffer_ (SSO) or heap block
    size_type size_;          // character count (excludes trailing '\0')
    size_type capacity_;      // max chars before regrow (excludes '\0'); 15 when in SSO mode

    char sso_buffer_[SSO_CAPACITY + 1];  // inline storage: SSO_CAPACITY chars + room for '\0'

    /**
     * @brief True when characters live in sso_buffer_ (no heap allocation).
     *
     * We never store a separate "mode" flag — the pointer value IS the mode:
     *     data_ == sso_buffer_  →  SSO (stack)
     *     data_ != sso_buffer_  →  heap
     */
    bool is_sso() const {
        return data_ == sso_buffer_;
    }

    /**
     * @brief Grow capacity to at least n if we are short (delegates to reserve).
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
     * @brief Move constructor — steal heap buffer or copy SSO inline.
     *
     * Two paths depending on the source's storage mode:
     *
     *   SSO source:  must COPY (buffer lives inside `other`, can't steal it)
     *        other.sso_buffer_ ──strcpy──▶ this.sso_buffer_
     *        this.data_ = sso_buffer_
     *
     *   Heap source: STEAL pointer in O(1), reset other to empty SSO
     *        this.data_  ◀── steal ──  other.data_
     *        other.data_ = other.sso_buffer_;  other.size_ = 0
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
     * @brief Reserve at least n characters of capacity (may leave SSO for heap).
     *
     * If n <= capacity_, this is a no-op. Otherwise we allocate a new heap
     * block, strcpy the existing content across, and free the old heap block
     * (SSO buffer is never delete[]'d).
     *
     *   SSO "Hello"  reserve(100):
     *       new_data ─▶ [Hello\0___________...]  (101 bytes on heap)
     *       data_ repointed; sso_buffer_ goes idle
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
     * @brief Shrink capacity to fit size_; may move heap → SSO.
     *
     * Three outcomes:
     *   (1) already tight or SSO  → no-op
     *   (2) size_ <= 15 on heap   → strcpy into sso_buffer_, delete[] heap
     *   (3) size_ > 15 on heap     → realloc exact-size heap block
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

    /**
     * @brief Append n bytes from s — may trigger SSO → heap transition.
     *
     * Steps:
     *   (1) ensure_capacity(size_ + n)   — may allocate heap if over SSO limit
     *   (2) memcpy new bytes after data_[size_]
     *   (3) size_ += n; data_[size_] = '\0'
     *
     *   SSO "Hi" + append(" there world"):
     *       size 2 + 16 = 18 > 15  →  reserve promotes to heap first
     */
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

    /**
     * @brief Insert n bytes at pos — memmove opens a gap, then memcpy fills it.
     *
     *   insert "XX" at pos 1 of "ABCDE":
     *       [A][B][C][D][E][\0]
     *        └── memmove tail right by 2 ──▶
     *       [A][ ][ ][B][C][D][E][\0]
     *           └── memcpy "XX" ──▶
     *       [A][X][X][B][C][D][E][\0]
     *
     * memmove (not memcpy) because source and dest overlap.
     */
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

    /**
     * @brief Replace [pos, pos+len) with n new bytes — three size cases.
     *
     *   same length:  memcpy in place
     *   shorter:      memcpy replacement, memmove tail left, shrink size_
     *   longer:       ensure_capacity, memmove tail right, memcpy replacement
     */
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
     * @brief Exchange contents — four cases for SSO × heap combinations.
     *
     *   both SSO:   strcpy swap via temp buffer (both buffers inside objects)
     *   both heap:  swap data_/capacity_ pointers — O(1), no char copies
     *   mixed:      copy SSO side to temp, steal heap pointer, strcpy temp
     *
     * size_ is swapped last so both strings stay consistent.
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

