#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include <utility>      // for std::move, std::forward
#include <type_traits>  // for std::remove_extent, std::is_array
#include <cstddef>      // for std::nullptr_t
#include <atomic>       // for std::atomic (thread-safe reference counting)

/**
 * @brief Custom implementation of std::shared_ptr
 * 
 * shared_ptr is a smart pointer that retains shared ownership of an object through a pointer.
 * Multiple shared_ptr instances can own the same object. The object is destroyed when:
 * - The last remaining shared_ptr owning the object is destroyed, OR
 * - The last remaining shared_ptr owning the object is assigned another pointer
 * 
 * Key characteristics:
 * - Shared ownership: Multiple shared_ptr can own the same object
 * - Reference counting: Keeps track of how many shared_ptr own the object
 * - Thread-safe counting: Reference count updates are atomic
 * - Copyable: Can be copied to share ownership
 * - Moveable: Can be moved for efficiency
 */

// Forward declarations
template<typename T> class SharedPtr;
template<typename T> class WeakPtr;

/**
 * @brief Control block that manages reference counting and deletion
 * 
 * This is the heart of shared_ptr. It stores:
 * - The managed pointer
 * - Strong reference count (number of shared_ptr instances)
 * - Weak reference count (number of weak_ptr instances)
 * - The deleter function
 */
template<typename T, typename Deleter>
class ControlBlock {
private:
    T* ptr_;                              // Pointer to managed object
    std::atomic<long> shared_count_;      // Strong reference count
    std::atomic<long> weak_count_;        // Weak reference count
    Deleter deleter_;                     // Deleter for the managed object

public:
    /**
     * @brief Construct control block with pointer and deleter
     */
    ControlBlock(T* ptr, Deleter deleter)
        : ptr_(ptr)
        , shared_count_(1)       // Start with 1 shared owner
        , weak_count_(0)         // No weak owners initially
        , deleter_(deleter) {}

    /**
     * @brief Destructor - should only be called when both counts reach 0
     */
    ~ControlBlock() = default;

    // Prevent copying and moving
    ControlBlock(const ControlBlock&) = delete;
    ControlBlock& operator=(const ControlBlock&) = delete;

    /**
     * @brief Increment shared reference count
     */
    void add_shared_ref() {
        shared_count_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Decrement shared reference count
     * @return true if this was the last shared reference
     */
    bool release_shared_ref() {
        // Decrement and check if we're the last owner
        if (shared_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // We were the last shared owner - delete the object
            if (ptr_) {
                deleter_(ptr_);
                ptr_ = nullptr;
            }
            
            // Now check if there are any weak references
            if (weak_count_.load(std::memory_order_acquire) == 0) {
                return true;  // No weak refs, can delete control block
            }
        }
        return false;
    }

    /**
     * @brief Increment weak reference count
     */
    void add_weak_ref() {
        weak_count_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Decrement weak reference count
     * @return true if this was the last weak reference and no shared refs exist
     */
    bool release_weak_ref() {
        if (weak_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // We were the last weak reference
            if (shared_count_.load(std::memory_order_acquire) == 0) {
                return true;  // No shared refs either, can delete control block
            }
        }
        return false;
    }

    /**
     * @brief Get current shared reference count
     */
    long use_count() const {
        return shared_count_.load(std::memory_order_acquire);
    }

    /**
     * @brief Get the managed pointer
     */
    T* get() const {
        return ptr_;
    }

    /**
     * @brief Try to lock weak reference (for weak_ptr::lock())
     * @return true if successful (shared count was > 0)
     */
    bool try_add_shared_ref() {
        long count = shared_count_.load(std::memory_order_acquire);
        while (count > 0) {
            // Try to increment if still > 0
            if (shared_count_.compare_exchange_weak(count, count + 1,
                                                     std::memory_order_acq_rel,
                                                     std::memory_order_acquire)) {
                return true;
            }
            // compare_exchange_weak updates count on failure, retry
        }
        return false;  // Object already deleted
    }
};

/**
 * @brief Default deleter for shared_ptr
 */
template<typename T>
struct SharedPtrDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

/**
 * @brief Main SharedPtr implementation
 * 
 * @tparam T The type of the managed object
 */
template<typename T>
class SharedPtr {
private:
    T* ptr_;                                    // Pointer to managed object (for quick access)
    ControlBlock<T, SharedPtrDeleter<T>>* cb_;  // Pointer to control block

    // Friend declarations for cross-type access
    template<typename U> friend class SharedPtr;
    template<typename U> friend class WeakPtr;
    
    // Friend declarations for cast functions
    template<typename U, typename V>
    friend SharedPtr<U> static_pointer_cast(const SharedPtr<V>& ptr);
    
    template<typename U, typename V>
    friend SharedPtr<U> dynamic_pointer_cast(const SharedPtr<V>& ptr);

    /**
     * @brief Private constructor from control block (used internally)
     * @param add_ref If true, increments the reference count
     */
    SharedPtr(T* ptr, ControlBlock<T, SharedPtrDeleter<T>>* cb, bool add_ref = true)
        : ptr_(ptr), cb_(cb) {
        if (cb_ && add_ref) {
            cb_->add_shared_ref();
        }
    }

public:
    // Type aliases
    using element_type = T;

    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /**
     * @brief Default constructor - creates an empty shared_ptr
     */
    constexpr SharedPtr() noexcept : ptr_(nullptr), cb_(nullptr) {}

    /**
     * @brief Nullptr constructor - creates an empty shared_ptr
     */
    constexpr SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), cb_(nullptr) {}

    /**
     * @brief Construct from raw pointer
     * @param p Raw pointer to take ownership of
     * 
     * Creates a new control block and takes ownership of the pointer.
     * If construction fails, the pointer is deleted to prevent leaks.
     */
    explicit SharedPtr(T* p) : ptr_(p), cb_(nullptr) {
        if (p) {
            try {
                cb_ = new ControlBlock<T, SharedPtrDeleter<T>>(p, SharedPtrDeleter<T>());
            } catch (...) {
                delete p;  // Clean up on failure
                throw;
            }
        }
    }

    /**
     * @brief Copy constructor
     * Shares ownership with other - increments reference count
     */
    SharedPtr(const SharedPtr& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->add_shared_ref();
        }
    }

    /**
     * @brief Copy constructor from compatible type
     * Allows conversion between compatible pointer types (e.g., Derived* to Base*)
     */
    template<typename U,
             typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharedPtr(const SharedPtr<U>& other) noexcept
        : ptr_(other.ptr_), cb_(reinterpret_cast<ControlBlock<T, SharedPtrDeleter<T>>*>(other.cb_)) {
        if (cb_) {
            cb_->add_shared_ref();
        }
    }

    /**
     * @brief Move constructor
     * Transfers ownership from other without modifying reference count
     */
    SharedPtr(SharedPtr&& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    /**
     * @brief Move constructor from compatible type
     */
    template<typename U,
             typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharedPtr(SharedPtr<U>&& other) noexcept
        : ptr_(other.ptr_), cb_(reinterpret_cast<ControlBlock<T, SharedPtrDeleter<T>>*>(other.cb_)) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    /**
     * @brief Construct from weak_ptr (aliasing constructor for WeakPtr::lock())
     */
    explicit SharedPtr(const WeakPtr<T>& weak);

    // ============================================================================
    // DESTRUCTOR
    // ============================================================================

    /**
     * @brief Destructor
     * Decrements reference count and deletes object if this was the last owner
     */
    ~SharedPtr() {
        if (cb_) {
            if (cb_->release_shared_ref()) {
                // No more shared or weak references - delete control block
                delete cb_;
            }
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Copy assignment operator
     * Shares ownership with other
     */
    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this != &other) {
            // Create temporary to ensure proper cleanup even if exceptions occur
            SharedPtr temp(other);
            swap(temp);
        }
        return *this;
    }

    /**
     * @brief Copy assignment from compatible type
     */
    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) noexcept {
        SharedPtr temp(other);
        swap(temp);
        return *this;
    }

    /**
     * @brief Move assignment operator
     */
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            SharedPtr temp(std::move(other));
            swap(temp);
        }
        return *this;
    }

    /**
     * @brief Move assignment from compatible type
     */
    template<typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        SharedPtr temp(std::move(other));
        swap(temp);
        return *this;
    }

    /**
     * @brief Assignment from nullptr
     * Resets to empty state
     */
    SharedPtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Replace the managed object
     * @param p New pointer to manage (default: nullptr)
     */
    void reset(T* p = nullptr) {
        if (p == ptr_) {
            return;  // Same pointer, nothing to do
        }
        
        SharedPtr temp(p);
        swap(temp);
    }

    /**
     * @brief Swap contents with another shared_ptr
     */
    void swap(SharedPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(cb_, other.cb_);
    }

    // ============================================================================
    // OBSERVERS
    // ============================================================================

    /**
     * @brief Get the raw pointer without affecting ownership
     * @return Raw pointer to the managed object (or nullptr)
     */
    T* get() const noexcept {
        return ptr_;
    }

    /**
     * @brief Get the number of shared_ptr instances managing this object
     * @return Reference count (0 if empty)
     */
    long use_count() const noexcept {
        return cb_ ? cb_->use_count() : 0;
    }

    /**
     * @brief Check if this is the only shared_ptr managing the object
     * @return true if use_count() == 1
     */
    bool unique() const noexcept {
        return use_count() == 1;
    }

    /**
     * @brief Check if the shared_ptr manages an object
     * @return true if managing an object, false if empty
     */
    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    // ============================================================================
    // DEREFERENCE OPERATORS
    // ============================================================================

    /**
     * @brief Dereference the managed object
     * Behavior is undefined if get() == nullptr
     */
    T& operator*() const noexcept {
        return *ptr_;
    }

    /**
     * @brief Access member of the managed object
     * Behavior is undefined if get() == nullptr
     */
    T* operator->() const noexcept {
        return ptr_;
    }
};

/**
 * @brief WeakPtr - non-owning observer of SharedPtr
 * 
 * weak_ptr holds a non-owning reference to an object managed by shared_ptr.
 * It must be converted to shared_ptr to access the object.
 * 
 * Use cases:
 * - Break circular references
 * - Cache objects without preventing deletion
 * - Observer pattern
 */
template<typename T>
class WeakPtr {
private:
    T* ptr_;                                    // Pointer to managed object
    ControlBlock<T, SharedPtrDeleter<T>>* cb_;  // Pointer to control block

    template<typename U> friend class SharedPtr;
    template<typename U> friend class WeakPtr;

public:
    // Type aliases
    using element_type = T;

    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /**
     * @brief Default constructor - creates empty weak_ptr
     */
    constexpr WeakPtr() noexcept : ptr_(nullptr), cb_(nullptr) {}

    /**
     * @brief Copy constructor from WeakPtr
     */
    WeakPtr(const WeakPtr& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->add_weak_ref();
        }
    }

    /**
     * @brief Copy constructor from SharedPtr
     * Creates a weak reference to the object owned by shared_ptr
     */
    WeakPtr(const SharedPtr<T>& shared) noexcept
        : ptr_(shared.ptr_), cb_(shared.cb_) {
        if (cb_) {
            cb_->add_weak_ref();
        }
    }

    /**
     * @brief Move constructor
     */
    WeakPtr(WeakPtr&& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    // ============================================================================
    // DESTRUCTOR
    // ============================================================================

    /**
     * @brief Destructor
     * Decrements weak reference count
     */
    ~WeakPtr() {
        if (cb_) {
            if (cb_->release_weak_ref()) {
                // No more weak or shared references - delete control block
                delete cb_;
            }
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Copy assignment from WeakPtr
     */
    WeakPtr& operator=(const WeakPtr& other) noexcept {
        if (this != &other) {
            WeakPtr temp(other);
            swap(temp);
        }
        return *this;
    }

    /**
     * @brief Copy assignment from SharedPtr
     */
    WeakPtr& operator=(const SharedPtr<T>& shared) noexcept {
        WeakPtr temp(shared);
        swap(temp);
        return *this;
    }

    /**
     * @brief Move assignment
     */
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this != &other) {
            WeakPtr temp(std::move(other));
            swap(temp);
        }
        return *this;
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Reset to empty state
     */
    void reset() noexcept {
        WeakPtr temp;
        swap(temp);
    }

    /**
     * @brief Swap contents with another weak_ptr
     */
    void swap(WeakPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(cb_, other.cb_);
    }

    // ============================================================================
    // OBSERVERS
    // ============================================================================

    /**
     * @brief Get the number of shared_ptr instances managing the object
     * @return Reference count (0 if object was deleted)
     */
    long use_count() const noexcept {
        return cb_ ? cb_->use_count() : 0;
    }

    /**
     * @brief Check if the managed object was already deleted
     * @return true if the object was deleted (use_count() == 0)
     */
    bool expired() const noexcept {
        return use_count() == 0;
    }

    /**
     * @brief Create a shared_ptr that manages the observed object
     * @return shared_ptr that shares ownership of the object
     *         Empty shared_ptr if object was already deleted
     */
    SharedPtr<T> lock() const noexcept {
        if (cb_ && cb_->try_add_shared_ref()) {
            // Successfully incremented shared count
            // Create SharedPtr and manually set members to avoid double-increment
            return SharedPtr<T>(ptr_, cb_, false);  // false = don't increment again
        }
        return SharedPtr<T>();  // Object was deleted, return empty
    }
};

// Implementation of SharedPtr constructor from WeakPtr
template<typename T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& weak)
    : ptr_(nullptr), cb_(nullptr) {
    if (weak.cb_ && weak.cb_->try_add_shared_ref()) {
        ptr_ = weak.ptr_;
        cb_ = weak.cb_;
    }
    // If try_add_shared_ref fails, remain empty
}

// ============================================================================
// COMPARISON OPERATORS
// ============================================================================

template<typename T1, typename T2>
bool operator==(const SharedPtr<T1>& lhs, const SharedPtr<T2>& rhs) {
    return lhs.get() == rhs.get();
}

template<typename T1, typename T2>
bool operator!=(const SharedPtr<T1>& lhs, const SharedPtr<T2>& rhs) {
    return !(lhs == rhs);
}

template<typename T>
bool operator==(const SharedPtr<T>& ptr, std::nullptr_t) {
    return !ptr;
}

template<typename T>
bool operator==(std::nullptr_t, const SharedPtr<T>& ptr) {
    return !ptr;
}

template<typename T>
bool operator!=(const SharedPtr<T>& ptr, std::nullptr_t) {
    return static_cast<bool>(ptr);
}

template<typename T>
bool operator!=(std::nullptr_t, const SharedPtr<T>& ptr) {
    return static_cast<bool>(ptr);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Swap two shared_ptr objects
 */
template<typename T>
void swap(SharedPtr<T>& lhs, SharedPtr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * @brief Swap two weak_ptr objects
 */
template<typename T>
void swap(WeakPtr<T>& lhs, WeakPtr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * @brief Create a shared_ptr (C++11 make_shared equivalent)
 * 
 * This is the preferred way to create shared_ptr objects because:
 * 1. It's exception-safe
 * 2. It's more concise
 * 3. It's more efficient (single allocation for object + control block)
 * 4. It avoids explicit use of 'new'
 * 
 * Note: This simple implementation does two allocations (object + control block)
 * A production implementation would do a single allocation for better performance
 */
template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief Static cast for shared_ptr
 */
template<typename T, typename U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U>& ptr) {
    T* result = static_cast<T*>(ptr.get());
    SharedPtr<T> ret;
    ret.ptr_ = result;
    ret.cb_ = reinterpret_cast<ControlBlock<T, SharedPtrDeleter<T>>*>(ptr.cb_);
    if (ret.cb_) {
        ret.cb_->add_shared_ref();
    }
    return ret;
}

/**
 * @brief Dynamic cast for shared_ptr
 */
template<typename T, typename U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U>& ptr) {
    T* result = dynamic_cast<T*>(ptr.get());
    if (result) {
        SharedPtr<T> ret;
        ret.ptr_ = result;
        ret.cb_ = reinterpret_cast<ControlBlock<T, SharedPtrDeleter<T>>*>(ptr.cb_);
        if (ret.cb_) {
            ret.cb_->add_shared_ref();
        }
        return ret;
    }
    return SharedPtr<T>();  // Cast failed, return empty
}

#endif // SHARED_PTR_HPP

