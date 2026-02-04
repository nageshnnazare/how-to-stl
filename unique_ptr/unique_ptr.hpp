#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include <utility>      // for std::move, std::forward
#include <type_traits>  // for std::remove_extent, std::is_array
#include <cstddef>      // for std::nullptr_t

/**
 * @brief Custom implementation of std::unique_ptr
 * 
 * unique_ptr is a smart pointer that owns and manages a dynamically allocated object
 * through a pointer and disposes of that object when the unique_ptr goes out of scope.
 * 
 * Key characteristics:
 * - Exclusive ownership: Only one unique_ptr can own an object at a time
 * - Non-copyable: Cannot be copied to prevent multiple ownership
 * - Moveable: Ownership can be transferred via move semantics
 * - Zero-overhead: No runtime overhead compared to raw pointers (when optimized)
 */

// Default deleter for single objects
template<typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

// Specialization for array types
template<typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

// Forward declaration for the main template
template<typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr;

/**
 * @brief Main unique_ptr implementation for single objects
 * 
 * @tparam T The type of the managed object
 * @tparam Deleter The deleter type used to destroy the object (default: DefaultDeleter<T>)
 */
template<typename T, typename Deleter>
class UniquePtr {
private:
    T* ptr_;           // Raw pointer to the managed object
    Deleter deleter_;  // Deleter object (can be stateless or stateful)

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
     * @brief Construct from raw pointer
     * @param p Raw pointer to take ownership of
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
     * @brief Move constructor
     * Transfers ownership from other to this
     * After the move, other no longer owns the object
     */
    UniquePtr(UniquePtr&& other) noexcept 
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;  // Release ownership from other
    }

    /**
     * @brief Move constructor from compatible type
     * Allows conversion between compatible pointer types (e.g., Derived* to Base*)
     * Note: Only works when deleters are compatible (both default deleters)
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
     * @brief Destructor
     * Automatically destroys the managed object using the deleter
     * This is the core RAII behavior - cleanup happens automatically
     */
    ~UniquePtr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);  // Call the deleter on the managed pointer
        }
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /**
     * @brief Move assignment operator
     * Destroys the currently managed object (if any) and takes ownership from other
     */
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());  // Release old, take ownership of new
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
     * @brief Release ownership of the managed object
     * @return Raw pointer to the previously managed object
     * 
     * After calling release(), the unique_ptr no longer owns the object
     * The caller is responsible for deleting the returned pointer
     */
    T* release() noexcept {
        T* old_ptr = ptr_;
        ptr_ = nullptr;  // Give up ownership without deleting
        return old_ptr;
    }

    /**
     * @brief Replace the managed object
     * @param p New pointer to manage (default: nullptr)
     * 
     * Destroys the currently managed object (if any) and takes ownership of p
     */
    void reset(T* p = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = p;
        if (old_ptr != nullptr) {
            deleter_(old_ptr);  // Delete the old object
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

/**
 * @brief Specialization for array types (T[])
 * 
 * This specialization:
 * - Uses delete[] instead of delete
 * - Provides operator[] instead of operator* and operator->
 * - Disables conversions between different pointer types
 */
template<typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    T* ptr_;
    Deleter deleter_;

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
     * @brief Array subscript operator
     * Allows indexing into the managed array
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
 * @brief Create a unique_ptr (C++14 make_unique equivalent)
 * 
 * This is the preferred way to create unique_ptr objects because:
 * 1. It's exception-safe
 * 2. It's more concise
 * 3. It avoids explicit use of 'new'
 */
template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief Create a unique_ptr for arrays with size
 */
template<typename T>
UniquePtr<T[]> makeUniqueArray(std::size_t size) {
    return UniquePtr<T[]>(new T[size]());
}

#endif // UNIQUE_PTR_HPP

