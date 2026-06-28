#ifndef PAIR_HPP
#define PAIR_HPP

#include <utility>      // for std::move, std::forward

// ============================================================================
//  Pair<T1, T2> -- a hand-rolled std::pair (two-value heterogeneous aggregate)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Pair is the smallest heterogeneous container: exactly two values of
// (possibly different) types, exposed as public members `first` and `second`.
// It is the glue behind associative containers — every Map entry is a
// Pair<Key, Value>, and Set stores Pair<Key, Key> internally (key compared
// twice for ordering).  Unlike Tuple, Pair is always arity-2 and uses named
// fields instead of index-based access.
//
// THE TWO FIELDS
// --------------
//     first   -> the "primary" element (key in map/set, x in a coordinate)
//     second  -> the "secondary" element (mapped value, y in a coordinate)
//
// MEMORY LAYOUT  Pair<int, std::string>  (conceptual; padding may appear)
// -------------------------------------------------------------------------
//
//     Pair object (stack, contiguous aggregate)
//     ┌────────────────────────────┬────────────────────────────┐
//     │ first  : int               │ second : std::string       │
//     │ (typically 4 bytes)        │ (typically 24–32 bytes)    │
//     └────────────────────────────┴────────────────────────────┘
//      offset 0                      offset alignof(string)
//
//     The compiler lays out `first` then `second` in declaration order.
//     If alignof(T2) > alignof(T1), padding bytes may sit between them so
//     `second` starts at a properly aligned address.  Total sizeof(Pair) is
//     NOT always sizeof(T1) + sizeof(T2).
//
//     Pair<int, char> on a typical 64-bit platform (no padding between):
//     ┌──────────┬─────┬─────── (possible tail padding) ───────┐
//     │ int (4B) │char │ 3 bytes padding to 8-byte alignment   │
//     └──────────┴─────┴─────────────────────────────────────────┘
//
// KEY IDEA — LEXICOGRAPHIC ORDERING
// ---------------------------------
// operator< compares `first` first; only if first ties does it look at second.
// This is exactly how std::map orders keys when values are Pair<Key, Value>:
//
//     (1, 9) < (2, 0)   because 1 < 2  (second ignored)
//     (1, 3) < (1, 9)   because first ties, then 3 < 9
//
//     compare a vs b:
//         a.first < b.first ?
//             yes → a < b
//             no  → a.first == b.first ? (a.second < b.second) : (a < b)
//
// Key characteristics:
// - Zero heap allocation; both members live inline in the Pair object
// - Aggregate-like: public `first` / `second` (structured bindings work)
// - O(1) construction, access, copy, move, and comparison
// - make_pair() deduces types via perfect forwarding (like std::make_pair)
// ============================================================================

/**
 * @brief Heterogeneous two-tuple with public `first` and `second` members.
 *
 * Used as the element type in Map/Set and anywhere a function needs to return
 * or pass around two related values without inventing a struct.
 */
template<typename T1, typename T2>
struct Pair {
    T1 first;   ///< Primary element (e.g. map key, x-coordinate)
    T2 second;  ///< Secondary element (e.g. mapped value, y-coordinate)

    /**
     * @brief Default-construct both members (value-initialization).
     */
    Pair() : first(), second() {}

    /**
     * @brief Copy-construct from lvalue arguments.
     *
     *     Pair<std::string, int> p("hello", 42);
     *     // first  ← copy of "hello"
     *     // second ← copy of 42
     */
    Pair(const T1& f, const T2& s) : first(f), second(s) {}

    /**
     * @brief Move-construct from rvalue arguments.
     *
     * Steals resources from temporaries (e.g. moving a string's buffer)
     * instead of deep-copying:
     *
     *     Pair<std::string, int> p(std::move(big_string), 7);
     *     // big_string may now be empty; buffer stolen into first
     */
    Pair(T1&& f, T2&& s) : first(std::move(f)), second(std::move(s)) {}

    Pair(const Pair&) = default;
    Pair(Pair&&) = default;
    Pair& operator=(const Pair&) = default;
    Pair& operator=(Pair&&) = default;

    /**
     * @brief Member-wise equality.
     *
     *     (a, b) == (c, d)  iff  a == c  AND  b == d
     *
     * Both comparisons must be valid for the element types.
     */
    bool operator==(const Pair& other) const {
        return first == other.first && second == other.second;
    }

    /**
     * @brief Logical negation of operator==.
     */
    bool operator!=(const Pair& other) const {
        return !(*this == other);
    }

    /**
     * @brief Lexicographic strict-less-than.
     *
     * Step by step:
     *   1. If first < other.first  → true  (we are smaller)
     *   2. If other.first < first  → false (we are larger)
     *   3. Otherwise first ties → compare second
     *
     * ASCII decision tree:
     *
     *     this.first vs other.first
     *           │
     *     ┌─────┴─────┐
     *   less        greater / equal
     *     │              │
     *   return        equal? ──no──▶ return false
     *   true              │
     *                  yes (tie)
     *                     │
     *              this.second < other.second ?
     *
     * WHY lexicographic order?  std::map and std::set require operator<
     * to define a strict weak ordering.  Comparing keys first, then values,
     * gives a total order on pairs whenever both element types are ordered.
     */
    bool operator<(const Pair& other) const {
        return first < other.first || (!(other.first < first) && second < other.second);
    }
};

/**
 * @brief Factory that constructs a Pair with deduced types.
 *
 * Perfect-forwarding preserves value category so rvalues move instead of copy:
 *
 *     auto p = make_pair(42, std::string("hi"));
 *     // deduces Pair<int, std::string>
 *     // 42 is copied (literal), string is moved if rvalue
 *
 * WHY not just Pair(42, "hi")?  Without make_pair, a call like
 * Pair(1, 2.0) still deduces correctly, but make_pair is the idiomatic
 * way to let the compiler pick T1/T2 from forwarding references — especially
 * when returning from functions or building pairs for map insertion.
 */
template<typename T1, typename T2>
Pair<T1, T2> make_pair(T1&& t1, T2&& t2) {
    return Pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

#endif // PAIR_HPP
