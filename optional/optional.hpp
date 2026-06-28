#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#include <stdexcept>    // for std::runtime_error
#include <utility>      // for std::move

// ============================================================================
//  Optional<T> -- a hand-rolled std::optional (maybe-value container)
// ============================================================================
//
// WHAT IT IS
// ----------
// An Optional may or may not contain a value of type T.  It replaces the
// pattern of using nullptr, -1, or magic sentinels to mean "no result" with
// an explicit, type-safe empty state.  The object always occupies sizeof(T)
// bytes of storage plus a bool flag — whether T is actually *alive* inside
// that storage depends on has_value_.
//
// THE TWO FIELDS
// --------------
//     storage_   -> raw, aligned byte buffer large enough to hold a T
//     has_value_ -> true iff a T object has been constructed in storage_
//
// STATE DIAGRAM — ENGAGED vs DISENGAGED
// -------------------------------------
//
//   DISENGAGED (default / after reset)          ENGAGED (holds a live T)
//   ┌─────────────────────────────┐            ┌─────────────────────────────┐
//   │ storage_ : raw bytes        │            │ storage_ : live T object    │
//   │   (no T constructed)        │            │   (placement-new'd)         │
//   │   [ ?? ?? ?? ... ]          │            │   [ valid T vtable/data ]   │
//   ├─────────────────────────────┤            ├─────────────────────────────┤
//   │ has_value_ = false          │            │ has_value_ = true           │
//   └─────────────────────────────┘            └─────────────────────────────┘
//   *ptr() is UB to dereference                *ptr() returns valid T&
//
// WHY RAW STORAGE INSTEAD OF `T value_` MEMBER?
// ---------------------------------------------
// If we wrote `T member_; bool has_value_;`, the compiler would always run
// T's constructor in Optional's constructor — even when we want "empty".
// Types without default constructors could not form an empty Optional at all.
//
// Instead we keep UNINITIALIZED bytes and construct T only when needed:
//
//     DISENGAGED:  no constructor runs; storage_ is just char[N]
//     ENGAGED:     new (storage_) T(args)   ← placement new
//     RESET:       ptr()->~T()              ← explicit destructor
//     ~Optional:   if engaged, ~T() then discard bytes
//
//     alignas(T) ensures storage_ meets T's alignment requirement so
//     placement new produces a correctly aligned T*.
//
// LIFETIME RULES (the whole trick)
// --------------------------------
//     1. Allocate raw:     alignas(T) unsigned char storage_[sizeof(T)]
//     2. Engage:           new (storage_) T(...)     only when has_value_=true
//     3. Access:           reinterpret_cast<T*>(storage_)  only if engaged
//     4. Disengage:        ptr()->~T(); has_value_=false
//
// Key characteristics:
// - Stack storage only (no heap for the optional itself)
// - Empty and value states are distinguishable even when T is default-constructible
// - value() throws on empty; operator* does not (caller must check)
// - Copy/move duplicate or steal the inner T only when source is engaged
// ============================================================================

/**
 * @brief Type-safe container that may or may not hold a value of type T.
 *
 * Models the C++17 std::optional interface (subset): explicit empty state,
 * placement-new construction, and manual destruction on reset.
 */
template<typename T>
class Optional {
private:
    alignas(T) unsigned char storage_[sizeof(T)];  ///< Raw, aligned buffer; holds a T only when engaged
    bool has_value_;                                ///< true iff storage_ contains a live T object

    /**
     * @brief Reinterpret storage as T* (caller must ensure has_value_).
     *
     * No allocation — storage_ already exists inline in the Optional object.
     */
    T* ptr() { return reinterpret_cast<T*>(storage_); }
    const T* ptr() const { return reinterpret_cast<const T*>(storage_); }

public:
    /**
     * @brief Create an empty optional (disengaged).
     *
     *     Optional<int> o;
     *     // storage_: uninitialized bytes
     *     // has_value_: false
     *     // NO int constructor runs
     */
    Optional() : has_value_(false) {}

    /**
     * @brief Engage by copy-constructing T inside storage_.
     *
     * Steps:
     *   1. has_value_ = true
     *   2. placement new (storage_) T(value)  → live T in raw buffer
     *
     *     raw storage_          after placement new
     *     [ ? ? ? ? ]    →      [ copy of value ]
     */
    Optional(const T& value) : has_value_(true) {
        new (storage_) T(value);
    }

    /**
     * @brief Engage by move-constructing T inside storage_.
     *
     * Steals resources from the source temporary (e.g. string buffer)
     * instead of copying.
     */
    Optional(T&& value) : has_value_(true) {
        new (storage_) T(std::move(value));
    }

    /**
     * @brief Destroy engaged value if present.
     *
     *     if has_value_:  run ~T() on storage_
     *     (raw bytes remain but are no longer a live object)
     */
    ~Optional() {
        if (has_value_) ptr()->~T();
    }

    /**
     * @brief Copy: duplicate engaged state; placement-new copy if source has value.
     *
     *     other empty  → this stays empty
     *     other full   → new (storage_) T(*other.ptr())
     */
    Optional(const Optional& other) : has_value_(other.has_value_) {
        if (has_value_) new (storage_) T(*other.ptr());
    }

    /**
     * @brief Move: steal other's value; leave other disengaged.
     *
     *     if other engaged:
     *         new (storage_) T(move(*other.ptr()))
     *         other.reset()   ← other becomes empty
     */
    Optional(Optional&& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (storage_) T(std::move(*other.ptr()));
            other.reset();
        }
    }

    /**
     * @brief Copy-assign: destroy current value, then copy other's state.
     *
     *     if engaged: ~T() first
     *     copy has_value_ flag
     *     if new flag true: placement-new copy from other
     */
    Optional& operator=(const Optional& other) {
        if (this != &other) {
            if (has_value_) ptr()->~T();
            has_value_ = other.has_value_;
            if (has_value_) new (storage_) T(*other.ptr());
        }
        return *this;
    }

    /** @brief Explicit check for engaged state. */
    bool has_value() const { return has_value_; }

    /** @brief Contextual conversion: true iff engaged (same as has_value()). */
    explicit operator bool() const { return has_value_; }

    /**
     * @brief Return reference to value; throw if disengaged.
     *
     * WHY throw?  Signals programmer error ("I expected a value") vs
     * value_or() which is for intentional fallbacks.
     */
    T& value() {
        if (!has_value_) throw std::runtime_error("Optional::value: no value");
        return *ptr();
    }

    const T& value() const {
        if (!has_value_) throw std::runtime_error("Optional::value: no value");
        return *ptr();
    }

    /**
     * @brief Return stored value or a copy of default_value if empty.
     *
     * Never throws; does not modify the optional.
     *
     *     Optional<int> o;  o.value_or(99)  →  99
     */
    T value_or(const T& default_value) const {
        return has_value_ ? *ptr() : default_value;
    }

    /**
     * @brief Dereference without check — UB if disengaged.
     *
     * Mirrors pointer syntax; caller should guard with if (opt) or has_value().
     */
    T& operator*() { return *ptr(); }
    const T& operator*() const { return *ptr(); }
    T* operator->() { return ptr(); }
    const T* operator->() const { return ptr(); }

    /**
     * @brief Disengage: destroy live T and clear flag.
     *
     *     ENGAGED ──reset()──▶ DISENGAGED
     *
     *     if has_value_:
     *         ptr()->~T()      explicit destructor call
     *         has_value_ = false
     *
     * WHY explicit ~T()?  The compiler will not destroy a non-existent object;
     * we must tell it exactly when the object in storage_ ends its lifetime.
     */
    void reset() {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }
};

#endif // OPTIONAL_HPP
