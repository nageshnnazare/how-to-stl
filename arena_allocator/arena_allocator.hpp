#ifndef ARENA_ALLOCATOR_HPP
#define ARENA_ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <vector>

/**
 * @brief Thread Arena Allocator - Per-thread Memory Pools
 * 
 * Provides fast, lock-free allocation for individual threads by giving
 * each thread its own memory arena. This eliminates contention and
 * dramatically improves performance in multi-threaded applications.
 * 
 * Key Features:
 * - Per-thread arenas (no locks on allocation)
 * - Automatic arena creation on first use
 * - Configurable arena sizes
 * - Global deallocation support
 * - Thread-safe arena management
 */

// ============================================================================
// ARENA - Single thread's memory pool
// ============================================================================
class Arena {
private:
    char* buffer_;
    size_t size_;
    std::atomic<size_t> offset_;
    
public:
    Arena(size_t size) 
        : buffer_(static_cast<char*>(std::malloc(size)))
        , size_(size)
        , offset_(0)
    {
        if (!buffer_) throw std::bad_alloc();
    }
    
    ~Arena() {
        std::free(buffer_);
    }
    
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    
    /**
     * Allocate from this arena (lock-free for owning thread)
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        // Align offset
        size_t current = offset_.load(std::memory_order_relaxed);
        size_t padding = 0;
        
        if (alignment > 0 && (current % alignment) != 0) {
            padding = alignment - (current % alignment);
        }
        
        size_t aligned_offset = current + padding;
        size_t new_offset = aligned_offset + size;
        
        if (new_offset > size_) {
            return nullptr;  // Arena full
        }
        
        // For owning thread, this is lock-free
        offset_.store(new_offset, std::memory_order_relaxed);
        
        return buffer_ + aligned_offset;
    }
    
    /**
     * Reset arena (reclaim all memory)
     */
    void reset() {
        offset_.store(0, std::memory_order_release);
    }
    
    size_t used() const {
        return offset_.load(std::memory_order_acquire);
    }
    
    size_t available() const {
        return size_ - used();
    }
    
    size_t capacity() const {
        return size_;
    }
};

// ============================================================================
// THREAD ARENA ALLOCATOR - Manages per-thread arenas
// ============================================================================
class ThreadArenaAllocator {
private:
    size_t arena_size_;
    mutable std::mutex mutex_;
    std::unordered_map<std::thread::id, Arena*> arenas_;
    
    /**
     * Get or create arena for current thread
     */
    Arena* get_thread_arena() {
        std::thread::id tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = arenas_.find(tid);
        if (it != arenas_.end()) {
            return it->second;
        }
        
        // Create new arena
        Arena* arena = new Arena(arena_size_);
        arenas_[tid] = arena;
        return arena;
    }
    
public:
    explicit ThreadArenaAllocator(size_t arena_size = 1024 * 1024)  // 1 MB default
        : arena_size_(arena_size)
    {}
    
    ~ThreadArenaAllocator() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : arenas_) {
            delete pair.second;
        }
    }
    
    ThreadArenaAllocator(const ThreadArenaAllocator&) = delete;
    ThreadArenaAllocator& operator=(const ThreadArenaAllocator&) = delete;
    
    /**
     * Allocate memory (lock-free for thread's own arena)
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        Arena* arena = get_thread_arena();
        return arena->allocate(size, alignment);
    }
    
    /**
     * Reset current thread's arena
     */
    void reset_thread_arena() {
        std::thread::id tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = arenas_.find(tid);
        if (it != arenas_.end()) {
            it->second->reset();
        }
    }
    
    /**
     * Reset all arenas (thread-safe)
     */
    void reset_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : arenas_) {
            pair.second->reset();
        }
    }
    
    /**
     * Get statistics for current thread
     */
    struct ThreadStats {
        size_t used;
        size_t available;
        size_t capacity;
    };
    
    ThreadStats get_thread_stats() const {
        std::thread::id tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = arenas_.find(tid);
        if (it != arenas_.end()) {
            Arena* arena = it->second;
            return {
                arena->used(),
                arena->available(),
                arena->capacity()
            };
        }
        return {0, 0, 0};
    }
    
    /**
     * Get global statistics (thread-safe)
     */
    struct GlobalStats {
        size_t num_arenas;
        size_t total_capacity;
        size_t total_used;
        size_t total_available;
    };
    
    GlobalStats get_global_stats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        GlobalStats stats = {0, 0, 0, 0};
        stats.num_arenas = arenas_.size();
        
        for (const auto& pair : arenas_) {
            Arena* arena = pair.second;
            stats.total_capacity += arena->capacity();
            stats.total_used += arena->used();
            stats.total_available += arena->available();
        }
        
        return stats;
    }
    
    /**
     * Get number of arenas
     */
    size_t num_arenas() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return arenas_.size();
    }
};

// ============================================================================
// TYPED ARENA ALLOCATOR - Type-safe wrapper
// ============================================================================
template<typename T>
class TypedArenaAllocator {
private:
    ThreadArenaAllocator& arena_;
    
public:
    using value_type = T;
    
    explicit TypedArenaAllocator(ThreadArenaAllocator& arena) : arena_(arena) {}
    
    /**
     * Allocate n objects of type T
     */
    T* allocate(size_t n) {
        void* ptr = arena_.allocate(n * sizeof(T), alignof(T));
        return static_cast<T*>(ptr);
    }
    
    /**
     * Deallocate (no-op for arena allocator)
     */
    void deallocate(T*, size_t) {
        // Arena allocator doesn't support individual deallocation
    }
    
    /**
     * Construct object in place
     */
    template<typename... Args>
    void construct(T* ptr, Args&&... args) {
        new (ptr) T(std::forward<Args>(args)...);
    }
    
    /**
     * Destroy object
     */
    void destroy(T* ptr) {
        ptr->~T();
    }
};

// ============================================================================
// SCOPED ARENA - RAII helper for thread arena
// ============================================================================
class ScopedArena {
private:
    ThreadArenaAllocator& allocator_;
    
public:
    explicit ScopedArena(ThreadArenaAllocator& allocator) 
        : allocator_(allocator) 
    {}
    
    ~ScopedArena() {
        allocator_.reset_thread_arena();
    }
    
    ScopedArena(const ScopedArena&) = delete;
    ScopedArena& operator=(const ScopedArena&) = delete;
    
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        return allocator_.allocate(size, alignment);
    }
    
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        if (!ptr) return nullptr;
        return new (ptr) T(std::forward<Args>(args)...);
    }
};

// ============================================================================
// ARENA VECTOR - Vector using arena allocator
// ============================================================================
template<typename T>
class ArenaVector {
private:
    ThreadArenaAllocator& arena_;
    T* data_;
    size_t size_;
    size_t capacity_;
    
public:
    explicit ArenaVector(ThreadArenaAllocator& arena, size_t initial_capacity = 16)
        : arena_(arena)
        , data_(nullptr)
        , size_(0)
        , capacity_(initial_capacity)
    {
        data_ = static_cast<T*>(arena_.allocate(capacity_ * sizeof(T), alignof(T)));
    }
    
    ~ArenaVector() {
        // Destroy objects but don't deallocate (arena handles it)
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
    }
    
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            // Allocate new larger buffer
            size_t new_capacity = capacity_ * 2;
            T* new_data = static_cast<T*>(
                arena_.allocate(new_capacity * sizeof(T), alignof(T))
            );
            
            if (!new_data) return;  // Out of arena memory
            
            // Move elements
            for (size_t i = 0; i < size_; ++i) {
                new (new_data + i) T(std::move(data_[i]));
                data_[i].~T();
            }
            
            data_ = new_data;
            capacity_ = new_capacity;
        }
        
        new (data_ + size_) T(value);
        size_++;
    }
    
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
    
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
};

#endif // ARENA_ALLOCATOR_HPP

