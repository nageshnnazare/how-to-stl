#ifndef STACK_HPP
#define STACK_HPP

#include <cstddef>   // for size_t
#include <deque>     // default underlying container
#include <utility>   // for move, swap

// ============================================================================
//  Stack<T> -- a hand-rolled std::stack (LIFO container adapter)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Stack is NOT a standalone data structure here — it is a *container adapter*.
// All storage and allocation live in an underlying Container `c_` (default:
// std::deque<T>). Stack exposes only the LIFO interface: push/pop/top at the
// *top* end, which maps to the back of the underlying sequence.
//
// THE ONE FIELD
// -------------
//
//     c_  -> underlying sequence (default std::deque<T>)
//
// Stack adds no extra nodes or pointers; it is a thin policy layer over `c_`.
//
// LIFO LAYOUT (default deque backend — logical "top" is the back)
// ---------------------------------------------------------------
//
//     Stack adapter                    underlying c_ (deque)
//     ┌─────────────┐                 front                           back = TOP
//     │ c_     ●────┼────────────────▶ [10] [20] [30] [40]
//     └─────────────┘                      ▲              ▲
//                                      bottom          top() / pop() / push()
//
// Vertical mental picture (last pushed sits on top):
//
//         push(40) ──▶  ┌─────┐
//         push(30) ──▶  │ 40  │  ← top (back of deque)
//         push(20) ──▶  │ 30  │
//         push(10) ──▶  │ 20  │
//                       │ 10  │
//                       └─────┘
//         pop() removes from top: 40 first (Last In, First Out)
//
// KEY OPERATIONS (all at ONE end — the back)
// ------------------------------------------
//     push(x)  →  c_.push_back(x)     grow at top
//     pop()    →  c_.pop_back()       shrink at top
//     top()    →  c_.back()           read top without remove
//
// WHY deque as default (like std::stack): O(1) push/pop at back without
// invalidating pointers elsewhere; unlike vector, no reallocation on growth
// that would invalidate all references at once.
//
// Key characteristics:
// - LIFO: last pushed is first popped
// - O(1) push, pop, top (amortized for vector backend)
// - No iterators exposed — only the top element is visible
// - Customizable backend: Stack<int, std::vector<int>> etc.
// ============================================================================

template<typename T, typename Container = std::deque<T>>
class Stack {
private:
    Container c_;  // underlying sequence; top == back for default mapping

public:
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using container_type = Container;

    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================

    /** @brief Default constructor — empty underlying container. */
    Stack() : c_() {}

    /**
     * @brief Construct stack by copying an existing container.
     *
     * Useful to wrap a pre-filled deque/vector as a stack in one step.
     */
    explicit Stack(const Container& cont) : c_(cont) {}

    /**
     * @brief Construct stack by moving an existing container.
     *
     * The source container is left empty; stack owns the elements.
     */
    explicit Stack(Container&& cont) : c_(std::move(cont)) {}

    /** @brief Copy constructor — deep copy of underlying sequence. */
    Stack(const Stack& other) : c_(other.c_) {}

    /** @brief Move constructor — O(1) steal of underlying container. */
    Stack(Stack&& other) noexcept : c_(std::move(other.c_)) {}

    /** @brief Destructor — underlying container destroyed (RAII). */
    ~Stack() = default;

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /** @brief Copy assignment — replace contents with copy of other's sequence. */
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            c_ = other.c_;
        }
        return *this;
    }

    /** @brief Move assignment — steal other's underlying container. */
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            c_ = std::move(other.c_);
        }
        return *this;
    }

    // ============================================================================
    // ELEMENT ACCESS
    // ============================================================================

    /**
     * @brief Access the top element (most recently pushed).
     *
     *     stack:  [..][..][TOP]  ← c_.back()
     *
     * Undefined if empty (like std::stack). No bounds check.
     */
    reference top() {
        return c_.back();
    }

    /** @brief const overload — read-only view of top. */
    const_reference top() const {
        return c_.back();
    }

    // ============================================================================
    // CAPACITY
    // ============================================================================

    /** @brief True when underlying container has no elements. */
    bool empty() const noexcept {
        return c_.empty();
    }

    /** @brief Number of elements on the stack. */
    size_type size() const noexcept {
        return c_.size();
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Push a copy onto the top (LIFO grow at back).
     *
     *     before:  [10][20]     push(30)     after:  [10][20][30]
     *                                              top ────────▲
     *
     * WHY push_back: stack top is modeled as the *back* of the sequence so all
     * mutations hit one end only — the defining constraint of stack adapters.
     */
    void push(const value_type& value) {
        c_.push_back(value);
    }

    /**
     * @brief Push by move — avoids copy when source is expiring.
     *
     * Same layout as copy push; element is move-constructed into new back slot.
     */
    void push(value_type&& value) {
        c_.push_back(std::move(value));
    }

    /**
     * @brief Construct element in-place at top via perfect forwarding.
     *
     *     c_.emplace_back(args...)  →  new top without extra move/copy of T
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        c_.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Remove the top element (does not return it — call top() first).
     *
     *     before:  [10][20][30]   pop()   after:  [10][20]
     *                    top ▲                      new top ▲
     *
     * WHY pop_back: mirrors push_back at the same end. Undefined if empty.
     */
    void pop() {
        c_.pop_back();
    }

    /**
     * @brief Exchange underlying containers with another stack in O(1).
     */
    void swap(Stack& other) noexcept {
        std::swap(c_, other.c_);
    }

    // ============================================================================
    // COMPARISON OPERATORS
    // ============================================================================

    /** @brief Lexicographic equality of underlying sequences. */
    friend bool operator==(const Stack& lhs, const Stack& rhs) {
        return lhs.c_ == rhs.c_;
    }

    /** @brief Inequality via negated equality. */
    friend bool operator!=(const Stack& lhs, const Stack& rhs) {
        return !(lhs == rhs);
    }

    /** @brief Lexicographic less on underlying containers. */
    friend bool operator<(const Stack& lhs, const Stack& rhs) {
        return lhs.c_ < rhs.c_;
    }

    /** @brief Lexicographic <= */
    friend bool operator<=(const Stack& lhs, const Stack& rhs) {
        return !(rhs < lhs);
    }

    /** @brief Lexicographic > */
    friend bool operator>(const Stack& lhs, const Stack& rhs) {
        return rhs < lhs;
    }

    /** @brief Lexicographic >= */
    friend bool operator>=(const Stack& lhs, const Stack& rhs) {
        return !(lhs < rhs);
    }
};

/** @brief Non-member swap — ADL-friendly like std::swap(stack, stack). */
template<typename T, typename Container>
void swap(Stack<T, Container>& lhs, Stack<T, Container>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // STACK_HPP
