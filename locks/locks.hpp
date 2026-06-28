#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <atomic>               // for spinlock, pointer CAS, RW counters
#include <thread>               // for yield, thread::id
#include <mutex>                // for std::mutex backing condition_variable
#include <condition_variable>   // for RW lock waiting
#include <chrono>               // (available for timed extensions)
#include <cstdint>              // for int32_t, uint64_t
#include <unordered_map>        // for per-thread read counts in RecursiveRWLock

// ============================================================================
//  Locks — five hand-rolled synchronization primitives
// ============================================================================
//
// WHAT THIS HEADER IS
// -------------------
// Mutual exclusion and reader/writer coordination built from atomics, mutexes,
// and condition variables — no opaque pthread wrappers. Each class targets a
// different contention pattern: spin for micro-critical sections, CAS pointers
// for lock-free stacks, RW locks for read-heavy shared data.
//
// THE FIVE PRIMITIVES
// -------------------
//
//   MutexLock                 spinlock via atomic_flag + RAII Guard
//   PointerLock<T>            atomic pointer + version (ABA awareness)
//   ReadWriteLock             many readers OR one writer
//   RecursiveRWLock           same thread may re-enter read/write
//   RecursiveRWPointerLock<T> recursive RW + versioned pointer access
//
// CONTENTION PICTURE (two threads, one MutexLock)
// -----------------------------------------------
//
//     Thread A                          Thread B
//        │                                 │
//        ├─ lock(): TAS → acquired         ├─ lock(): TAS → SPIN
//        │   [ critical section ]          │       yield() loop...
//        ├─ unlock(): clear flag           │
//        │                                 ├─ lock(): TAS → acquired
//
// WHY RAII GUARDS?
// ----------------
// Manual lock/unlock leaks on exceptions and early returns. Guards tie unlock
// to scope exit — the same pattern as std::lock_guard.
// ============================================================================

// ============================================================================
//  MutexLock — spinlock on std::atomic_flag
// ============================================================================
//
// WHAT IT IS
// ----------
// A test-and-set spinlock: lock() loops on flag_.test_and_set(acquire) until
// it succeeds; unlock() clears the flag with release semantics. Yields while
// spinning to reduce bus traffic under contention.
//
// THE ONE FIELD
// -------------
//
//     flag_  -> std::atomic_flag (cleared = free, set = held)
//
// LOCK ACQUIRE LOOP
// -----------------
//
//     FREE:  flag_ = 0
//     Thread A: test_and_set → was 0 → now 1 → ENTER critical section
//     Thread B: test_and_set → was 1 → spin → yield → retry ...
//     Thread A: clear → flag_ = 0
//     Thread B: test_and_set → was 0 → acquired
//
// Best for very short critical sections; long holds waste CPU while others spin.
// ============================================================================
class MutexLock {
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;  // false = unlocked, true = locked

public:
    MutexLock() = default;
    ~MutexLock() = default;

    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;

    /**
     * @brief Acquire the lock, spinning until successful.
     *
     *     while test_and_set(acquire) == already_set:
     *         yield()    // backoff — avoid hammering the cache line
     *
     * acquire on success: subsequent reads/writes cannot move before lock.
     */
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    /** @brief Try once; return true if lock acquired without blocking. */
    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    /**
     * @brief Release the lock.
     *
     *     flag_.clear(release)  — publishes that critical section ended
     */
    void unlock() {
        flag_.clear(std::memory_order_release);
    }

    /**
     * RAII guard — locks in ctor, unlocks in dtor.
     *
     *     { Guard g(mutex);  ... }  // unlock even if throw
     *
     * WHY: prevents forgotten unlock() on every exit path.
     */
    class Guard {
        MutexLock& lock_;
    public:
        explicit Guard(MutexLock& lock) : lock_(lock) {
            lock_.lock();
        }
        ~Guard() {
            lock_.unlock();
        }
        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;
    };
};

// ============================================================================
//  PointerLock<T> — atomic pointer with version counter
// ============================================================================
//
// WHAT IT IS
// ----------
// Wraps std::atomic<T*> with a separate version_ counter incremented on every
// successful store or compare_exchange. Teaches ABA awareness for lock-free stacks.
//
// THE TWO FIELDS
// --------------
//
//     ptr_      -> current head / shared pointer
//     version_  -> monotonic change counter (not a full 128-bit tagged pointer)
//
// LOCK-FREE STACK PUSH (conceptual)
// ---------------------------------
//
//     head ──▶ [A]──▶ [B]
//     push C:
//         loop: old = load(head); C->next = old;
//               if CAS(head, old, C): version++; break
//
// ABA: if old head address reappears after pop/push cycle, version differs on
// a full tagged-pointer scheme; here version_ increments on each mutation.
// ============================================================================
template<typename T>
class PointerLock {
private:
    struct VersionedPointer {
        T* ptr;
        uint64_t version;

        VersionedPointer() : ptr(nullptr), version(0) {}
        VersionedPointer(T* p, uint64_t v) : ptr(p), version(v) {}
    };

    std::atomic<T*> ptr_;           // lock-free pointer payload
    std::atomic<uint64_t> version_; // bumped on each successful mutation

public:
    PointerLock() : ptr_(nullptr), version_(0) {}

    explicit PointerLock(T* ptr) : ptr_(ptr), version_(0) {}

    ~PointerLock() = default;

    PointerLock(const PointerLock&) = delete;
    PointerLock& operator=(const PointerLock&) = delete;

    /** @brief Atomic load of ptr_ with caller-chosen memory order. */
    T* load(std::memory_order order = std::memory_order_seq_cst) const {
        return ptr_.load(order);
    }

    /**
     * @brief Store new pointer and increment version.
     *
     *     ptr_ = new_ptr; version_++
     */
    void store(T* new_ptr, std::memory_order order = std::memory_order_seq_cst) {
        ptr_.store(new_ptr, order);
        version_.fetch_add(1, order);
    }

    /**
     * @brief CAS ptr_; increment version on success.
     *
     *     if ptr_.CAS(expected, desired): version_++; return true
     *     else: expected updated; return false → retry loop
     */
    bool compare_exchange_weak(T*& expected, T* desired) {
        if (ptr_.compare_exchange_weak(expected, desired,
                                       std::memory_order_release,
                                       std::memory_order_relaxed)) {
            version_.fetch_add(1, std::memory_order_release);
            return true;
        }
        return false;
    }

    bool compare_exchange_strong(T*& expected, T* desired) {
        if (ptr_.compare_exchange_strong(expected, desired,
                                         std::memory_order_release,
                                         std::memory_order_relaxed)) {
            version_.fetch_add(1, std::memory_order_release);
            return true;
        }
        return false;
    }

    /** @brief Observe how many successful mutations occurred. */
    uint64_t version() const {
        return version_.load(std::memory_order_acquire);
    }
};

// ============================================================================
//  ReadWriteLock — shared readers OR exclusive writer
// ============================================================================
//
// WHAT IT IS
// ----------
// readers_ counts active readers (>= 0). A writer sets readers_ to WRITER_FLAG
// (large negative sentinel). mutex_ + cv_ block until the desired state is safe.
//
// THE THREE FIELDS
// ----------------
//
//     readers_  -> atomic reader count; WRITER_FLAG means writer holds lock
//     mutex_    -> protects condition_variable predicate checks
//     cv_       -> parks threads until readers_ state allows progress
//
// STATE MACHINE
// -------------
//
//     readers_ == 0        idle — writer may enter
//     readers_ > 0         N concurrent readers
//     readers_ == WRITER   exclusive writer — readers and writers wait
//
//     Readers:  wait until readers_ >= 0; fetch_add(1)
//     Writer:   wait until readers_ == 0; store(WRITER_FLAG)
// ============================================================================
class ReadWriteLock {
private:
    std::atomic<int32_t> readers_;    // >=0 readers; WRITER_FLAG = writer active
    std::mutex mutex_;                  // serializes waiters on condition_variable
    std::condition_variable cv_;        // sleep instead of spin when blocked

    static constexpr int32_t WRITER_FLAG = -1000000;

public:
    ReadWriteLock() : readers_(0) {}

    ~ReadWriteLock() = default;

    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;

    /**
     * @brief Acquire shared read lock.
     *
     *     lock(mutex_)
     *     wait while readers_ < 0   // writer active
     *     readers_++
     *     unlock(mutex_)
     */
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return readers_.load() >= 0; });
        readers_.fetch_add(1, std::memory_order_acquire);
    }

    /** @brief Non-blocking read lock; fails if writer holds lock. */
    bool try_lock_read() {
        int32_t expected = readers_.load(std::memory_order_relaxed);
        if (expected < 0) return false;

        return readers_.compare_exchange_strong(expected, expected + 1,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed);
    }

    /**
     * @brief Release read lock; wake writers if last reader exits.
     *
     *     if fetch_sub(1) == 1: notify_all()  // was last reader
     */
    void unlock_read() {
        if (readers_.fetch_sub(1, std::memory_order_release) == 1) {
            std::lock_guard<std::mutex> lock(mutex_);
            cv_.notify_all();
        }
    }

    /**
     * @brief Acquire exclusive write lock.
     *
     *     wait while readers_ != 0
     *     readers_ = WRITER_FLAG
     */
    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return readers_.load() == 0; });
        readers_.store(WRITER_FLAG, std::memory_order_release);
    }

    /** @brief Non-blocking write lock; succeeds only when idle. */
    bool try_lock_write() {
        int32_t expected = 0;
        return readers_.compare_exchange_strong(expected, WRITER_FLAG,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed);
    }

    /**
     * @brief Release write lock and wake all waiters.
     *
     *     readers_ = 0; notify_all()
     */
    void unlock_write() {
        readers_.store(0, std::memory_order_release);
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    }

    /** RAII shared read guard. */
    class ReadGuard {
        ReadWriteLock& lock_;
    public:
        explicit ReadGuard(ReadWriteLock& lock) : lock_(lock) {
            lock_.lock_read();
        }
        ~ReadGuard() {
            lock_.unlock_read();
        }
        ReadGuard(const ReadGuard&) = delete;
        ReadGuard& operator=(const ReadGuard&) = delete;
    };

    /** RAII exclusive write guard. */
    class WriteGuard {
        ReadWriteLock& lock_;
    public:
        explicit WriteGuard(ReadWriteLock& lock) : lock_(lock) {
            lock_.lock_write();
        }
        ~WriteGuard() {
            lock_.unlock_write();
        }
        WriteGuard(const WriteGuard&) = delete;
        WriteGuard& operator=(const WriteGuard&) = delete;
    };
};

// ============================================================================
//  RecursiveRWLock — reentrant reader/writer with per-thread counts
// ============================================================================
//
// WHAT IT IS
// ----------
// Tracks writer_id_ + write_count_ for exclusive reentrancy, and read_counts_
// map for per-thread nested read locks. Writer thread may also nest read locks.
//
// THE FIVE FIELDS
// ---------------
//
//     mutex_, cv_        -> block until policy allows
//     writer_id_          -> thread::id holding write lock (or default)
//     write_count_        -> nested write depth for writer_id_
//     reader_count_       -> total foreign readers (excludes writer's reads)
//     read_counts_        -> map tid → nested read depth
//
// REENTRANT WRITE (same thread)
// -----------------------------
//
//     Thread T: lock_write() → writer_id_=T, write_count_=1
//     Thread T: lock_write() → same tid → write_count_++  (no block)
//     Thread T: unlock_write() → write_count_--; if 0 → release
// ============================================================================
class RecursiveRWLock {
private:
    struct ThreadState {
        int32_t read_count;
        int32_t write_count;
        ThreadState() : read_count(0), write_count(0) {}
    };

    std::mutex mutex_;                                      // protects all state below
    std::condition_variable cv_;                            // wait for writer/readers
    std::thread::id writer_id_;                             // current exclusive owner
    int32_t write_count_;                                   // nested write depth
    int32_t reader_count_;                                  // non-writer readers active
    std::unordered_map<std::thread::id, int32_t> read_counts_;  // per-thread read depth

public:
    RecursiveRWLock() : write_count_(0), reader_count_(0) {}

    ~RecursiveRWLock() = default;

    RecursiveRWLock(const RecursiveRWLock&) = delete;
    RecursiveRWLock& operator=(const RecursiveRWLock&) = delete;

    /**
     * @brief Acquire read lock; same thread may call repeatedly.
     *
     * If caller is writer_id_: bump read_counts_[tid] only (no wait).
     * Else: wait until write_count_==0; increment reader_count_.
     */
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::thread::id tid = std::this_thread::get_id();

        if (tid == writer_id_) {
            read_counts_[tid]++;
            return;
        }

        cv_.wait(lock, [this] { return write_count_ == 0; });

        read_counts_[tid]++;
        reader_count_++;
    }

    /**
     * @brief Drop one read nesting level; notify writers when last reader gone.
     */
    void unlock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::thread::id tid = std::this_thread::get_id();

        auto it = read_counts_.find(tid);
        if (it != read_counts_.end() && it->second > 0) {
            it->second--;
            if (tid != writer_id_) {
                reader_count_--;
            }

            if (it->second == 0) {
                read_counts_.erase(it);
            }

            if (reader_count_ == 0) {
                cv_.notify_all();
            }
        }
    }

    /**
     * @brief Acquire write lock; same thread may re-enter.
     *
     * If tid == writer_id_: write_count_++
     * Else: wait until reader_count_==0 && write_count_==0; claim writer_id_
     */
    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::thread::id tid = std::this_thread::get_id();

        if (tid == writer_id_) {
            write_count_++;
            return;
        }

        cv_.wait(lock, [this] { return reader_count_ == 0 && write_count_ == 0; });

        writer_id_ = tid;
        write_count_ = 1;
    }

    /**
     * @brief Drop one write nesting level; release exclusivity at depth 0.
     */
    void unlock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::thread::id tid = std::this_thread::get_id();

        if (tid == writer_id_ && write_count_ > 0) {
            write_count_--;

            if (write_count_ == 0) {
                writer_id_ = std::thread::id();
                cv_.notify_all();
            }
        }
    }

    class ReadGuard {
        RecursiveRWLock& lock_;
    public:
        explicit ReadGuard(RecursiveRWLock& lock) : lock_(lock) {
            lock_.lock_read();
        }
        ~ReadGuard() {
            lock_.unlock_read();
        }
    };

    class WriteGuard {
        RecursiveRWLock& lock_;
    public:
        explicit WriteGuard(RecursiveRWLock& lock) : lock_(lock) {
            lock_.lock_write();
        }
        ~WriteGuard() {
            lock_.unlock_write();
        }
    };
};

// ============================================================================
//  RecursiveRWPointerLock<T> — recursive RW lock around a shared T*
// ============================================================================
//
// WHAT IT IS
// ----------
// Combines RecursiveRWLock with a plain T* and atomic version_. read() returns
// ReadAccess (shared); write() returns WriteAccess (exclusive + set()).
//
// THE THREE FIELDS
// ----------------
//
//     ptr_      -> protected object address (non-atomic; guarded by lock_)
//     lock_     -> RecursiveRWLock serializing access
//     version_  -> incremented on WriteAccess::set()
//
// ACCESS PATTERN
// --------------
//
//     auto r = lock.read();   // ReadGuard inside ReadAccess
//     use r->field;           // many threads OK
//
//     auto w = lock.write();  // WriteGuard inside WriteAccess
//     w->field = x; w.set(new_ptr);
// ============================================================================
template<typename T>
class RecursiveRWPointerLock {
private:
    T* ptr_;                            // guarded payload pointer
    RecursiveRWLock lock_;              // reentrant RW synchronization
    std::atomic<uint64_t> version_;     // bumps when pointer swapped via set()

public:
    RecursiveRWPointerLock() : ptr_(nullptr), version_(0) {}

    explicit RecursiveRWPointerLock(T* ptr) : ptr_(ptr), version_(0) {}

    ~RecursiveRWPointerLock() = default;

    RecursiveRWPointerLock(const RecursiveRWPointerLock&) = delete;
    RecursiveRWPointerLock& operator=(const RecursiveRWPointerLock&) = delete;

    /**
     * RAII shared read view — holds ReadGuard for lock_ lifetime.
     */
    class ReadAccess {
        RecursiveRWPointerLock& parent_;
        typename RecursiveRWLock::ReadGuard guard_;

    public:
        explicit ReadAccess(RecursiveRWPointerLock& parent)
            : parent_(parent), guard_(parent.lock_) {}

        const T* operator->() const { return parent_.ptr_; }
        const T& operator*() const { return *parent_.ptr_; }
        const T* get() const { return parent_.ptr_; }

        uint64_t version() const {
            return parent_.version_.load(std::memory_order_acquire);
        }
    };

    /**
     * RAII exclusive write view — may call set() to swap ptr_ and bump version.
     */
    class WriteAccess {
        RecursiveRWPointerLock& parent_;
        typename RecursiveRWLock::WriteGuard guard_;

    public:
        explicit WriteAccess(RecursiveRWPointerLock& parent)
            : parent_(parent), guard_(parent.lock_) {}

        T* operator->() { return parent_.ptr_; }
        T& operator*() { return *parent_.ptr_; }
        T* get() { return parent_.ptr_; }

        /**
         * @brief Replace guarded pointer and publish version change.
         */
        void set(T* new_ptr) {
            parent_.ptr_ = new_ptr;
            parent_.version_.fetch_add(1, std::memory_order_release);
        }

        uint64_t version() const {
            return parent_.version_.load(std::memory_order_acquire);
        }
    };

    /** @brief Construct ReadAccess with shared lock held. */
    ReadAccess read() {
        return ReadAccess(*this);
    }

    /** @brief Construct WriteAccess with exclusive lock held. */
    WriteAccess write() {
        return WriteAccess(*this);
    }

    /** @brief Current version without taking a lock (racy for strict protocols). */
    uint64_t version() const {
        return version_.load(std::memory_order_acquire);
    }
};

#endif // LOCKS_HPP
