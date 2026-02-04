#ifndef STACK_HPP
#define STACK_HPP

#include <cstddef>
#include <deque>
#include <utility>

/**
 * @brief Stack implementation - LIFO (Last In, First Out) container adapter
 * 
 * Stack is a container adapter that provides LIFO data structure.
 * 
 * Key characteristics:
 * - Last In, First Out (LIFO) access
 * - O(1) push, pop, and top operations
 * - No iteration (only top element accessible)
 * - Built on top of underlying container (default: deque)
 */

template<typename T, typename Container = std::deque<T>>
class Stack {
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
    Stack() : c_() {}
    
    /**
     * @brief Constructor with container
     */
    explicit Stack(const Container& cont) : c_(cont) {}
    
    /**
     * @brief Constructor with move container
     */
    explicit Stack(Container&& cont) : c_(std::move(cont)) {}
    
    /**
     * @brief Copy constructor
     */
    Stack(const Stack& other) : c_(other.c_) {}
    
    /**
     * @brief Move constructor
     */
    Stack(Stack&& other) noexcept : c_(std::move(other.c_)) {}
    
    /**
     * @brief Destructor
     */
    ~Stack() = default;
    
    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================
    
    /**
     * @brief Copy assignment
     */
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            c_ = other.c_;
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     */
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
     * @brief Access the top element
     * @return reference to top element
     */
    reference top() {
        return c_.back();
    }
    
    /**
     * @brief Access the top element (const)
     * @return const reference to top element
     */
    const_reference top() const {
        return c_.back();
    }
    
    // ============================================================================
    // CAPACITY
    // ============================================================================
    
    /**
     * @brief Check if stack is empty
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
     * @brief Push element onto stack (copy)
     * Time complexity: O(1)
     */
    void push(const value_type& value) {
        c_.push_back(value);
    }
    
    /**
     * @brief Push element onto stack (move)
     * Time complexity: O(1)
     */
    void push(value_type&& value) {
        c_.push_back(std::move(value));
    }
    
    /**
     * @brief Construct element in-place at top
     * Time complexity: O(1)
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        c_.emplace_back(std::forward<Args>(args)...);
    }
    
    /**
     * @brief Remove top element
     * Time complexity: O(1)
     */
    void pop() {
        c_.pop_back();
    }
    
    /**
     * @brief Swap contents with another stack
     */
    void swap(Stack& other) noexcept {
        std::swap(c_, other.c_);
    }
    
    // ============================================================================
    // COMPARISON OPERATORS
    // ============================================================================
    
    /**
     * @brief Equality comparison
     */
    friend bool operator==(const Stack& lhs, const Stack& rhs) {
        return lhs.c_ == rhs.c_;
    }
    
    /**
     * @brief Inequality comparison
     */
    friend bool operator!=(const Stack& lhs, const Stack& rhs) {
        return !(lhs == rhs);
    }
    
    /**
     * @brief Less than comparison
     */
    friend bool operator<(const Stack& lhs, const Stack& rhs) {
        return lhs.c_ < rhs.c_;
    }
    
    /**
     * @brief Less than or equal comparison
     */
    friend bool operator<=(const Stack& lhs, const Stack& rhs) {
        return !(rhs < lhs);
    }
    
    /**
     * @brief Greater than comparison
     */
    friend bool operator>(const Stack& lhs, const Stack& rhs) {
        return rhs < lhs;
    }
    
    /**
     * @brief Greater than or equal comparison
     */
    friend bool operator>=(const Stack& lhs, const Stack& rhs) {
        return !(lhs < rhs);
    }
};

/**
 * @brief Swap specialization
 */
template<typename T, typename Container>
void swap(Stack<T, Container>& lhs, Stack<T, Container>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // STACK_HPP

