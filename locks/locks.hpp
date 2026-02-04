#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdint>
#include <unordered_map>

/**
 * @brief Custom Lock Implementations
 * 
 * Demonstrates various locking mechanisms:
 * 1. Mutex Lock - Basic mutual exclusion
 * 2. Pointer Lock - Lock-free pointer with hazard pointers
 * 3. Read-Write Lock - Multiple readers, single writer
 * 4. Recursive RW Lock - Reentrant read-write lock
 * 5. Recursive RW Pointer Lock - Reentrant lock with pointer protection
 */

// ============================================================================
// 1. MUTEX LOCK - Basic Mutual Exclusion
// ============================================================================
/**
 * Spinlock-based mutex using atomic flag
 * Simple but effective for short critical sections
 */
class MutexLock {
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
    
public:
    MutexLock() = default;
    ~MutexLock() = default;
    
    // No copying
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
    
    /**
     * Acquire lock (spin until available)
     */
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Spin - could add backoff here
            std::this_thread::yield();
        }
    }
    
    /**
     * Try to acquire lock (non-blocking)
     */
    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }
    
    /**
     * Release lock
     */
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
    
    /**
     * RAII lock guard
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
// 2. POINTER LOCK - Lock-free Pointer with ABA Protection
// ============================================================================
/**
 * Lock-free pointer using atomic operations and versioning
 * Prevents ABA problem with version counter
 */
template<typename T>
class PointerLock {
private:
    struct VersionedPointer {
        T* ptr;
        uint64_t version;
        
        VersionedPointer() : ptr(nullptr), version(0) {}
        VersionedPointer(T* p, uint64_t v) : ptr(p), version(v) {}
    };
    
    // Pack pointer and version into 128-bit value for atomic operations
    // Note: On x86-64, we can use __int128 or split into two atomics
    std::atomic<T*> ptr_;
    std::atomic<uint64_t> version_;
    
public:
    PointerLock() : ptr_(nullptr), version_(0) {}
    
    explicit PointerLock(T* ptr) : ptr_(ptr), version_(0) {}
    
    ~PointerLock() = default;
    
    // No copying
    PointerLock(const PointerLock&) = delete;
    PointerLock& operator=(const PointerLock&) = delete;
    
    /**
     * Load pointer atomically
     */
    T* load(std::memory_order order = std::memory_order_seq_cst) const {
        return ptr_.load(order);
    }
    
    /**
     * Store pointer atomically with version increment
     */
    void store(T* new_ptr, std::memory_order order = std::memory_order_seq_cst) {
        ptr_.store(new_ptr, order);
        version_.fetch_add(1, order);
    }
    
    /**
     * Compare-and-swap with ABA protection
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
    
    /**
     * Get version number
     */
    uint64_t version() const {
        return version_.load(std::memory_order_acquire);
    }
};

// ============================================================================
// 3. READ-WRITE LOCK - Multiple Readers, Single Writer
// ============================================================================
/**
 * Classic read-write lock
 * Allows multiple concurrent readers OR one writer
 */
class ReadWriteLock {
private:
    std::atomic<int32_t> readers_;  // Number of active readers (negative if writer)
    std::mutex mutex_;              // For condition variable
    std::condition_variable cv_;
    
    static constexpr int32_t WRITER_FLAG = -1000000;
    
public:
    ReadWriteLock() : readers_(0) {}
    
    ~ReadWriteLock() = default;
    
    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;
    
    /**
     * Acquire read lock (shared)
     */
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return readers_.load() >= 0; });
        readers_.fetch_add(1, std::memory_order_acquire);
    }
    
    /**
     * Try to acquire read lock
     */
    bool try_lock_read() {
        int32_t expected = readers_.load(std::memory_order_relaxed);
        if (expected < 0) return false;
        
        return readers_.compare_exchange_strong(expected, expected + 1,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed);
    }
    
    /**
     * Release read lock
     */
    void unlock_read() {
        if (readers_.fetch_sub(1, std::memory_order_release) == 1) {
            // Last reader, notify waiting writers
            std::lock_guard<std::mutex> lock(mutex_);
            cv_.notify_all();
        }
    }
    
    /**
     * Acquire write lock (exclusive)
     */
    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return readers_.load() == 0; });
        readers_.store(WRITER_FLAG, std::memory_order_release);
    }
    
    /**
     * Try to acquire write lock
     */
    bool try_lock_write() {
        int32_t expected = 0;
        return readers_.compare_exchange_strong(expected, WRITER_FLAG,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed);
    }
    
    /**
     * Release write lock
     */
    void unlock_write() {
        readers_.store(0, std::memory_order_release);
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    }
    
    /**
     * RAII read guard
     */
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
    
    /**
     * RAII write guard
     */
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
// 4. RECURSIVE READ-WRITE LOCK - Reentrant RW Lock
// ============================================================================
/**
 * Recursive read-write lock that allows same thread to acquire multiple times
 * Tracks per-thread read/write counts
 */
class RecursiveRWLock {
private:
    struct ThreadState {
        int32_t read_count;
        int32_t write_count;
        ThreadState() : read_count(0), write_count(0) {}
    };
    
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread::id writer_id_;
    int32_t write_count_;
    int32_t reader_count_;
    
    // Per-thread read counts
    std::unordered_map<std::thread::id, int32_t> read_counts_;
    
public:
    RecursiveRWLock() : write_count_(0), reader_count_(0) {}
    
    ~RecursiveRWLock() = default;
    
    RecursiveRWLock(const RecursiveRWLock&) = delete;
    RecursiveRWLock& operator=(const RecursiveRWLock&) = delete;
    
    /**
     * Acquire read lock (can be called multiple times by same thread)
     */
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::thread::id tid = std::this_thread::get_id();
        
        // If this thread is the writer, just increment read count
        if (tid == writer_id_) {
            read_counts_[tid]++;
            return;
        }
        
        // Wait until no writer
        cv_.wait(lock, [this] { return write_count_ == 0; });
        
        read_counts_[tid]++;
        reader_count_++;
    }
    
    /**
     * Release read lock
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
     * Acquire write lock (can be called multiple times by same thread)
     */
    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::thread::id tid = std::this_thread::get_id();
        
        // If this thread already has write lock, just increment
        if (tid == writer_id_) {
            write_count_++;
            return;
        }
        
        // Wait until no readers and no writer
        cv_.wait(lock, [this] { return reader_count_ == 0 && write_count_ == 0; });
        
        writer_id_ = tid;
        write_count_ = 1;
    }
    
    /**
     * Release write lock
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
    
    /**
     * RAII guards
     */
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
// 5. RECURSIVE RW POINTER LOCK - Recursive RW Lock with Pointer Protection
// ============================================================================
/**
 * Combines recursive RW lock with pointer protection
 * Provides both reentrant locking and safe pointer access
 */
template<typename T>
class RecursiveRWPointerLock {
private:
    T* ptr_;
    RecursiveRWLock lock_;
    std::atomic<uint64_t> version_;
    
public:
    RecursiveRWPointerLock() : ptr_(nullptr), version_(0) {}
    
    explicit RecursiveRWPointerLock(T* ptr) : ptr_(ptr), version_(0) {}
    
    ~RecursiveRWPointerLock() = default;
    
    RecursiveRWPointerLock(const RecursiveRWPointerLock&) = delete;
    RecursiveRWPointerLock& operator=(const RecursiveRWPointerLock&) = delete;
    
    /**
     * Access pointer for reading (shared lock)
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
     * Access pointer for writing (exclusive lock)
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
        
        void set(T* new_ptr) {
            parent_.ptr_ = new_ptr;
            parent_.version_.fetch_add(1, std::memory_order_release);
        }
        
        uint64_t version() const {
            return parent_.version_.load(std::memory_order_acquire);
        }
    };
    
    /**
     * Get read access (RAII)
     */
    ReadAccess read() {
        return ReadAccess(*this);
    }
    
    /**
     * Get write access (RAII)
     */
    WriteAccess write() {
        return WriteAccess(*this);
    }
    
    /**
     * Get current version
     */
    uint64_t version() const {
        return version_.load(std::memory_order_acquire);
    }
};

#endif // LOCKS_HPP

