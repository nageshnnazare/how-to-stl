#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstddef>
#include <deque>
#include <utility>

/**
 * @brief Queue implementation - FIFO (First In, First Out) container adapter
 * 
 * Queue is a container adapter that provides FIFO data structure.
 * 
 * Key characteristics:
 * - First In, First Out (FIFO) access
 * - O(1) push, pop, front, and back operations
 * - No iteration (only front/back accessible)
 * - Built on top of underlying container (default: deque)
 */

template<typename T, typename Container = std::deque<T>>
class Queue {
private:
    Container c_;  // Underlying container
    
public:
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using container_type = Container;
    
    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================
    
    /**
     * @brief Default constructor
     */
    Queue() : c_() {}
    
    /**
     * @brief Constructor with container
     */
    explicit Queue(const Container& cont) : c_(cont) {}
    
    /**
     * @brief Constructor with move container
     */
    explicit Queue(Container&& cont) : c_(std::move(cont)) {}
    
    /**
     * @brief Copy constructor
     */
    Queue(const Queue& other) : c_(other.c_) {}
    
    /**
     * @brief Move constructor
     */
    Queue(Queue&& other) noexcept : c_(std::move(other.c_)) {}
    
    /**
     * @brief Destructor
     */
    ~Queue() = default;
    
    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================
    
    /**
     * @brief Copy assignment
     */
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            c_ = other.c_;
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     */
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
     * @brief Access the first element
     * @return reference to front element
     */
    reference front() {
        return c_.front();
    }
    
    /**
     * @brief Access the first element (const)
     * @return const reference to front element
     */
    const_reference front() const {
        return c_.front();
    }
    
    /**
     * @brief Access the last element
     * @return reference to back element
     */
    reference back() {
        return c_.back();
    }
    
    /**
     * @brief Access the last element (const)
     * @return const reference to back element
     */
    const_reference back() const {
        return c_.back();
    }
    
    // ============================================================================
    // CAPACITY
    // ============================================================================
    
    /**
     * @brief Check if queue is empty
     */
    bool empty() const noexcept {
        return c_.empty();
    }
    
    /**
     * @brief Get number of elements
     */
    size_type size() const noexcept {
        return c_.size();
    }
    
    // ============================================================================
    // MODIFIERS
    // ============================================================================
    
    /**
     * @brief Push element to back of queue (copy)
     * Time complexity: O(1)
     */
    void push(const value_type& value) {
        c_.push_back(value);
    }
    
    /**
     * @brief Push element to back of queue (move)
     * Time complexity: O(1)
     */
    void push(value_type&& value) {
        c_.push_back(std::move(value));
    }
    
    /**
     * @brief Construct element in-place at back
     * Time complexity: O(1)
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        c_.emplace_back(std::forward<Args>(args)...);
    }
    
    /**
     * @brief Remove front element
     * Time complexity: O(1)
     */
    void pop() {
        c_.pop_front();
    }
    
    /**
     * @brief Swap contents with another queue
     */
    void swap(Queue& other) noexcept {
        std::swap(c_, other.c_);
    }
    
    // ============================================================================
    // COMPARISON OPERATORS
    // ============================================================================
    
    /**
     * @brief Equality comparison
     */
    friend bool operator==(const Queue& lhs, const Queue& rhs) {
        return lhs.c_ == rhs.c_;
    }
    
    /**
     * @brief Inequality comparison
     */
    friend bool operator!=(const Queue& lhs, const Queue& rhs) {
        return !(lhs == rhs);
    }
    
    /**
     * @brief Less than comparison
     */
    friend bool operator<(const Queue& lhs, const Queue& rhs) {
        return lhs.c_ < rhs.c_;
    }
    
    /**
     * @brief Less than or equal comparison
     */
    friend bool operator<=(const Queue& lhs, const Queue& rhs) {
        return !(rhs < lhs);
    }
    
    /**
     * @brief Greater than comparison
     */
    friend bool operator>(const Queue& lhs, const Queue& rhs) {
        return rhs < lhs;
    }
    
    /**
     * @brief Greater than or equal comparison
     */
    friend bool operator>=(const Queue& lhs, const Queue& rhs) {
        return !(lhs < rhs);
    }
};

/**
 * @brief Swap specialization
 */
template<typename T, typename Container>
void swap(Queue<T, Container>& lhs, Queue<T, Container>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // QUEUE_HPP

