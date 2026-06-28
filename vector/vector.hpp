#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <cstddef>      // for size_t, ptrdiff_t
#include <stdexcept>    // for out_of_range, length_error
#include <algorithm>    // for copy, move, swap
#include <iterator>     // for iterator traits
#include <initializer_list>  // for initializer_list
#include <utility>      // for move, forward

// ============================================================================
//  Vector<T> -- a hand-rolled std::vector (contiguous dynamic array)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Vector is a growable array. Elements live back-to-back in a single block
// of heap memory, so indexing is a pointer addition (O(1)) and iteration is
// cache-friendly. When the block fills up, we allocate a bigger block, move
// the elements over, and free the old one.
//
// THE THREE FIELDS
// ----------------
// We track exactly three things:
//
//     data_     -> pointer to the heap block (or nullptr when empty)
//     size_     -> how many elements are actually CONSTRUCTED
//     capacity_ -> how many elements the block COULD hold before regrowing
//
// The invariant 0 <= size_ <= capacity_ always holds.
//
// MEMORY LAYOUT (size_ = 3, capacity_ = 5)
// ----------------------------------------
//
//     Vector object (on stack)            Heap block (capacity_ * sizeof(T))
//     ┌───────────────────┐               ┌─────┬─────┬─────┬─────┬─────┐
//     │ data_      ●──────┼──────────────▶│  A  │  B  │  C  │  ?  │  ?  │
//     │ size_      = 3    │               └─────┴─────┴─────┴─────┴─────┘
//     │ capacity_  = 5    │                 0     1     2     3     4
//     └───────────────────┘               └──── constructed ───┘└ raw ┘
//                                            (live objects)     (uninitialized
//                                                                storage, NOT
//                                                                yet objects)
//
// Slots [0, size_)        hold live T objects.
// Slots [size_, capacity_) are RAW BYTES -- allocated but not yet a T. We must
// never read them and must use placement-new before treating them as a T.
//
// WHY ::operator new INSTEAD OF new T[n]?
// ---------------------------------------
// `new T[n]` would default-construct every element immediately, which is wrong:
// capacity is meant to be *reserved* space, not *constructed* space. So we
// separate the two STL steps explicitly:
//     1. allocate raw storage      -> ::operator new(bytes)   (no constructors)
//     2. construct an element      -> placement new           (construct_at)
//     3. destroy an element        -> p->~T()                 (destroy_range)
//     4. free raw storage          -> ::operator delete       (no destructors)
//
// GEOMETRIC GROWTH (why push_back is amortized O(1))
// --------------------------------------------------
// When full, capacity doubles (0 -> 1 -> 2 -> 4 -> 8 -> 16 ...). Doubling means
// that across N push_backs the total copying work is N + N/2 + N/4 + ... < 2N,
// i.e. O(N) total => O(1) *amortized* per push_back, even though an individual
// push_back that triggers a reallocation is O(N).
//
//     push_back when size_ == capacity_:
//
//     old: [A][B]                (cap 2, full)
//           │  │   move
//           ▼  ▼
//     new: [A][B][ ][ ]          (cap 4)  then write C -> [A][B][C][ ]
//
// Key characteristics:
// - Contiguous storage, random access in O(1)
// - Geometric (doubling) growth => amortized O(1) push_back
// - RAII: the destructor frees memory; no manual cleanup needed
// - Full value semantics: deep copy, cheap move
// - Strong exception guarantee on most growth operations
// ============================================================================

template<typename T>
class Vector {
public:
    // Type aliases (similar to std::vector)
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

private:
    T* data_;            // points at the heap block, or nullptr if capacity_ == 0
    size_type size_;     // number of LIVE (constructed) elements: slots [0, size_)
    size_type capacity_; // number of slots the block can hold before regrowing

    /**
     * @brief Grow to hold at least n elements, but only if we are short.
     *
     * Unlike reserve(), this is a no-op when we already have room, so callers
     * can use it as a cheap "make sure there is space" guard.
     */
    void ensure_capacity(size_type n) {
        if (n > capacity_) {
            reserve(n);
        }
    }

    /**
     * @brief Run ~T() on every element in [first, last).
     *
     * This only destroys objects; it does NOT free the underlying storage.
     * After this, those slots are raw memory again (back to the "?" state in
     * the layout diagram above).
     */
    void destroy_range(T* first, T* last) {
        for (T* p = first; p != last; ++p) {
            p->~T();
        }
    }

    /**
     * @brief Turn a raw slot into a live T via placement new.
     *
     * `new (p) T(...)` constructs an object *in the memory p already points to*
     * instead of allocating. This is how we materialise a real object inside a
     * previously-raw capacity slot. The variadic args make it work for copy,
     * move, and emplace alike (perfect forwarding).
     */
    template<typename... Args>
    void construct_at(T* p, Args&&... args) {
        new (p) T(std::forward<Args>(args)...);
    }

public:
    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /**
     * @brief Default constructor - creates empty vector
     */
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    /**
     * @brief Construct with n default-constructed elements
     */
    explicit Vector(size_type n) : data_(nullptr), size_(0), capacity_(0) {
        if (n > 0) {
            data_ = static_cast<T*>(::operator new(n * sizeof(T)));
            capacity_ = n;
            
            for (size_type i = 0; i < n; ++i) {
                construct_at(data_ + i);
            }
            size_ = n;
        }
    }

    /**
     * @brief Construct with n copies of value
     */
    Vector(size_type n, const T& value) : data_(nullptr), size_(0), capacity_(0) {
        if (n > 0) {
            data_ = static_cast<T*>(::operator new(n * sizeof(T)));
            capacity_ = n;
            
            for (size_type i = 0; i < n; ++i) {
                construct_at(data_ + i, value);
            }
            size_ = n;
        }
    }

    /**
     * @brief Construct from initializer list
     */
    Vector(std::initializer_list<T> init) : data_(nullptr), size_(0), capacity_(0) {
        if (init.size() > 0) {
            data_ = static_cast<T*>(::operator new(init.size() * sizeof(T)));
            capacity_ = init.size();
            
            size_type i = 0;
            for (const auto& item : init) {
                construct_at(data_ + i, item);
                ++i;
            }
            size_ = init.size();
        }
    }

    /**
     * @brief Copy constructor
     */
    Vector(const Vector& other) : data_(nullptr), size_(0), capacity_(0) {
        if (other.size_ > 0) {
            data_ = static_cast<T*>(::operator new(other.size_ * sizeof(T)));
            capacity_ = other.size_;
            
            for (size_type i = 0; i < other.size_; ++i) {
                construct_at(data_ + i, other.data_[i]);
            }
            size_ = other.size_;
        }
    }

    /**
     * @brief Move constructor
     */
    Vector(Vector&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // ============================================================================
    // DESTRUCTOR
    // ============================================================================

    /**
     * @brief Destructor - frees allocated memory
     */
    ~Vector() {
        clear();
        if (data_) {
            ::operator delete(data_);
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Copy assignment
     */
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector temp(other);
            swap(temp);
        }
        return *this;
    }

    /**
     * @brief Move assignment
     */
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            // Clean up current data
            clear();
            if (data_) {
                ::operator delete(data_);
            }
            
            // Steal other's data
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            // Reset other
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    /**
     * @brief Assignment from initializer list
     */
    Vector& operator=(std::initializer_list<T> init) {
        Vector temp(init);
        swap(temp);
        return *this;
    }

    // ============================================================================
    // ELEMENT ACCESS
    // ============================================================================

    /**
     * @brief Access element at position (with bounds checking)
     */
    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("Vector::at: position out of range");
        }
        return data_[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Vector::at: position out of range");
        }
        return data_[pos];
    }

    /**
     * @brief Access element at position (no bounds checking)
     */
    reference operator[](size_type pos) {
        return data_[pos];
    }

    const_reference operator[](size_type pos) const {
        return data_[pos];
    }

    /**
     * @brief Access first element
     */
    reference front() {
        return data_[0];
    }

    const_reference front() const {
        return data_[0];
    }

    /**
     * @brief Access last element
     */
    reference back() {
        return data_[size_ - 1];
    }

    const_reference back() const {
        return data_[size_ - 1];
    }

    /**
     * @brief Get pointer to underlying array
     */
    T* data() noexcept {
        return data_;
    }

    const T* data() const noexcept {
        return data_;
    }

    // ============================================================================
    // ITERATORS
    // ============================================================================

    iterator begin() noexcept { return data_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator cbegin() const noexcept { return data_; }

    iterator end() noexcept { return data_ + size_; }
    const_iterator end() const noexcept { return data_ + size_; }
    const_iterator cend() const noexcept { return data_ + size_; }

    // ============================================================================
    // CAPACITY
    // ============================================================================

    /**
     * @brief Check if vector is empty
     */
    bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief Get number of elements
     */
    size_type size() const noexcept {
        return size_;
    }

    /**
     * @brief Get maximum possible size
     */
    size_type max_size() const noexcept {
        return static_cast<size_type>(-1) / sizeof(T);
    }

    /**
     * @brief Make sure the block can hold at least n elements (reallocation).
     *
     * This is the single place where the storage block actually moves. Every
     * pointer/iterator/reference into the old block is INVALIDATED afterwards,
     * because the elements physically relocate to a new address.
     *
     * Steps (each numbered below):
     *
     *   before:  data_ ─▶ [A][B][C]            (cap 3, full)
     *
     *   (1) allocate a fresh, larger raw block:
     *            new_data ─▶ [ ][ ][ ][ ][ ]   (cap n, all raw)
     *
     *   (2) move each old element into the new block (placement new). We MOVE
     *       rather than copy so that, e.g., a Vector<string> just steals each
     *       string's buffer instead of duplicating characters:
     *            new_data ─▶ [A][B][C][ ][ ]
     *
     *   (3) destroy the (now moved-from) old elements, then
     *   (4) free the old raw block, and finally
     *       repoint data_ at the new block.
     */
    void reserve(size_type n) {
        if (n <= capacity_) {
            return;  // already big enough -- never shrink in reserve()
        }

        // (1) Raw storage only -- no T constructors run here.
        T* new_data = static_cast<T*>(::operator new(n * sizeof(T)));

        // (2) Relocate existing elements by move-construction.
        for (size_type i = 0; i < size_; ++i) {
            construct_at(new_data + i, std::move(data_[i]));
        }

        // (3) Old objects are husks now -- destroy them.
        destroy_range(data_, data_ + size_);

        // (4) Release the old block (no-op if we were empty).
        if (data_) {
            ::operator delete(data_);
        }

        data_ = new_data;
        capacity_ = n;
    }

    /**
     * @brief Get current capacity
     */
    size_type capacity() const noexcept {
        return capacity_;
    }

    /**
     * @brief Reduce capacity to fit size
     */
    void shrink_to_fit() {
        if (capacity_ == size_) {
            return;
        }
        
        if (size_ == 0) {
            if (data_) {
                ::operator delete(data_);
            }
            data_ = nullptr;
            capacity_ = 0;
            return;
        }
        
        // Allocate exact-size buffer
        T* new_data = static_cast<T*>(::operator new(size_ * sizeof(T)));
        
        // Move elements
        for (size_type i = 0; i < size_; ++i) {
            construct_at(new_data + i, std::move(data_[i]));
        }
        
        // Clean up old buffer
        destroy_range(data_, data_ + size_);
        ::operator delete(data_);
        
        data_ = new_data;
        capacity_ = size_;
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Clear all elements
     */
    void clear() noexcept {
        destroy_range(data_, data_ + size_);
        size_ = 0;
    }

    /**
     * @brief Insert a copy of value before pos; returns iterator to it.
     *
     * Inserting in the middle is O(n): everything from pos onward must slide
     * one slot to the right to open a gap.
     *
     *   insert X at index 1 of [A][B][C]:
     *
     *       [A][B][C][ ]        (the trailing [ ] is the new capacity slot)
     *        │  └─┬─┘ ▲
     *        │    └───┘  shift B,C right by one
     *       [A][ ][B][C]
     *           ▲
     *       [A][X][B][C]        drop X into the gap
     *
     * SUBTLETY: the rightmost destination slot (old data_[size_]) is RAW memory,
     * so it must be filled with construct_at (placement new). Every other
     * destination already holds a live object, so we move-ASSIGN into it. We
     * capture `index` before reserve() because reserve() may move the block and
     * invalidate the incoming `pos` pointer.
     */
    iterator insert(const_iterator pos, const T& value) {
        size_type index = pos - data_;  // remember position as an index, not a pointer

        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);  // may relocate; that's why we saved `index` above
        }

        // Shift the tail right by one, walking from the back to avoid overwrites.
        for (size_type i = size_; i > index; --i) {
            if (i == size_) {
                construct_at(data_ + i, std::move(data_[i - 1]));  // into RAW slot
            } else {
                data_[i] = std::move(data_[i - 1]);                // into LIVE slot
            }
        }

        // Place the new value: the gap is live unless we inserted at the very end.
        if (index < size_) {
            data_[index] = value;            // overwrite a live (moved-from) slot
        } else {
            construct_at(data_ + index, value);  // index == size_: still raw
        }

        ++size_;
        return data_ + index;
    }

    /**
     * @brief Insert element at position (move version)
     */
    iterator insert(const_iterator pos, T&& value) {
        size_type index = pos - data_;
        
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        
        // Shift elements right
        for (size_type i = size_; i > index; --i) {
            if (i == size_) {
                construct_at(data_ + i, std::move(data_[i - 1]));
            } else {
                data_[i] = std::move(data_[i - 1]);
            }
        }
        
        // Insert new element
        if (index < size_) {
            data_[index] = std::move(value);
        } else {
            construct_at(data_ + index, std::move(value));
        }
        
        ++size_;
        return data_ + index;
    }

    /**
     * @brief Erase element at position
     */
    iterator erase(const_iterator pos) {
        size_type index = pos - data_;
        
        // Shift elements left
        for (size_type i = index; i < size_ - 1; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        
        // Destroy last element
        data_[size_ - 1].~T();
        --size_;
        
        return data_ + index;
    }

    /**
     * @brief Erase range [first, last)
     */
    iterator erase(const_iterator first, const_iterator last) {
        size_type first_index = first - data_;
        size_type last_index = last - data_;
        size_type count = last_index - first_index;
        
        if (count == 0) {
            return data_ + first_index;
        }
        
        // Shift elements left
        for (size_type i = first_index; i < size_ - count; ++i) {
            data_[i] = std::move(data_[i + count]);
        }
        
        // Destroy elements at end
        destroy_range(data_ + size_ - count, data_ + size_);
        size_ -= count;
        
        return data_ + first_index;
    }

    /**
     * @brief Append a copy of value at the end. Amortized O(1).
     *
     * If there is spare capacity we just construct into the next raw slot
     * (data_ + size_) -- pure O(1). Only when the block is exactly full do we
     * pay for a reallocation (double the capacity), and geometric doubling makes
     * that cost average out to O(1) per element across many push_backs.
     */
    void push_back(const T& value) {
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;  // 0->1->2->4->8...
            reserve(new_cap);
        }

        construct_at(data_ + size_, value);  // build into the first free slot
        ++size_;
    }

    /**
     * @brief Add element at end (move version)
     */
    void push_back(T&& value) {
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        
        construct_at(data_ + size_, std::move(value));
        ++size_;
    }

    /**
     * @brief Construct element in-place at end
     */
    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        
        construct_at(data_ + size_, std::forward<Args>(args)...);
        ++size_;
    }

    /**
     * @brief Remove last element
     */
    void pop_back() {
        if (size_ > 0) {
            --size_;
            data_[size_].~T();
        }
    }

    /**
     * @brief Resize vector
     */
    void resize(size_type n) {
        if (n < size_) {
            destroy_range(data_ + n, data_ + size_);
            size_ = n;
        } else if (n > size_) {
            ensure_capacity(n);
            for (size_type i = size_; i < n; ++i) {
                construct_at(data_ + i);
            }
            size_ = n;
        }
    }

    /**
     * @brief Resize vector with fill value
     */
    void resize(size_type n, const T& value) {
        if (n < size_) {
            destroy_range(data_ + n, data_ + size_);
            size_ = n;
        } else if (n > size_) {
            ensure_capacity(n);
            for (size_type i = size_; i < n; ++i) {
                construct_at(data_ + i, value);
            }
            size_ = n;
        }
    }

    /**
     * @brief Swap with another vector
     */
    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
};

// ============================================================================
// NON-MEMBER FUNCTIONS
// ============================================================================

/**
 * @brief Swap two vectors
 */
template<typename T>
void swap(Vector<T>& lhs, Vector<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * @brief Equality comparison
 */
template<typename T>
bool operator==(const Vector<T>& lhs, const Vector<T>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    
    for (typename Vector<T>::size_type i = 0; i < lhs.size(); ++i) {
        if (!(lhs[i] == rhs[i])) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Inequality comparison
 */
template<typename T>
bool operator!=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Less-than comparison
 */
template<typename T>
bool operator<(const Vector<T>& lhs, const Vector<T>& rhs) {
    typename Vector<T>::size_type min_size = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
    
    for (typename Vector<T>::size_type i = 0; i < min_size; ++i) {
        if (lhs[i] < rhs[i]) return true;
        if (rhs[i] < lhs[i]) return false;
    }
    
    return lhs.size() < rhs.size();
}

/**
 * @brief Greater-than comparison
 */
template<typename T>
bool operator>(const Vector<T>& lhs, const Vector<T>& rhs) {
    return rhs < lhs;
}

/**
 * @brief Less-than-or-equal comparison
 */
template<typename T>
bool operator<=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(rhs < lhs);
}

/**
 * @brief Greater-than-or-equal comparison
 */
template<typename T>
bool operator>=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs < rhs);
}

#endif // VECTOR_HPP

