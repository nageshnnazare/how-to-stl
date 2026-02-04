#ifndef DEQUE_HPP
#define DEQUE_HPP

#include <cstddef>      // for size_t, ptrdiff_t
#include <stdexcept>    // for out_of_range
#include <iterator>     // for iterator traits
#include <initializer_list>  // for initializer_list
#include <utility>      // for move, forward
#include <algorithm>    // for copy, move

/**
 * @brief Custom implementation of std::deque
 * 
 * Deque is a double-ended queue that supports efficient insertion and deletion
 * at both ends, plus random access.
 * 
 * Key characteristics:
 * - Chunk-based storage (array of arrays)
 * - O(1) insertion/deletion at both ends
 * - O(1) random access
 * - No reallocation of existing elements
 * - More complex than vector, simpler than list
 * 
 * Implementation uses map of chunks where each chunk holds CHUNK_SIZE elements.
 */

template<typename T>
class Deque {
public:
    // Type aliases
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    // Forward declaration of iterator
    class iterator;
    class const_iterator;

private:
    static constexpr size_type CHUNK_SIZE = 16;  // Elements per chunk
    
    T** map_;           // Array of pointers to chunks
    size_type map_size_;     // Number of chunk pointers
    size_type first_chunk_;  // Index of first used chunk
    size_type last_chunk_;   // Index of last used chunk
    size_type first_index_;  // Index within first chunk
    size_type last_index_;   // Index within last chunk (one past last element)
    size_type size_;         // Total number of elements

    /**
     * @brief Allocate a new chunk
     */
    T* allocate_chunk() {
        return static_cast<T*>(::operator new(CHUNK_SIZE * sizeof(T)));
    }

    /**
     * @brief Deallocate a chunk
     */
    void deallocate_chunk(T* chunk) {
        ::operator delete(chunk);
    }

    /**
     * @brief Initialize empty map
     */
    void init_map(size_type num_elements = 0) {
        size_type num_chunks = num_elements / CHUNK_SIZE + 1;
        map_size_ = num_chunks + 2;  // Extra space on both sides
        
        map_ = static_cast<T**>(::operator new(map_size_ * sizeof(T*)));
        for (size_type i = 0; i < map_size_; ++i) {
            map_[i] = nullptr;
        }
        
        // Center the chunks
        first_chunk_ = map_size_ / 2;
        last_chunk_ = first_chunk_;
        first_index_ = CHUNK_SIZE / 2;
        last_index_ = first_index_;
        size_ = 0;
        
        // Allocate at least one chunk
        map_[first_chunk_] = allocate_chunk();
    }

    /**
     * @brief Ensure space at front
     */
    void reserve_front() {
        if (first_chunk_ == 0 || (first_chunk_ == 1 && first_index_ == 0)) {
            reallocate_map(true);
        }
    }

    /**
     * @brief Ensure space at back
     */
    void reserve_back() {
        if (last_chunk_ >= map_size_ - 1 || 
            (last_chunk_ == map_size_ - 2 && last_index_ == CHUNK_SIZE)) {
            reallocate_map(false);
        }
    }

    /**
     * @brief Reallocate map with more space
     */
    void reallocate_map(bool /* add_at_front */) {
        size_type new_map_size = map_size_ * 2;
        
        T** new_map = static_cast<T**>(::operator new(new_map_size * sizeof(T*)));
        for (size_type i = 0; i < new_map_size; ++i) {
            new_map[i] = nullptr;
        }
        
        // Copy chunk pointers to center of new map
        size_type new_first_chunk = (new_map_size - (last_chunk_ - first_chunk_ + 1)) / 2;
        
        for (size_type i = first_chunk_; i <= last_chunk_; ++i) {
            new_map[new_first_chunk + (i - first_chunk_)] = map_[i];
        }
        
        ::operator delete(map_);
        
        map_ = new_map;
        map_size_ = new_map_size;
        last_chunk_ = new_first_chunk + (last_chunk_ - first_chunk_);
        first_chunk_ = new_first_chunk;
    }

    /**
     * @brief Construct element at position
     */
    template<typename... Args>
    void construct_at(T* p, Args&&... args) {
        new (p) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Destroy element at position
     */
    void destroy_at(T* p) {
        p->~T();
    }

public:
    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /**
     * @brief Default constructor - creates empty deque
     */
    Deque() : map_(nullptr), map_size_(0), first_chunk_(0), last_chunk_(0),
              first_index_(0), last_index_(0), size_(0) {
        init_map(0);
    }

    /**
     * @brief Construct with n default-constructed elements
     */
    explicit Deque(size_type n) : map_(nullptr), map_size_(0), first_chunk_(0),
                                   last_chunk_(0), first_index_(0), last_index_(0), size_(0) {
        init_map(n);
        for (size_type i = 0; i < n; ++i) {
            push_back(T());
        }
    }

    /**
     * @brief Construct with n copies of value
     */
    Deque(size_type n, const T& value) : map_(nullptr), map_size_(0), first_chunk_(0),
                                          last_chunk_(0), first_index_(0), last_index_(0), size_(0) {
        init_map(n);
        for (size_type i = 0; i < n; ++i) {
            push_back(value);
        }
    }

    /**
     * @brief Construct from initializer list
     */
    Deque(std::initializer_list<T> init) : map_(nullptr), map_size_(0), first_chunk_(0),
                                            last_chunk_(0), first_index_(0), last_index_(0), size_(0) {
        init_map(init.size());
        for (const auto& item : init) {
            push_back(item);
        }
    }

    /**
     * @brief Copy constructor
     */
    Deque(const Deque& other) : map_(nullptr), map_size_(0), first_chunk_(0),
                                 last_chunk_(0), first_index_(0), last_index_(0), size_(0) {
        init_map(other.size_);
        for (const auto& item : other) {
            push_back(item);
        }
    }

    /**
     * @brief Move constructor
     */
    Deque(Deque&& other) noexcept
        : map_(other.map_), map_size_(other.map_size_),
          first_chunk_(other.first_chunk_), last_chunk_(other.last_chunk_),
          first_index_(other.first_index_), last_index_(other.last_index_),
          size_(other.size_) {
        other.map_ = nullptr;
        other.map_size_ = 0;
        other.first_chunk_ = 0;
        other.last_chunk_ = 0;
        other.first_index_ = 0;
        other.last_index_ = 0;
        other.size_ = 0;
    }

    // ============================================================================
    // DESTRUCTOR
    // ============================================================================

    /**
     * @brief Destructor - frees all memory
     */
    ~Deque() {
        clear();
        if (map_) {
            for (size_type i = 0; i < map_size_; ++i) {
                if (map_[i]) {
                    deallocate_chunk(map_[i]);
                }
            }
            ::operator delete(map_);
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Copy assignment
     */
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }

    /**
     * @brief Move assignment
     */
    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            // Clean up current data
            clear();
            if (map_) {
                for (size_type i = 0; i < map_size_; ++i) {
                    if (map_[i]) {
                        deallocate_chunk(map_[i]);
                    }
                }
                ::operator delete(map_);
            }
            
            // Steal other's data
            map_ = other.map_;
            map_size_ = other.map_size_;
            first_chunk_ = other.first_chunk_;
            last_chunk_ = other.last_chunk_;
            first_index_ = other.first_index_;
            last_index_ = other.last_index_;
            size_ = other.size_;
            
            // Reset other
            other.map_ = nullptr;
            other.map_size_ = 0;
            other.first_chunk_ = 0;
            other.last_chunk_ = 0;
            other.first_index_ = 0;
            other.last_index_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    /**
     * @brief Assignment from initializer list
     */
    Deque& operator=(std::initializer_list<T> init) {
        clear();
        for (const auto& item : init) {
            push_back(item);
        }
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
            throw std::out_of_range("Deque::at: position out of range");
        }
        return (*this)[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Deque::at: position out of range");
        }
        return (*this)[pos];
    }

    /**
     * @brief Access element at position (no bounds checking)
     */
    reference operator[](size_type pos) {
        size_type absolute_index = first_index_ + pos;
        size_type chunk = first_chunk_ + absolute_index / CHUNK_SIZE;
        size_type index = absolute_index % CHUNK_SIZE;
        return map_[chunk][index];
    }

    const_reference operator[](size_type pos) const {
        size_type absolute_index = first_index_ + pos;
        size_type chunk = first_chunk_ + absolute_index / CHUNK_SIZE;
        size_type index = absolute_index % CHUNK_SIZE;
        return map_[chunk][index];
    }

    /**
     * @brief Access first element
     */
    reference front() {
        return map_[first_chunk_][first_index_];
    }

    const_reference front() const {
        return map_[first_chunk_][first_index_];
    }

    /**
     * @brief Access last element
     */
    reference back() {
        if (last_index_ == 0) {
            return map_[last_chunk_ - 1][CHUNK_SIZE - 1];
        }
        return map_[last_chunk_][last_index_ - 1];
    }

    const_reference back() const {
        if (last_index_ == 0) {
            return map_[last_chunk_ - 1][CHUNK_SIZE - 1];
        }
        return map_[last_chunk_][last_index_ - 1];
    }

    // ============================================================================
    // ITERATORS
    // ============================================================================

    class iterator {
    private:
        Deque* deque_;
        size_type pos_;

        friend class Deque;
        friend class const_iterator;

        iterator(Deque* d, size_type p) : deque_(d), pos_(p) {}

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() : deque_(nullptr), pos_(0) {}

        reference operator*() const { return (*deque_)[pos_]; }
        pointer operator->() const { return &(*deque_)[pos_]; }

        iterator& operator++() { ++pos_; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++pos_; return tmp; }
        iterator& operator--() { --pos_; return *this; }
        iterator operator--(int) { iterator tmp = *this; --pos_; return tmp; }

        iterator& operator+=(difference_type n) { pos_ += n; return *this; }
        iterator& operator-=(difference_type n) { pos_ -= n; return *this; }

        iterator operator+(difference_type n) const { return iterator(deque_, pos_ + n); }
        iterator operator-(difference_type n) const { return iterator(deque_, pos_ - n); }

        difference_type operator-(const iterator& other) const { return pos_ - other.pos_; }

        reference operator[](difference_type n) const { return (*deque_)[pos_ + n]; }

        bool operator==(const iterator& other) const { return pos_ == other.pos_; }
        bool operator!=(const iterator& other) const { return pos_ != other.pos_; }
        bool operator<(const iterator& other) const { return pos_ < other.pos_; }
        bool operator>(const iterator& other) const { return pos_ > other.pos_; }
        bool operator<=(const iterator& other) const { return pos_ <= other.pos_; }
        bool operator>=(const iterator& other) const { return pos_ >= other.pos_; }
    };

    class const_iterator {
    private:
        const Deque* deque_;
        size_type pos_;

        friend class Deque;

        const_iterator(const Deque* d, size_type p) : deque_(d), pos_(p) {}

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() : deque_(nullptr), pos_(0) {}
        const_iterator(const iterator& it) : deque_(it.deque_), pos_(it.pos_) {}

        reference operator*() const { return (*deque_)[pos_]; }
        pointer operator->() const { return &(*deque_)[pos_]; }

        const_iterator& operator++() { ++pos_; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ++pos_; return tmp; }
        const_iterator& operator--() { --pos_; return *this; }
        const_iterator operator--(int) { const_iterator tmp = *this; --pos_; return tmp; }

        const_iterator& operator+=(difference_type n) { pos_ += n; return *this; }
        const_iterator& operator-=(difference_type n) { pos_ -= n; return *this; }

        const_iterator operator+(difference_type n) const { return const_iterator(deque_, pos_ + n); }
        const_iterator operator-(difference_type n) const { return const_iterator(deque_, pos_ - n); }

        difference_type operator-(const const_iterator& other) const { return pos_ - other.pos_; }

        reference operator[](difference_type n) const { return (*deque_)[pos_ + n]; }

        bool operator==(const const_iterator& other) const { return pos_ == other.pos_; }
        bool operator!=(const const_iterator& other) const { return pos_ != other.pos_; }
        bool operator<(const const_iterator& other) const { return pos_ < other.pos_; }
        bool operator>(const const_iterator& other) const { return pos_ > other.pos_; }
        bool operator<=(const const_iterator& other) const { return pos_ <= other.pos_; }
        bool operator>=(const const_iterator& other) const { return pos_ >= other.pos_; }
    };

    iterator begin() noexcept { return iterator(this, 0); }
    const_iterator begin() const noexcept { return const_iterator(this, 0); }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

    iterator end() noexcept { return iterator(this, size_); }
    const_iterator end() const noexcept { return const_iterator(this, size_); }
    const_iterator cend() const noexcept { return const_iterator(this, size_); }

    // ============================================================================
    // CAPACITY
    // ============================================================================

    /**
     * @brief Check if deque is empty
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

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Clear all elements
     */
    void clear() noexcept {
        while (!empty()) {
            pop_back();
        }
    }

    /**
     * @brief Add element at front
     */
    void push_front(const T& value) {
        if (first_index_ == 0) {
            if (first_chunk_ == 0) {
                reserve_front();
            }
            --first_chunk_;
            if (!map_[first_chunk_]) {
                map_[first_chunk_] = allocate_chunk();
            }
            first_index_ = CHUNK_SIZE;
        }
        
        --first_index_;
        construct_at(&map_[first_chunk_][first_index_], value);
        ++size_;
    }

    /**
     * @brief Add element at front (move version)
     */
    void push_front(T&& value) {
        if (first_index_ == 0) {
            if (first_chunk_ == 0) {
                reserve_front();
            }
            --first_chunk_;
            if (!map_[first_chunk_]) {
                map_[first_chunk_] = allocate_chunk();
            }
            first_index_ = CHUNK_SIZE;
        }
        
        --first_index_;
        construct_at(&map_[first_chunk_][first_index_], std::move(value));
        ++size_;
    }

    /**
     * @brief Add element at back
     */
    void push_back(const T& value) {
        if (last_index_ == CHUNK_SIZE) {
            ++last_chunk_;
            if (last_chunk_ >= map_size_) {
                reserve_back();
            }
            if (!map_[last_chunk_]) {
                map_[last_chunk_] = allocate_chunk();
            }
            last_index_ = 0;
        }
        
        construct_at(&map_[last_chunk_][last_index_], value);
        ++last_index_;
        ++size_;
    }

    /**
     * @brief Add element at back (move version)
     */
    void push_back(T&& value) {
        if (last_index_ == CHUNK_SIZE) {
            ++last_chunk_;
            if (last_chunk_ >= map_size_) {
                reserve_back();
            }
            if (!map_[last_chunk_]) {
                map_[last_chunk_] = allocate_chunk();
            }
            last_index_ = 0;
        }
        
        construct_at(&map_[last_chunk_][last_index_], std::move(value));
        ++last_index_;
        ++size_;
    }

    /**
     * @brief Construct element in-place at front
     */
    template<typename... Args>
    void emplace_front(Args&&... args) {
        if (first_index_ == 0) {
            if (first_chunk_ == 0) {
                reserve_front();
            }
            --first_chunk_;
            if (!map_[first_chunk_]) {
                map_[first_chunk_] = allocate_chunk();
            }
            first_index_ = CHUNK_SIZE;
        }
        
        --first_index_;
        construct_at(&map_[first_chunk_][first_index_], std::forward<Args>(args)...);
        ++size_;
    }

    /**
     * @brief Construct element in-place at back
     */
    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (last_index_ == CHUNK_SIZE) {
            ++last_chunk_;
            if (last_chunk_ >= map_size_) {
                reserve_back();
            }
            if (!map_[last_chunk_]) {
                map_[last_chunk_] = allocate_chunk();
            }
            last_index_ = 0;
        }
        
        construct_at(&map_[last_chunk_][last_index_], std::forward<Args>(args)...);
        ++last_index_;
        ++size_;
    }

    /**
     * @brief Remove first element
     */
    void pop_front() {
        if (size_ > 0) {
            destroy_at(&map_[first_chunk_][first_index_]);
            ++first_index_;
            --size_;
            
            if (first_index_ == CHUNK_SIZE) {
                ++first_chunk_;
                first_index_ = 0;
            }
        }
    }

    /**
     * @brief Remove last element
     */
    void pop_back() {
        if (size_ > 0) {
            if (last_index_ == 0) {
                --last_chunk_;
                last_index_ = CHUNK_SIZE;
            }
            --last_index_;
            
            destroy_at(&map_[last_chunk_][last_index_]);
            --size_;
        }
    }

    /**
     * @brief Resize deque
     */
    void resize(size_type n) {
        while (size_ < n) {
            push_back(T());
        }
        while (size_ > n) {
            pop_back();
        }
    }

    /**
     * @brief Resize deque with fill value
     */
    void resize(size_type n, const T& value) {
        while (size_ < n) {
            push_back(value);
        }
        while (size_ > n) {
            pop_back();
        }
    }

    /**
     * @brief Swap with another deque
     */
    void swap(Deque& other) noexcept {
        std::swap(map_, other.map_);
        std::swap(map_size_, other.map_size_);
        std::swap(first_chunk_, other.first_chunk_);
        std::swap(last_chunk_, other.last_chunk_);
        std::swap(first_index_, other.first_index_);
        std::swap(last_index_, other.last_index_);
        std::swap(size_, other.size_);
    }
};

// ============================================================================
// NON-MEMBER FUNCTIONS
// ============================================================================

/**
 * @brief Swap two deques
 */
template<typename T>
void swap(Deque<T>& lhs, Deque<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * @brief Equality comparison
 */
template<typename T>
bool operator==(const Deque<T>& lhs, const Deque<T>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    
    for (typename Deque<T>::size_type i = 0; i < lhs.size(); ++i) {
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
bool operator!=(const Deque<T>& lhs, const Deque<T>& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Less-than comparison
 */
template<typename T>
bool operator<(const Deque<T>& lhs, const Deque<T>& rhs) {
    typename Deque<T>::size_type min_size = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
    
    for (typename Deque<T>::size_type i = 0; i < min_size; ++i) {
        if (lhs[i] < rhs[i]) return true;
        if (rhs[i] < lhs[i]) return false;
    }
    
    return lhs.size() < rhs.size();
}

/**
 * @brief Greater-than comparison
 */
template<typename T>
bool operator>(const Deque<T>& lhs, const Deque<T>& rhs) {
    return rhs < lhs;
}

/**
 * @brief Less-than-or-equal comparison
 */
template<typename T>
bool operator<=(const Deque<T>& lhs, const Deque<T>& rhs) {
    return !(rhs < lhs);
}

/**
 * @brief Greater-than-or-equal comparison
 */
template<typename T>
bool operator>=(const Deque<T>& lhs, const Deque<T>& rhs) {
    return !(lhs < rhs);
}

#endif // DEQUE_HPP

