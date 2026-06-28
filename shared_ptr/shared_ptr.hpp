#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include <utility>      // for std::move, std::forward
#include <type_traits>  // for std::remove_extent, std::is_array
#include <cstddef>      // for std::nullptr_t
#include <atomic>       // for std::atomic (thread-safe reference counting)

// ============================================================================
//  SharedPtr<T> / WeakPtr<T> -- hand-rolled std::shared_ptr / std::weak_ptr
// ============================================================================
//
// WHAT IT IS
// ----------
// SharedPtr shares ownership of one heap object across many handles. A separate
// ControlBlock on the heap holds atomic strong/weak counts and the deleter.
// When the last SharedPtr dies, the object is deleted; when both counts hit 0,
// the control block is deleted. WeakPtr observes without keeping the object alive.
//
// SHARED_PTR FIELDS
// -----------------
//     ptr_  -> cached address of managed object (fast operator->)
//     cb_   -> pointer to shared ControlBlock (counts + deleter + ptr_)
//
// MEMORY LAYOUT (two SharedPtrs, one object)
// ------------------------------------------
//
//   SharedPtr A              ControlBlock (heap)           Object (heap)
//   ┌──────────────┐        ┌────────────────────┐       ┌──────────┐
//   │ ptr_    ●────┼───────>│ ptr_               │──────>│    T     │
//   │ cb_     ●────┼───┐    │ shared_count_ = 2  │       └──────────┘
//   └──────────────┘   │    │ weak_count_   = 0  │
//                      │    │ deleter_           │
//   SharedPtr B        │    └────────────────────┘
//   ┌──────────────┐   │
//   │ ptr_    ●────┼───┘  (same cb_)
//   │ cb_     ●────┼──────> (same block)
//   └──────────────┘
//
// STRONG COUNT LIFECYCLE
// ----------------------
//   copy SharedPtr  → shared_count++
//   ~SharedPtr      → shared_count--; if was 1 → deleter_(object)
//   weak_count==0 after that → delete control block
//
// Key characteristics:
// - Copyable shared ownership; move transfers one owner's seat
// - Atomic ref counts (thread-safe counting, NOT thread-safe *object* access)
// - WeakPtr breaks cycles: observes via cb_, does not bump shared_count_
// ============================================================================

// Forward declarations
template<typename T> class SharedPtr;
template<typename T> class WeakPtr;

// ============================================================================
//  ControlBlock<T, Deleter> -- reference counts + deletion policy
// ============================================================================
//
//     ┌─────────────────────────────────────┐
//     │ ptr_          → managed object      │
//     │ shared_count_ → # of SharedPtr      │
//     │ weak_count_   → # of WeakPtr        │
//     │ deleter_      → called at strong==0 │
//     └─────────────────────────────────────┘
//
// Object deleted when shared_count hits 0. Control block deleted when
// shared_count==0 AND weak_count==0 (WeakPtr may outlive the object).
// ============================================================================
template<typename T, typename Deleter>
class ControlBlock {
private:
    T* ptr_;                         // object address; set nullptr after deleter runs
    std::atomic<long> shared_count_; // strong owners (SharedPtr instances)
    std::atomic<long> weak_count_;   // weak observers (WeakPtr instances)
    Deleter deleter_;                // invoked on ptr_ when last strong ref released

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
     * @brief Strong ref ++ (SharedPtr copy / lock success).
     *
     *     shared_count_:  n  →  n+1   (relaxed — count alone needs no ordering)
     */
    void add_shared_ref() {
        shared_count_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Strong ref -- ; destroy object if this was the last owner.
     *
     *     fetch_sub returns previous value; if it was 1, we were the last SharedPtr:
     *
     *         shared: 1 → 0  →  deleter_(ptr_); ptr_=nullptr
     *         if weak_count==0 → return true (caller deletes ControlBlock)
     *
     * acq_rel on decrement pairs with lock()'s acquire for visibility of writes.
     */
    bool release_shared_ref() {
        if (shared_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (ptr_) {
                deleter_(ptr_);
                ptr_ = nullptr;
            }
            
            if (weak_count_.load(std::memory_order_acquire) == 0) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Weak ref ++ (WeakPtr copy / construction from SharedPtr).
     */
    void add_weak_ref() {
        weak_count_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Weak ref -- ; free control block if no shared refs remain.
     *
     *     weak: 1 → 0  AND  shared==0  →  return true (delete cb)
     */
    bool release_weak_ref() {
        if (weak_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (shared_count_.load(std::memory_order_acquire) == 0) {
                return true;
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
     * @brief weak_ptr::lock() — try to resurrect a strong ref (CAS loop).
     *
     *     load count; while count>0: CAS count→count+1
     *     success → object still alive; fail → retry with updated count
     *     count==0 → object already destroyed → return false
     *
     * WHY CAS: between expired() and increment, another thread may drop last SharedPtr.
     */
    bool try_add_shared_ref() {
        long count = shared_count_.load(std::memory_order_acquire);
        while (count > 0) {
            if (shared_count_.compare_exchange_weak(count, count + 1,
                                                     std::memory_order_acq_rel,
                                                     std::memory_order_acquire)) {
                return true;
            }
        }
        return false;
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

template<typename T>
class SharedPtr {
private:
    T* ptr_;                                    // denormalized object address for fast ->
    ControlBlock<T, SharedPtrDeleter<T>>* cb_;  // shared metadata; nullptr when empty

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
     * @brief Construct from raw pointer — allocate control block, shared_count=1.
     *
     *     new T  ──>  new ControlBlock(p, deleter)  ──>  ptr_, cb_ set
     *
     * If ControlBlock allocation throws → delete p (strong exception safety).
     */
    explicit SharedPtr(T* p) : ptr_(p), cb_(nullptr) {
        if (p) {
            try {
                cb_ = new ControlBlock<T, SharedPtrDeleter<T>>(p, SharedPtrDeleter<T>());
            } catch (...) {
                delete p;
                throw;
            }
        }
    }

    /**
     * @brief Copy — share cb_, increment strong count.
     *
     *     sp1 ──┐
     *           ├──> cb (shared++) ──> Object
     *     sp2 ──┘
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
     * @brief Move — steal ptr_/cb_; total shared_count unchanged.
     *
     *     other becomes empty; one owner's handle slot moves to this.
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
     * @brief Destructor — release strong ref; maybe delete object + control block.
     *
     *     ~SharedPtr  →  release_shared_ref()
     *         returns true  →  delete cb_  (no weak refs left)
     *         returns false →  object and/or cb kept for WeakPtr
     */
    ~SharedPtr() {
        if (cb_) {
            if (cb_->release_shared_ref()) {
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
     * @brief Rebind to p via copy-and-swap (strong guarantee).
     *
     *     SharedPtr temp(p);  swap(temp);  ~temp releases old ownership
     *
     * No-op if p == ptr_ (same raw address).
     */
    void reset(T* p = nullptr) {
        if (p == ptr_) {
            return;
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

// ============================================================================
//  WeakPtr<T> -- non-owning observer; breaks SharedPtr cycles
// ============================================================================
//
//     SharedPtr ──strong──> Object
//     WeakPtr   ──weak────> ControlBlock only (weak_count++, NOT shared_count++)
//
// Cyclic leak without WeakPtr:
//     A ──strong──> B ──strong──> A  → counts never hit 0
//
// Fix: one edge WeakPtr → object can die, weak refs decay, cb freed later.
// ============================================================================
template<typename T>
class WeakPtr {
private:
    T* ptr_;                                    // cached address (may dangle if expired)
    ControlBlock<T, SharedPtrDeleter<T>>* cb_;  // keeps control block metadata alive

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
     * @brief Observe SharedPtr without keeping object alive.
     *
     *     shared_count unchanged; weak_count++
     *     Object still deleted when last SharedPtr dies.
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
     * @brief Promote to SharedPtr if object still alive (atomic CAS in try_add_shared_ref).
     *
     *     try_add_shared_ref ok  →  SharedPtr(ptr_, cb_, add_ref=false)
     *     failed / no cb_        →  empty SharedPtr
     *
     * add_ref=false: CAS already incremented; avoid double bump.
     */
    SharedPtr<T> lock() const noexcept {
        if (cb_ && cb_->try_add_shared_ref()) {
            return SharedPtr<T>(ptr_, cb_, false);
        }
        return SharedPtr<T>();
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
 * @brief Factory — wrap new T in SharedPtr (two allocations in this teaching impl).
 *
 *     new T  →  SharedPtr ctor  →  new ControlBlock
 *
 * Production std::make_shared fuses object+control block into one allocation.
 * Here we keep them separate so both steps are visible when learning.
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

