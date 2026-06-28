#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include <utility>      // for std::move, std::forward
#include <type_traits>  // for std::remove_extent, std::is_array
#include <cstddef>      // for std::nullptr_t

// ============================================================================
//  UniquePtr<T, Deleter> -- a hand-rolled std::unique_ptr (exclusive ownership)
// ============================================================================
//
// WHAT IT IS
// ----------
// A UniquePtr is a move-only RAII handle for exactly one heap object. When the
// handle is destroyed, reset, or replaced, the stored deleter runs on ptr_. Copies
// are deleted at compile time so two handles can never delete the same address.
//
// THE TWO FIELDS (primary template)
// ---------------------------------
//     ptr_      -> raw pointer to the managed object (nullptr when empty)
//     deleter_  -> callable invoked as deleter_(ptr_) in ~UniquePtr and reset()
//
// MEMORY LAYOUT (stateless deleter, 64-bit)
// -----------------------------------------
//
//     UniquePtr object (stack)            Heap object
//     ┌───────────────────┐               ┌─────────────────┐
//     │ ptr_       ●──────┼──────────────▶│   T instance    │
//     │ deleter_   (0 B*) │               │   members...    │
//     └───────────────────┘               └─────────────────┘
//     * Empty Base Optimization: stateless DefaultDeleter adds no size
//
// EXCLUSIVE OWNERSHIP (the key idea)
// ----------------------------------
//
//     Before move:   ptr1 ──> [Object]     ptr2 ──> nullptr
//     After move:    ptr1 ──> nullptr       ptr2 ──> [Object]
//
//     Only the non-null owner may delete. Copying would create two owners → UB.
//
// ARRAY SPECIALIZATION UniquePtr<T[]>
// -----------------------------------
// Same two fields; uses delete[] via DefaultDeleter<T[]>; exposes operator[]
// instead of operator* / operator->; no Derived[] → Base[] conversions.
// ============================================================================

// Callable deleter for single objects — selected automatically when T is not T[].
template<typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;  // single-object deletion
    }
};

// Array deleter — delete[] must be used for memory allocated with new T[n].
template<typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

// Forward declaration for the main template
template<typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr;

template<typename T, typename Deleter>
class UniquePtr {
private:
    T* ptr_;           // sole owning raw pointer; nullptr after move-from or release()
    Deleter deleter_;  // destruction policy; stateless deleters cost 0 bytes (EBO)

public:
    // Type aliases
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /**
     * @brief Default constructor - creates an empty unique_ptr
     */
    constexpr UniquePtr() noexcept : ptr_(nullptr), deleter_() {}

    /**
     * @brief Nullptr constructor - creates an empty unique_ptr
     */
    constexpr UniquePtr(std::nullptr_t) noexcept : ptr_(nullptr), deleter_() {}

    /**
     * @brief Take ownership of a raw pointer (explicit — no implicit conversion).
     *
     *     caller's heap          UniquePtr
     *     [ new T ]  ──give──>   ptr_ = p
     *
     * WHY explicit: prevents func(UniquePtr<T>(implicit from new T*)) accidents.
     */
    explicit UniquePtr(T* p) noexcept : ptr_(p), deleter_() {}

    /**
     * @brief Construct from raw pointer with custom deleter
     * @param p Raw pointer to take ownership of
     * @param d Deleter object (copied or moved)
     */
    UniquePtr(T* p, const Deleter& d) noexcept : ptr_(p), deleter_(d) {}
    
    UniquePtr(T* p, Deleter&& d) noexcept : ptr_(p), deleter_(std::move(d)) {}

    /**
     * @brief Move constructor — transfer exclusive ownership.
     *
     *     BEFORE                    AFTER
     *     other ──> [Obj]           other ──> nullptr
     *     this  ──> nullptr         this  ──> [Obj]
     *
     * Steps: (1) steal other.ptr_  (2) move deleter_  (3) other.ptr_ = nullptr
     * WHY null the source: otherwise both would call deleter_ on destruction.
     */
    UniquePtr(UniquePtr&& other) noexcept 
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    /**
     * @brief Converting move — UniquePtr<U> → UniquePtr<T> when U* converts to T*.
     *
     *     UniquePtr<Derived> ──release()──> Derived* ──store──> UniquePtr<Base>
     *
     * Uses release() so other is empty before this owns the pointer.
     * Deleter resets to default (both must use compatible default deleters).
     */
    template<typename U, typename E,
             typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    UniquePtr(UniquePtr<U, E>&& other) noexcept 
        : ptr_(other.release()), deleter_() {
    }

    // ============================================================================
    // DELETED COPY OPERATIONS
    // unique_ptr cannot be copied (exclusive ownership semantics)
    // ============================================================================

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // ============================================================================
    // DESTRUCTOR
    // ============================================================================

    /**
     * @brief RAII destructor — run deleter if we still own an object.
     *
     *     ~UniquePtr()  →  if (ptr_) deleter_(ptr_);
     *
     * Moved-from objects have ptr_ == nullptr → no-op.
     * WHY check nullptr: moved-from and default-constructed handles must not delete.
     */
    ~UniquePtr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Move assignment — destroy current object, adopt other's pointer.
     *
     *     this had [Old], other had [New]:
     *     reset(release(other))  →  deleter_([Old]), ptr_=[New], other=nullptr
     *
     * reset(release()) ordering: release() nulls other before reset deletes Old.
     */
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            deleter_ = std::move(other.deleter_);
        }
        return *this;
    }

    /**
     * @brief Move assignment from compatible type
     * Note: Only works when deleters are compatible (both default deleters)
     */
    template<typename U, typename E,
             typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        reset(other.release());
        return *this;
    }

    /**
     * @brief Assignment from nullptr
     * Destroys the currently managed object and resets to empty state
     */
    UniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Relinquish ownership without calling the deleter.
     *
     *     Owning ──release()──> ptr_=nullptr, return raw
     *                                    │
     *                                    v
     *                          caller must delete manually
     *
     * WHY: bridge to legacy APIs that take T* and expect caller-owned memory.
     */
    T* release() noexcept {
        T* old_ptr = ptr_;
        ptr_ = nullptr;
        return old_ptr;
    }

    /**
     * @brief Replace managed object; delete previous via deleter_.
     *
     *     (1) old = ptr_     (2) ptr_ = p     (3) if (old) deleter_(old)
     *
     * WHY assign before delete: safe for ptr.reset(ptr.get()) — old == p, one live obj.
     */
    void reset(T* p = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = p;
        if (old_ptr != nullptr) {
            deleter_(old_ptr);
        }
    }

    /**
     * @brief Swap contents with another unique_ptr
     */
    void swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    // ============================================================================
    // OBSERVERS
    // ============================================================================

    /**
     * @brief Get the raw pointer without releasing ownership
     * @return Raw pointer to the managed object (or nullptr)
     */
    T* get() const noexcept {
        return ptr_;
    }

    /**
     * @brief Get reference to the deleter
     */
    Deleter& get_deleter() noexcept {
        return deleter_;
    }

    const Deleter& get_deleter() const noexcept {
        return deleter_;
    }

    /**
     * @brief Check if the unique_ptr owns an object
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

    // Friend declaration for cross-type access
    template<typename U, typename E>
    friend class UniquePtr;
};

// ============================================================================
//  UniquePtr<T[], Deleter> -- array specialization (delete[], operator[])
// ============================================================================
//
// MEMORY LAYOUT
// -------------
//     UniquePtr<T[]> (stack)              Heap array
//     ┌───────────────────┐               ┌───┬───┬───┬───┐
//     │ ptr_       ●──────┼──────────────▶│[0]│[1]│[2]│...│
//     │ deleter_          │               └───┴───┴───┴───┘
//     └───────────────────┘               arr[i] → *(ptr_ + i)
//
// No operator* / operator-> (an array is not a single object).
// No U[] → T[] converting constructor (array covariance is UB with wrong delete[]).
// ============================================================================
template<typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    T* ptr_;           // pointer to first element of owned array
    Deleter deleter_;  // typically DefaultDeleter<T[]> → delete[]

public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // Constructors
    constexpr UniquePtr() noexcept : ptr_(nullptr), deleter_() {}
    constexpr UniquePtr(std::nullptr_t) noexcept : ptr_(nullptr), deleter_() {}
    
    explicit UniquePtr(T* p) noexcept : ptr_(p), deleter_() {}
    
    UniquePtr(T* p, const Deleter& d) noexcept : ptr_(p), deleter_(d) {}
    UniquePtr(T* p, Deleter&& d) noexcept : ptr_(p), deleter_(std::move(d)) {}

    // Move constructor
    UniquePtr(UniquePtr&& other) noexcept 
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    // Deleted copy operations
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // Destructor
    ~UniquePtr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);  // Will call delete[] for arrays
        }
    }

    // Move assignment
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            deleter_ = std::move(other.deleter_);
        }
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // Modifiers
    T* release() noexcept {
        T* old_ptr = ptr_;
        ptr_ = nullptr;
        return old_ptr;
    }

    void reset(T* p = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = p;
        if (old_ptr != nullptr) {
            deleter_(old_ptr);
        }
    }

    void swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    // Observers
    T* get() const noexcept {
        return ptr_;
    }

    Deleter& get_deleter() noexcept {
        return deleter_;
    }

    const Deleter& get_deleter() const noexcept {
        return deleter_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    /**
     * @brief Index into the managed array (no bounds check).
     *
     *     arr[i]  →  *(ptr_ + i)
     *
     * UB if empty or index out of range — same as raw T*.
     */
    T& operator[](std::size_t index) const noexcept {
        return ptr_[index];
    }
};

// ============================================================================
// COMPARISON OPERATORS
// ============================================================================

template<typename T1, typename D1, typename T2, typename D2>
bool operator==(const UniquePtr<T1, D1>& lhs, const UniquePtr<T2, D2>& rhs) {
    return lhs.get() == rhs.get();
}

template<typename T1, typename D1, typename T2, typename D2>
bool operator!=(const UniquePtr<T1, D1>& lhs, const UniquePtr<T2, D2>& rhs) {
    return !(lhs == rhs);
}

template<typename T, typename D>
bool operator==(const UniquePtr<T, D>& ptr, std::nullptr_t) {
    return !ptr;
}

template<typename T, typename D>
bool operator==(std::nullptr_t, const UniquePtr<T, D>& ptr) {
    return !ptr;
}

template<typename T, typename D>
bool operator!=(const UniquePtr<T, D>& ptr, std::nullptr_t) {
    return static_cast<bool>(ptr);
}

template<typename T, typename D>
bool operator!=(std::nullptr_t, const UniquePtr<T, D>& ptr) {
    return static_cast<bool>(ptr);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Swap two unique_ptr objects
 */
template<typename T, typename D>
void swap(UniquePtr<T, D>& lhs, UniquePtr<T, D>& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * @brief Factory — allocate T and wrap immediately (preferred over UniquePtr(new T)).
 *
 * EXCEPTION SAFETY (why this beats UniquePtr<T>(new T) in function args):
 *
 *     UNSAFE:  f(UniquePtr<A>(new A()), UniquePtr<B>(new B()))
 *              evaluation order unspecified → if 2nd new throws, 1st may leak
 *
 *     SAFE:    f(makeUnique<A>(), makeUnique<B>())
 *              each new is wrapped before the next argument is evaluated
 *
 * Perfect-forwards args so move-only types and emplace-style construction work.
 */
template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief Factory for value-initialized arrays: new T[n]() zero/init each element.
 *
 *     makeUniqueArray<int>(5)  →  UniquePtr<int[]> owning [0,0,0,0,0]
 */
template<typename T>
UniquePtr<T[]> makeUniqueArray(std::size_t size) {
    return UniquePtr<T[]>(new T[size]());
}

#endif // UNIQUE_PTR_HPP

