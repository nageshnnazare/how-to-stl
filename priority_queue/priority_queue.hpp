#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <cstddef>
#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>

/**
 * @brief Priority Queue implementation using binary heap
 * 
 * A container adapter providing constant time lookup of the largest (by default) element.
 * 
 * Key characteristics:
 * - Max heap by default (largest element at top)
 * - O(log n) insertion and removal
 * - O(1) access to top element
 * - Implemented using binary heap stored in vector
 */

template<typename T, typename Container = std::vector<T>, typename Compare = std::less<T>>
class PriorityQueue {
private:
    Container c_;       // Underlying container
    Compare comp_;      // Comparison function
    
    /**
     * @brief Get parent index
     */
    size_t parent(size_t i) const {
        return (i - 1) / 2;
    }
    
    /**
     * @brief Get left child index
     */
    size_t left(size_t i) const {
        return 2 * i + 1;
    }
    
    /**
     * @brief Get right child index
     */
    size_t right(size_t i) const {
        return 2 * i + 2;
    }
    
    /**
     * @brief Bubble up element at index i to maintain heap property
     * Used after insertion
     */
    void heapify_up(size_t i) {
        while (i > 0 && comp_(c_[parent(i)], c_[i])) {
            std::swap(c_[i], c_[parent(i)]);
            i = parent(i);
        }
    }
    
    /**
     * @brief Bubble down element at index i to maintain heap property
     * Used after removal
     */
    void heapify_down(size_t i) {
        size_t largest = i;
        size_t l = left(i);
        size_t r = right(i);
        
        // Find largest among node and its children
        if (l < c_.size() && comp_(c_[largest], c_[l])) {
            largest = l;
        }
        if (r < c_.size() && comp_(c_[largest], c_[r])) {
            largest = r;
        }
        
        // If largest is not current node, swap and continue
        if (largest != i) {
            std::swap(c_[i], c_[largest]);
            heapify_down(largest);
        }
    }
    
    /**
     * @brief Build heap from unsorted data (Floyd's algorithm)
     * O(n) time complexity
     */
    void build_heap() {
        // Start from last non-leaf node and heapify down
        if (c_.size() <= 1) return;
        
        for (int i = parent(c_.size() - 1); i >= 0; --i) {
            heapify_down(static_cast<size_t>(i));
        }
    }

public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using container_type = Container;
    using value_compare = Compare;
    
    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================
    
    /**
     * @brief Default constructor
     */
    PriorityQueue() : c_(), comp_() {}
    
    /**
     * @brief Constructor with custom comparator
     */
    explicit PriorityQueue(const Compare& compare) : c_(), comp_(compare) {}
    
    /**
     * @brief Constructor with custom comparator and container
     */
    PriorityQueue(const Compare& compare, const Container& cont)
        : c_(cont), comp_(compare) {
        build_heap();
    }
    
    /**
     * @brief Constructor with custom comparator and move container
     */
    PriorityQueue(const Compare& compare, Container&& cont)
        : c_(std::move(cont)), comp_(compare) {
        build_heap();
    }
    
    /**
     * @brief Range constructor
     */
    template<typename InputIt>
    PriorityQueue(InputIt first, InputIt last, const Compare& compare = Compare())
        : c_(first, last), comp_(compare) {
        build_heap();
    }
    
    /**
     * @brief Copy constructor
     */
    PriorityQueue(const PriorityQueue& other) : c_(other.c_), comp_(other.comp_) {}
    
    /**
     * @brief Move constructor
     */
    PriorityQueue(PriorityQueue&& other) noexcept
        : c_(std::move(other.c_)), comp_(std::move(other.comp_)) {}
    
    /**
     * @brief Destructor
     */
    ~PriorityQueue() = default;
    
    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================
    
    /**
     * @brief Copy assignment
     */
    PriorityQueue& operator=(const PriorityQueue& other) {
        if (this != &other) {
            c_ = other.c_;
            comp_ = other.comp_;
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     */
    PriorityQueue& operator=(PriorityQueue&& other) noexcept {
        if (this != &other) {
            c_ = std::move(other.c_);
            comp_ = std::move(other.comp_);
        }
        return *this;
    }
    
    // ============================================================================
    // ELEMENT ACCESS
    // ============================================================================
    
    /**
     * @brief Access the top element (highest priority)
     * @return const reference to top element
     * @throws std::out_of_range if queue is empty
     */
    const_reference top() const {
        if (empty()) {
            throw std::out_of_range("PriorityQueue::top: queue is empty");
        }
        return c_.front();
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
     * @brief Insert element (copy)
     * Time complexity: O(log n)
     */
    void push(const value_type& value) {
        c_.push_back(value);
        heapify_up(c_.size() - 1);
    }
    
    /**
     * @brief Insert element (move)
     * Time complexity: O(log n)
     */
    void push(value_type&& value) {
        c_.push_back(std::move(value));
        heapify_up(c_.size() - 1);
    }
    
    /**
     * @brief Construct element in-place
     * Time complexity: O(log n)
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        c_.emplace_back(std::forward<Args>(args)...);
        heapify_up(c_.size() - 1);
    }
    
    /**
     * @brief Remove top element
     * Time complexity: O(log n)
     * @throws std::out_of_range if queue is empty
     */
    void pop() {
        if (empty()) {
            throw std::out_of_range("PriorityQueue::pop: queue is empty");
        }
        
        // Move last element to top and heapify down
        c_.front() = std::move(c_.back());
        c_.pop_back();
        
        if (!empty()) {
            heapify_down(0);
        }
    }
    
    /**
     * @brief Swap contents with another priority queue
     */
    void swap(PriorityQueue& other) noexcept {
        std::swap(c_, other.c_);
        std::swap(comp_, other.comp_);
    }
    
    // ============================================================================
    // COMPARISON (for testing/debugging)
    // ============================================================================
    
    /**
     * @brief Check if heap property is satisfied (for testing)
     */
    bool is_heap() const {
        for (size_t i = 0; i < c_.size(); ++i) {
            size_t l = left(i);
            size_t r = right(i);
            
            if (l < c_.size() && comp_(c_[i], c_[l])) {
                return false;
            }
            if (r < c_.size() && comp_(c_[i], c_[r])) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief Swap specialization
 */
template<typename T, typename Container, typename Compare>
void swap(PriorityQueue<T, Container, Compare>& lhs,
          PriorityQueue<T, Container, Compare>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // PRIORITY_QUEUE_HPP

