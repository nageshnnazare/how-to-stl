#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstddef>   // for size_t
#include <deque>     // default underlying container
#include <utility>   // for move, swap

// ============================================================================
//  Queue<T> -- a hand-rolled std::queue (FIFO container adapter)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Queue is a *container adapter*: storage lives in underlying Container `c_`
// (default std::deque<T>). Queue restricts access to FIFO semantics — insert at
// the *back*, remove from the *front*, peek at both ends.
//
// THE ONE FIELD
// -------------
//
//     c_  -> underlying sequence (default std::deque<T>)
//
// FIFO LAYOUT (enqueue back, dequeue front)
// -----------------------------------------
//
//     Queue adapter                 underlying c_ (deque)
//     ┌─────────────┐              FRONT (dequeue)          BACK (enqueue)
//     │ c_     ●────┼─────────────▶ [10] [20] [30] [40]
//     └─────────────┘                ▲                    ▲
//                               front()              push() / back()
//                               pop()
//
// Flow diagram — elements enter right, exit left:
//
//     enqueue ──▶  ... ──▶ [10] [20] [30] [40]  ──▶ dequeue
//              (push_back)     queue grows ──▶      (pop_front)
//
//     push(10) push(20) push(30):   exit order pop→ 10, 20, 30  (First In, First Out)
//
// KEY OPERATIONS (TWO ends — opposite of stack)
// ---------------------------------------------
//     push(x)   →  c_.push_back(x)    enqueue at rear
//     pop()     →  c_.pop_front()    dequeue from front
//     front()   →  c_.front()        oldest element
//     back()    →  c_.back()         newest element (still in queue)
//
// WHY deque: O(1) push_back AND pop_front; vector would be O(n) at front.
//
// Key characteristics:
// - FIFO: first pushed is first popped
// - O(1) push, pop, front, back with deque backend
// - No iteration API — only both ends visible
// - BFS, scheduling, and buffering are classic use cases
// ============================================================================

template<typename T, typename Container = std::deque<T>>
class Queue {
private:
    Container c_;  // underlying sequence; front=dequeue end, back=enqueue end

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
    Queue() : c_() {}

    /** @brief Wrap a copy of an existing container as a queue. */
    explicit Queue(const Container& cont) : c_(cont) {}

    /** @brief Wrap a moved container as a queue. */
    explicit Queue(Container&& cont) : c_(std::move(cont)) {}

    /** @brief Copy constructor. */
    Queue(const Queue& other) : c_(other.c_) {}

    /** @brief Move constructor. */
    Queue(Queue&& other) noexcept : c_(std::move(other.c_)) {}

    /** @brief Destructor — RAII on underlying container. */
    ~Queue() = default;

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    /** @brief Copy assignment. */
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            c_ = other.c_;
        }
        return *this;
    }

    /** @brief Move assignment. */
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            c_ = std::move(other.c_);
        }
        return *this;
    }

    // ============================================================================
    // ELEMENT ACCESS
    // ============================================================================

    /**
     * @brief Access the front (oldest) element — next to be popped.
     *
     *     [10] [20] [30] [40]
     *      ▲
     *   front()  → 10
     */
    reference front() {
        return c_.front();
    }

    /** @brief const overload of front(). */
    const_reference front() const {
        return c_.front();
    }

    /**
     * @brief Access the back (newest) element — last enqueued, still waiting.
     *
     *     [10] [20] [30] [40]
     *                    ▲
     *                 back()  → 40
     */
    reference back() {
        return c_.back();
    }

    /** @brief const overload of back(). */
    const_reference back() const {
        return c_.back();
    }

    // ============================================================================
    // CAPACITY
    // ============================================================================

    /** @brief True when queue has no elements. */
    bool empty() const noexcept {
        return c_.empty();
    }

    /** @brief Number of elements waiting in the queue. */
    size_type size() const noexcept {
        return c_.size();
    }

    // ============================================================================
    // MODIFIERS
    // ============================================================================

    /**
     * @brief Enqueue a copy at the back (rear).
     *
     *     before:  [10][20]    push(30)    after:  [10][20][30]
     *                                              rear ─────▲
     *
     * WHY push_back: new arrivals join the tail; front stays the oldest.
     */
    void push(const value_type& value) {
        c_.push_back(value);
    }

    /** @brief Enqueue by move at the back. */
    void push(value_type&& value) {
        c_.push_back(std::move(value));
    }

    /**
     * @brief Construct in-place at back — avoids temporary T.
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        c_.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Dequeue the front element (oldest leaves first).
     *
     *     before:  [10][20][30]   pop()   after:  [20][30]
     *              ▲                              ▲
     *           front                           new front
     *
     * WHY pop_front: FIFO requires removing from the opposite end from push.
     * Undefined if empty.
     */
    void pop() {
        c_.pop_front();
    }

    /** @brief O(1) swap of underlying containers. */
    void swap(Queue& other) noexcept {
        std::swap(c_, other.c_);
    }

    // ============================================================================
    // COMPARISON OPERATORS
    // ============================================================================

    /** @brief Lexicographic equality of underlying sequences. */
    friend bool operator==(const Queue& lhs, const Queue& rhs) {
        return lhs.c_ == rhs.c_;
    }

    friend bool operator!=(const Queue& lhs, const Queue& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(const Queue& lhs, const Queue& rhs) {
        return lhs.c_ < rhs.c_;
    }

    friend bool operator<=(const Queue& lhs, const Queue& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>(const Queue& lhs, const Queue& rhs) {
        return rhs < lhs;
    }

    friend bool operator>=(const Queue& lhs, const Queue& rhs) {
        return !(lhs < rhs);
    }
};

/** @brief Non-member swap for ADL. */
template<typename T, typename Container>
void swap(Queue<T, Container>& lhs, Queue<T, Container>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // QUEUE_HPP
