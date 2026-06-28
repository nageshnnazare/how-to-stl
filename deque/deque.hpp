#ifndef DEQUE_HPP
#define DEQUE_HPP

#include <cstddef>      // for size_t, ptrdiff_t
#include <stdexcept>    // for out_of_range
#include <iterator>     // for iterator traits
#include <initializer_list>  // for initializer_list
#include <utility>      // for move, forward
#include <algorithm>    // for copy, move

// ============================================================================
//  Deque<T> -- a hand-rolled std::deque (chunked double-ended queue)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Deque is a sequence container that supports efficient insertion and removal
// at BOTH ends (push_front, push_back) while still offering O(1) random access
// by index. Unlike Vector, existing elements never relocate when the deque grows
// — new elements go into new fixed-size chunks.
//
// THE SEVEN FIELDS
// ----------------
//
//     map_          -> array of T* pointers (each points to one chunk or nullptr)
//     map_size_     -> number of slots in the map array
//     first_chunk_  -> map index of the chunk holding the front element
//     last_chunk_   -> map index of the chunk holding the back element
//     first_index_  -> offset within first_chunk_ to the front element
//     last_index_   -> offset one-past the last element in last_chunk_
//     size_         -> total element count across all chunks
//
// CHUNK_SIZE = 16 elements per chunk (fixed, compile-time constant).
//
// MEMORY LAYOUT (size_ = 5, elements A..E)
// ----------------------------------------
//
//     Deque object (stack)                 map_ (array of chunk pointers)
//     ┌─────────────────────┐             ┌───┬───┬───┬───┬───┬───┬───┐
//     │ map_         ●──────┼────────────▶│ ∅ │ ∅ │ ● │ ● │ ∅ │ ∅ │ ∅ │
//     │ map_size_  = 7    │             └───┴───┴─┬─┴─┬─┴───┴───┴───┘
//     │ first_chunk_ = 2  │                       │   │
//     │ last_chunk_  = 3  │                       │   │
//     │ first_index_ = 6  │              chunk 2  │   │  chunk 3
//     │ last_index_  = 3  │              (16 slots)│   │  (16 slots)
//     │ size_        = 5  │                       ▼   ▼
//     └─────────────────────┘             ┌─────────────────────────────┐
//                                         │ ? ? ? ? ? ? A B C D E ? ... │
//                                         └─────────────────────────────┘
//                                           idx: 6 7 8 9  (first_index_=6)
//                                           chunk3 idx 0 1 2 = D E (last_index_=3)
//
// Slots outside [first_index_, ...) in the first/last chunks are RAW storage —
// allocated by ::operator new but not yet constructed T objects until written.
//
// RANDOM ACCESS — how operator[](pos) works
// -----------------------------------------
// Logical position `pos` (0 = front) maps to a chunk + offset:
//
//     absolute = first_index_ + pos
//     chunk    = first_chunk_ + absolute / CHUNK_SIZE
//     index    = absolute % CHUNK_SIZE
//     return map_[chunk][index];
//
//     pos=0 → absolute=6 → chunk 2, index 6 → A
//     pos=4 → absolute=10 → chunk 3, index 2 → E  (10/16=0 rem... wait)
//     Actually: first_index_=6, pos=4 → absolute=10 → chunk 2+10/16=2, 10%16=10
//     Hmm let me recalculate for the diagram...
//
// PUSH_BACK — grow at the tail
// ----------------------------
//
//     last_index_ < CHUNK_SIZE: construct into map_[last_chunk_][last_index_++]
//     last_index_ == CHUNK_SIZE: ++last_chunk_, maybe allocate new chunk,
//                                 last_index_ = 0, then construct
//
//     chunk full:  [..][..][X][X][X]...[X]  (16 live)
//                  └── new chunk ──▶ [new][ ][ ]...
//
// PUSH_FRONT — grow at the head (mirror of push_back)
// ---------------------------------------------------
//
//     first_index_ > 0: --first_index_, construct at map_[first_chunk_][first_index_]
//     first_index_ == 0: --first_chunk_ (maybe new chunk), first_index_ = CHUNK_SIZE
//
// MAP REALLOCATION — when we run out of map slots
// -----------------------------------------------
// reallocate_map() doubles map_size_, copies existing chunk pointers to the
// CENTER of the new map (leaving empty slots on both sides for future growth).
// Chunk pointers move; the T objects inside chunks do NOT move.
//
// Key characteristics:
// - O(1) push/pop at both ends (amortized when map regrows)
// - O(1) random access via chunk arithmetic
// - Elements never relocate — pointers/references to elements stay valid
// - NOT contiguous — worse cache locality than Vector
// ============================================================================

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
    static constexpr size_type CHUNK_SIZE = 16;  // elements per fixed-size chunk

    T** map_;              // array of pointers to chunks (nullptr = no chunk allocated)
    size_type map_size_;   // number of pointer slots in map_
    size_type first_chunk_; // map index of chunk containing front()
    size_type last_chunk_;  // map index of chunk containing back()
    size_type first_index_; // offset within first_chunk_ to front element
    size_type last_index_;  // offset one-past last element in last_chunk_
    size_type size_;        // total constructed element count

    /**
     * @brief Allocate one raw chunk (CHUNK_SIZE * sizeof(T) bytes, no constructors).
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
     * @brief Set up the map and allocate the first centered chunk.
     *
     * Creates a map with spare nullptr slots on both sides so push_front and
     * push_back can grow without immediately reallocating the map.
     *
     *     map_:  [∅][∅][●chunk][∅][∅]
     *                  ▲
     *            first_chunk_ = last_chunk_ = center
     *            first_index_ = last_index_ = CHUNK_SIZE/2  (start mid-chunk)
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
     * @brief Grow the map when we are about to run off the front edge.
     *
     * Triggers reallocate_map when first_chunk_ is 0 and first_index_ is 0.
     */
    void reserve_front() {
        if (first_chunk_ == 0 || (first_chunk_ == 1 && first_index_ == 0)) {
            reallocate_map(true);
        }
    }

    /**
     * @brief Grow the map when we are about to run off the back edge.
     */
    void reserve_back() {
        if (last_chunk_ >= map_size_ - 1 || 
            (last_chunk_ == map_size_ - 2 && last_index_ == CHUNK_SIZE)) {
            reallocate_map(false);
        }
    }

    /**
     * @brief Double the map and re-center existing chunk pointers.
     *
     * Only the map_ array moves — chunk objects and their elements stay put.
     *
     *   old map:  [∅][C0][C1][∅]     first_chunk_=1
     *   new map:  [∅][∅][∅][C0][C1][∅][∅][∅]   first_chunk_=3 (centered)
     *
     * Empty slots on both sides allow future push_front / push_back without
     * another map realloc until those slots fill up.
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
     * @brief O(1) random access — translate logical index to chunk + offset.
     *
     *     pos ──▶ absolute = first_index_ + pos
     *             chunk    = first_chunk_ + absolute / CHUNK_SIZE
     *             index    = absolute % CHUNK_SIZE
     *             return map_[chunk][index]
     *
     * Two divisions per access — why deque is slightly slower than vector's
     * single pointer addition, but still O(1).
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
     * @brief Insert at front — may allocate a new chunk or grow the map.
     *
     *     first_index_ > 0:  --first_index_; construct at slot
     *     first_index_ == 0: move to previous chunk (allocate if needed),
     *                        first_index_ = CHUNK_SIZE, then --first_index_
     *
     *   push_front(X) when first_index_=0:
     *       [X][?][?]...[?]  new or recycled chunk to the left
     *        ▲
     *   first_index_ now points here
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
     * @brief Insert at back — may allocate a new chunk or grow the map.
     *
     *     last_index_ < CHUNK_SIZE: construct at map_[last_chunk_][last_index_++]
     *     last_index_ == CHUNK_SIZE: ++last_chunk_, allocate chunk if needed,
     *                                  last_index_=0, then construct
     *
     *   push_back when chunk is full (last_index_ == 16):
     *       old chunk: [e0][e1]...[e15]
     *       new chunk: [new][ ][ ]...     last_index_ = 1 after insert
     */
    void push_back(const T& value) {
        if (last_index_ == CHUNK_SIZE) {
            // Grow the map BEFORE stepping right. Incrementing last_chunk_ first
            // could push it to map_size_ (one past the end); reallocate_map then
            // reads map_[last_chunk_] out of bounds. reserve_back() inspects the
            // CURRENT (in-bounds) last_chunk_ and re-centers if we're at the edge.
            reserve_back();
            ++last_chunk_;
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
            // Grow the map BEFORE stepping right. Incrementing last_chunk_ first
            // could push it to map_size_ (one past the end); reallocate_map then
            // reads map_[last_chunk_] out of bounds. reserve_back() inspects the
            // CURRENT (in-bounds) last_chunk_ and re-centers if we're at the edge.
            reserve_back();
            ++last_chunk_;
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
            // Grow the map BEFORE stepping right. Incrementing last_chunk_ first
            // could push it to map_size_ (one past the end); reallocate_map then
            // reads map_[last_chunk_] out of bounds. reserve_back() inspects the
            // CURRENT (in-bounds) last_chunk_ and re-centers if we're at the edge.
            reserve_back();
            ++last_chunk_;
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
     * @brief Remove front — destroy element, advance first_index_ or next chunk.
     *
     *     destroy map_[first_chunk_][first_index_]
     *     ++first_index_; if == CHUNK_SIZE: ++first_chunk_; first_index_ = 0
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
     * @brief Remove back — may step to previous chunk when last_index_ == 0.
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

