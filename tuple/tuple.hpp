#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <cstddef>      // for size_t
#include <utility>      // for std::move, std::forward
#include <type_traits>  // for std::integral_constant (compile-time get<I> dispatch)

// ============================================================================
//  Tuple<Types...> -- a hand-rolled std::tuple (2- and 3-element specializations)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Tuple is a fixed-size, heterogeneous product type: each element has its
// own compile-time index and its own type.  This header implements arity 2
// and 3 via explicit template specializations (not the full recursive
// std::tuple machinery), but the *access pattern* mirrors the real library:
// get<I>(t) returns element I at compile time with zero runtime cost.
//
// HOW std::tuple DOES IT (full library — for comparison)
// ------------------------------------------------------
// The standard library nests tuples recursively:
//
//     Tuple<Head, Tail...>  inherits from  Tuple<Tail...>
//         │                              │
//         └── stores Head head_          └── stores the rest
//
//     Tuple<T1, T2, T3>  looks like:
//
//         Tuple<T1,T2,T3>
//         ├── T1 first_          (this specialization's member)
//         └── Tuple<T2,T3>       (base / nested tail)
//                 ├── T2 first_
//                 └── Tuple<T3>
//                         └── T3 first_
//
//     get<0>: return the head of the outermost Tuple
//     get<1>: cast to Tuple<T2,T3>&  → get<0> on the tail  (recurse once)
//     get<2>: cast to Tuple<T3>&     → get<0> twice        (recurse twice)
//
// THIS IMPLEMENTATION (what is actually in this file)
// ---------------------------------------------------
// We flatten the layout: each specialization stores named members directly.
// get<I>() dispatches with tag dispatch (compile-time overload, no recursion):
//
//     Tuple<int, double, string>     Tuple<string, int>
//     ┌──────┬────────┬─────────┐    ┌─────────┬──────┐
//     │ int  │ double │ string  │    │ string  │ int  │
//     │first_│second_ │ third_  │    │ first_  │second│
//     └──────┴────────┴─────────┘    └─────────┴──────┘
//       I=0    I=1      I=2            I=0       I=1
//
//     get<1>(t)  →  forwards to the helper overload tagged with index 1,
//                   which returns second_ (selected at compile time; no runtime if)
//
// Key characteristics:
// - Stack-only storage; no dynamic allocation
// - Index access is O(1) and inlined to a direct member reference
// - Type-safe: get<2> on a 2-tuple is a compile error
// - Equality is element-wise AND across all members
// ============================================================================

// Primary template — only 2- and 3-type specializations below are defined.
template<typename... Types>
class Tuple;

// ============================================================================
// 2-element specialization
// ============================================================================

/**
 * @brief Two-element heterogeneous tuple.
 *
 * Layout:
 *
 *     Tuple<T1, T2> object
 *     ┌─────────────┬─────────────┐
 *     │  T1 first_  │ T2 second_  │
 *     └─────────────┴─────────────┘
 *       get<0>()      get<1>()
 */
template<typename T1, typename T2>
class Tuple<T1, T2> {
private:
    T1 first_;   ///< Element at compile-time index 0
    T2 second_;  ///< Element at compile-time index 1

public:
    /** @brief Value-initialize both elements. */
    Tuple() : first_(), second_() {}

    /** @brief Copy-construct each element from lvalues. */
    Tuple(const T1& t1, const T2& t2) : first_(t1), second_(t2) {}

    /** @brief Move-construct each element from rvalues. */
    Tuple(T1&& t1, T2&& t2) : first_(std::move(t1)), second_(std::move(t2)) {}

    /**
     * @brief Member access by compile-time index I.
     *
     * Dispatch (resolved entirely at compile time via tag dispatch):
     *
     *     get<I>()
     *         │
     *         ├── I == 0 ──▶ return first_
     *         └── I == 1 ──▶ return second_
     *
     * WHY tag dispatch (std::integral_constant) instead of a runtime if?
     * Each element has a different type, so one function body cannot `return`
     * both. We forward to an overloaded helper selected by the compile-time
     * index tag, so only the matching overload (and its return type) is used.
     * Invalid I (e.g. get<2> on a 2-tuple) fails to compile — no such overload.
     */
    template<size_t I>
    auto& get() { return get_impl(std::integral_constant<size_t, I>{}); }

    template<size_t I>
    const auto& get() const { return get_impl(std::integral_constant<size_t, I>{}); }

private:
    T1& get_impl(std::integral_constant<size_t, 0>) { return first_; }
    T2& get_impl(std::integral_constant<size_t, 1>) { return second_; }
    const T1& get_impl(std::integral_constant<size_t, 0>) const { return first_; }
    const T2& get_impl(std::integral_constant<size_t, 1>) const { return second_; }

public:
    /**
     * @brief Element-wise equality: all indices must match.
     *
     *     t == u  iff  get<0>(t)==get<0>(u) && get<1>(t)==get<1>(u)
     */
    bool operator==(const Tuple& other) const {
        return first_ == other.first_ && second_ == other.second_;
    }
};

// ============================================================================
// 3-element specialization
// ============================================================================

/**
 * @brief Three-element heterogeneous tuple.
 *
 * Layout:
 *
 *     Tuple<T1, T2, T3> object
 *     ┌──────────┬───────────┬──────────┐
 *     │ T1 first_│ T2 second_│ T3 third_│
 *     └──────────┴───────────┴──────────┘
 *       get<0>()   get<1>()    get<2>()
 *
 * In a full recursive std::tuple, get<2> would recurse into the tail twice;
 * here we map I==2 directly to third_ — same observable behavior, simpler code.
 */
template<typename T1, typename T2, typename T3>
class Tuple<T1, T2, T3> {
private:
    T1 first_;   ///< Element at compile-time index 0
    T2 second_;  ///< Element at compile-time index 1
    T3 third_;   ///< Element at compile-time index 2

public:
    Tuple() : first_(), second_(), third_() {}

    Tuple(const T1& t1, const T2& t2, const T3& t3)
        : first_(t1), second_(t2), third_(t3) {}

    Tuple(T1&& t1, T2&& t2, T3&& t3)
        : first_(std::move(t1)), second_(std::move(t2)), third_(std::move(t3)) {}

    /**
     * @brief Index dispatch for 3-tuple:
     *
     *     I == 0 → first_
     *     I == 1 → second_
     *     I == 2 → third_
     */
    template<size_t I>
    auto& get() { return get_impl(std::integral_constant<size_t, I>{}); }

    template<size_t I>
    const auto& get() const { return get_impl(std::integral_constant<size_t, I>{}); }

private:
    T1& get_impl(std::integral_constant<size_t, 0>) { return first_; }
    T2& get_impl(std::integral_constant<size_t, 1>) { return second_; }
    T3& get_impl(std::integral_constant<size_t, 2>) { return third_; }
    const T1& get_impl(std::integral_constant<size_t, 0>) const { return first_; }
    const T2& get_impl(std::integral_constant<size_t, 1>) const { return second_; }
    const T3& get_impl(std::integral_constant<size_t, 2>) const { return third_; }

public:
    bool operator==(const Tuple& other) const {
        return first_ == other.first_ &&
               second_ == other.second_ &&
               third_ == other.third_;
    }
};

/**
 * @brief Free-function get — mirrors std::get<I>(tuple).
 *
 * Forwards to the member get<I>() so call sites match standard syntax:
 *
 *     Tuple<int, double> t(1, 3.14);
 *     int& x = get<0>(t);   // same as t.get<0>()
 */
template<size_t I, typename... Types>
auto& get(Tuple<Types...>& t) {
    return t.template get<I>();
}

template<size_t I, typename... Types>
const auto& get(const Tuple<Types...>& t) {
    return t.template get<I>();
}

/**
 * @brief Construct a 2-tuple with deduced element types (perfect forwarding).
 */
template<typename T1, typename T2>
Tuple<T1, T2> make_tuple(T1&& t1, T2&& t2) {
    return Tuple<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

/**
 * @brief Construct a 3-tuple with deduced element types (perfect forwarding).
 */
template<typename T1, typename T2, typename T3>
Tuple<T1, T2, T3> make_tuple(T1&& t1, T2&& t2, T3&& t3) {
    return Tuple<T1, T2, T3>(std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3));
}

#endif // TUPLE_HPP
