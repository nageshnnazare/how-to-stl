#ifndef ARENA_ALLOCATOR_HPP
#define ARENA_ALLOCATOR_HPP

#include <cstddef>              // for size_t
#include <cstdint>             // for uintptr_t (address-based alignment)
#include <cstdlib>              // for malloc, free
#include <atomic>               // for atomic offset in Arena
#include <thread>               // for std::thread::id
#include <unordered_map>        // for per-thread arena registry
#include <mutex>                // for arena map protection
#include <vector>               // (used by ArenaVector growth path)

// ============================================================================
//  ThreadArenaAllocator — per-thread bump arenas for lock-free hot paths
// ============================================================================
//
// WHAT IT IS
// ----------
// A bump (linear) allocator per OS thread. Each thread gets its own contiguous
// slab; allocate() is pointer arithmetic on a local offset — no mutex on the
// hot path after the arena exists. Individual frees are no-ops; reset() or
// ScopedArena rewinds the whole arena at once.
//
// THE TWO-LAYER MODEL
// -------------------
//
//   ThreadArenaAllocator (global manager)          Per-thread Arena (hot path)
//   ┌─────────────────────────────┐               ┌─────────────────────────┐
//   │ mutex_                      │               │ buffer_  ──▶ [used|free] │
//   │ arenas_: thread_id → Arena* │──creates──▶   │ offset_  (atomic bump)   │
//   │ arena_size_ (template size) │               └─────────────────────────┘
//   └─────────────────────────────┘
//
// Thread 1 ──▶ Arena₁ (1 MB)     allocate(): offset += size  (no lock)
// Thread 2 ──▶ Arena₂ (1 MB)     allocate(): offset += size  (no lock)
// Thread 3 ──▶ Arena₃ (1 MB)
//
// First allocation on a thread takes mutex_ once to create/map its Arena.
// After that, the owning thread bumps offset_ with relaxed atomics only.
//
// WHY PER-THREAD ARENAS?
// ----------------------
// malloc contends on a global lock when many threads allocate simultaneously.
// Giving each thread a private slab removes cross-thread interference:
//
//     malloc:      T1 ──┐
//                  T2 ──┼──▶ [ GLOBAL HEAP LOCK ] ──▶ slow
//                  T3 ──┘
//
//     arena:      T1 ──▶ Arena₁  (lock-free bump)
//                 T2 ──▶ Arena₂  (lock-free bump)
//                 T3 ──▶ Arena₃  (lock-free bump)
//
// Trade-off: no individual free; memory is reclaimed only via reset().
// ============================================================================

// ============================================================================
//  Arena — single-thread bump buffer
// ============================================================================
//
// WHAT IT IS
// ----------
// One malloc slab with an atomic bump pointer. The owning thread calls
// allocate() without locking; offset_ advances with memory_order_relaxed
// because no other thread should write this Arena's offset_ in correct usage.
//
// THE THREE FIELDS
// ----------------
//
//     buffer_  -> base of the slab
//     size_    -> total capacity (immutable after construct)
//     offset_  -> atomic bump: bytes [0, offset_) are handed out
//
// MEMORY LAYOUT (offset_ = 200, size_ = 1024)
// --------------------------------------------
//
//     buffer_:
//     ┌──────── used ────────┬──────── available ────────┐
//     │ obj │ pad │ obj │pad │                           │
//     └──────────────────────┴───────────────────────────┘
//     0                      200                         1024
//                            ▲
//                         offset_ (bump advances right)
//
// allocate(64, align=16):
//     (1) load offset_ (relaxed)
//     (2) pad to alignment boundary
//     (3) if new_offset > size_, return nullptr
//     (4) store new_offset; return buffer_ + aligned_offset
//
// reset(): offset_.store(0, release) — bulk free entire arena
// ============================================================================
class Arena {
private:
    char* buffer_;                  // base of the malloc'd slab for this thread
    size_t size_;                   // total byte capacity (fixed for arena lifetime)
    std::atomic<size_t> offset_;    // bump pointer; only the owning thread should advance it

public:
  /**
   * @brief Allocate a fixed-size bump arena slab.
   *
   * Throws std::bad_alloc if malloc fails. offset_ starts at 0.
   */
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
     * @brief Bump-allocate `size` bytes with alignment rounding.
     *
     * Hot path for the owning thread (lock-free after arena exists):
     *
     *     current = offset_.load(relaxed)
     *     aligned = round_up(current, alignment)
     *     new_off = aligned + size
     *     if new_off > size_: return nullptr
     *     offset_.store(new_off, relaxed)
     *     return buffer_ + aligned
     *
     *     before:  [████ used ████|░░░░ free ░░░░]  offset_=aligned_start
     *     after:   [████ used ████│ NEW │░ free ░]  offset_=new_off
     *
     * WHY relaxed ordering: single-writer (owning thread) on offset_; no
     * cross-thread publish needed for the pointer itself. acquire/release
     * used in used()/reset() for stats visibility.
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t current = offset_.load(std::memory_order_relaxed);
        size_t padding = 0;

        // Align against the ABSOLUTE address (buffer_ + current), not just the
        // offset. malloc only guarantees alignof(max_align_t) (typically 16) for
        // buffer_, so aligning the offset alone would NOT make the returned
        // pointer 32/64-byte aligned when the slab's base isn't. We pad to the
        // next address that is a multiple of `alignment`.
        if (alignment > 0) {
            uintptr_t cur_addr = reinterpret_cast<uintptr_t>(buffer_) + current;
            size_t misalign = static_cast<size_t>(cur_addr % alignment);
            if (misalign != 0) {
                padding = alignment - misalign;
            }
        }

        size_t aligned_offset = current + padding;
        size_t new_offset = aligned_offset + size;

        if (new_offset > size_) {
            return nullptr;
        }

        offset_.store(new_offset, std::memory_order_relaxed);

        return buffer_ + aligned_offset;
    }

    /**
     * @brief Rewind bump pointer to zero — O(1) bulk reclaim.
     *
     *     offset_: 840 ──▶ 0
     *
     * Does not run destructors; destroy live objects first.
     * release semantics so other threads reading used() see the reset.
     */
    void reset() {
        offset_.store(0, std::memory_order_release);
    }

    /** @brief Current high-water mark (acquire load for visibility). */
    size_t used() const {
        return offset_.load(std::memory_order_acquire);
    }

    /** @brief Bytes remaining before allocate() returns nullptr. */
    size_t available() const {
        return size_ - used();
    }

    /** @brief Total slab capacity. */
    size_t capacity() const {
        return size_;
    }
};

// ============================================================================
//  ThreadArenaAllocator — maps each std::thread::id to an Arena
// ============================================================================
//
// WHAT IT IS
// ----------
// Lazy factory: first allocate() on a thread locks mutex_, creates a new Arena
// of arena_size_ bytes, stores it in arenas_[thread_id]. Subsequent allocs on
// that thread hit Arena::allocate() with no map lock.
//
// THE THREE FIELDS
// ----------------
//
//     arena_size_  -> bytes per newly created Arena
//     mutex_       -> protects arenas_ map (create, lookup, stats, destroy)
//     arenas_      -> unordered_map<thread::id, Arena*>
//
// LIFECYCLE DIAGRAM
// -----------------
//
//   thread T first call to allocate():
//       lock(mutex_) ──▶ find(T) miss ──▶ new Arena(arena_size_) ──▶ arenas_[T]=arena
//       unlock ──▶ arena->allocate(...)   // hot path, no lock
//
//   ~ThreadArenaAllocator():
//       lock ──▶ delete every Arena* ──▶ unlock
// ============================================================================
class ThreadArenaAllocator {
private:
    size_t arena_size_;                                     // capacity for each new Arena
    mutable std::mutex mutex_;                              // guards arenas_ map mutations
    std::unordered_map<std::thread::id, Arena*> arenas_;   // thread → private bump slab

    /**
     * @brief Look up or create the Arena for the calling thread.
     *
     *     lock mutex_
     *     if tid in arenas_: return existing Arena*
     *     else: arenas_[tid] = new Arena(arena_size_); return it
     *     unlock
     *
     * Called on every allocate(); after first call per thread, map hit is
     * fast but still takes mutex_. Production systems often use thread_local
     * cache to avoid even this lookup.
     */
    Arena* get_thread_arena() {
        std::thread::id tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = arenas_.find(tid);
        if (it != arenas_.end()) {
            return it->second;
        }

        Arena* arena = new Arena(arena_size_);
        arenas_[tid] = arena;
        return arena;
    }

public:
    /**
     * @brief Construct manager; arenas are created lazily per thread.
     *
     * @param arena_size bytes per thread slab (default 1 MiB).
     */
    explicit ThreadArenaAllocator(size_t arena_size = 1024 * 1024)
        : arena_size_(arena_size)
    {}

    /**
     * @brief Destroy all per-thread arenas (mutex held during teardown).
     */
    ~ThreadArenaAllocator() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : arenas_) {
            delete pair.second;
        }
    }

    ThreadArenaAllocator(const ThreadArenaAllocator&) = delete;
    ThreadArenaAllocator& operator=(const ThreadArenaAllocator&) = delete;

    /**
     * @brief Allocate from the current thread's arena.
     *
     *     caller thread ──▶ get_thread_arena() ──▶ Arena::allocate()
     *
     * Returns nullptr if this thread's arena is full (no automatic growth).
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        Arena* arena = get_thread_arena();
        return arena->allocate(size, alignment);
    }

    /**
     * @brief Reset only the calling thread's arena (offset → 0).
     *
     * Locks mutex_ to find the Arena*, then calls Arena::reset().
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
     * @brief Reset every thread's arena (e.g. between global phases).
     */
    void reset_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : arenas_) {
            pair.second->reset();
        }
    }

    /** @brief Per-thread memory usage snapshot. */
    struct ThreadStats {
        size_t used;
        size_t available;
        size_t capacity;
    };

    /**
     * @brief Stats for the calling thread's arena (zeros if none yet).
     */
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

    /** @brief Aggregated stats across all live arenas. */
    struct GlobalStats {
        size_t num_arenas;
        size_t total_capacity;
        size_t total_used;
        size_t total_available;
    };

    /**
     * @brief Sum capacity/used across arenas_.size() entries.
     */
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

    /** @brief Number of thread arenas created so far. */
    size_t num_arenas() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return arenas_.size();
    }
};

// ============================================================================
//  TypedArenaAllocator<T> — STL-style typed wrapper
// ============================================================================
template<typename T>
class TypedArenaAllocator {
private:
    ThreadArenaAllocator& arena_;     // non-owning reference to the bump manager

public:
    using value_type = T;

    explicit TypedArenaAllocator(ThreadArenaAllocator& arena) : arena_(arena) {}

    /**
     * @brief Allocate raw storage for n objects of T (no construction).
     */
    T* allocate(size_t n) {
        void* ptr = arena_.allocate(n * sizeof(T), alignof(T));
        return static_cast<T*>(ptr);
    }

    /**
     * @brief Deallocate is a no-op — arena owns the bytes until reset.
     */
    void deallocate(T*, size_t) {
    }

    /**
     * @brief Placement-construct one T at ptr.
     */
    template<typename... Args>
    void construct(T* ptr, Args&&... args) {
        new (ptr) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Explicitly destroy one T (storage remains in arena).
     */
    void destroy(T* ptr) {
        ptr->~T();
    }
};

// ============================================================================
//  ScopedArena — RAII reset of current thread's arena on scope exit
// ============================================================================
//
//     { ScopedArena scope(alloc);
//         scope.allocate(...)   // bump during request/frame
//     }  // destructor calls reset_thread_arena() — all bytes reclaimed
//
// Prevents forgotten resets in request handlers and parsers.
// ============================================================================
class ScopedArena {
private:
    ThreadArenaAllocator& allocator_;   // arena to reset when this scope ends

public:
    explicit ScopedArena(ThreadArenaAllocator& allocator)
        : allocator_(allocator)
    {}

    /**
     * @brief Bulk-free this thread's arena (RAII cleanup).
     */
    ~ScopedArena() {
        allocator_.reset_thread_arena();
    }

    ScopedArena(const ScopedArena&) = delete;
    ScopedArena& operator=(const ScopedArena&) = delete;

    /** @brief Forward allocate to the underlying manager. */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        return allocator_.allocate(size, alignment);
    }

    /**
     * @brief Allocate + placement-new one T.
     *
     * Returns nullptr if arena exhausted (no construction attempted).
     */
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        if (!ptr) return nullptr;
        return new (ptr) T(std::forward<Args>(args)...);
    }
};

// ============================================================================
//  ArenaVector<T> — growable vector backed by arena storage
// ============================================================================
//
// WHAT IT IS
// ----------
// Like a minimal vector, but storage comes from ThreadArenaAllocator bumps.
// Growth allocates a NEW larger slab in the arena and move-constructs elements;
// old slab bytes are abandoned (arena reset reclaims everything later).
//
// THE FOUR FIELDS
// ---------------
//
//     arena_     -> reference to bump allocator
//     data_      -> pointer to current element array in arena
//     size_      -> live element count
//     capacity_  -> slots in current data_ block
//
// GROWTH (size_ == capacity_)
// ---------------------------
//
//     old data_ ──▶ [A][B][C]     full
//     new_data ──▶ [A][B][C][ ][ ]  (new bump, double capacity)
//     data_ = new_data; old bytes left as garbage until arena reset
// ============================================================================
template<typename T>
class ArenaVector {
private:
    ThreadArenaAllocator& arena_;   // source of all raw storage bumps
    T* data_;                       // pointer into arena for current buffer
    size_t size_;                   // number of constructed elements
    size_t capacity_;               // slots available before regrow

public:
    /**
     * @brief Reserve initial_capacity slots from the arena (raw bytes only).
     */
    explicit ArenaVector(ThreadArenaAllocator& arena, size_t initial_capacity = 16)
        : arena_(arena)
        , data_(nullptr)
        , size_(0)
        , capacity_(initial_capacity)
    {
        data_ = static_cast<T*>(arena_.allocate(capacity_ * sizeof(T), alignof(T)));
    }

    /**
     * @brief Destroy live elements; arena bytes reclaimed only on reset().
     */
    ~ArenaVector() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
    }

    /**
     * @brief Append one element; double capacity via new arena bump if full.
     *
     *     if size_ >= capacity_:
     *         new_data = arena_.allocate(2 * capacity_ * sizeof(T))
     *         move-construct elements into new_data; destroy old
     *         data_ = new_data; capacity_ *= 2
     *     construct_at(data_ + size_, value); ++size_
     */
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ * 2;
            T* new_data = static_cast<T*>(
                arena_.allocate(new_capacity * sizeof(T), alignof(T))
            );

            if (!new_data) return;

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
