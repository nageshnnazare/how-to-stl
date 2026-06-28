#ifndef DS_STACK_HPP
#define DS_STACK_HPP

// ============================================================================
//  Stack<T> -- a textbook LIFO stack (array-backed)
// ============================================================================
//
// THE IDEA
// --------
// A stack is "last in, first out": the most recently pushed element is the
// first one popped. We implement it with a dynamic array and a `top_` index
// that points one past the last element. Push writes at `top_` and advances;
// pop retreats `top_` without destroying the slot (like vector::pop_back).
//
// PICTURE (LIFO)
//
//     push 3, push 7, push 2:
//
//         +---+
//     top | 2 |  <-- pop returns 2
//         +---+
//         | 7 |
//         +---+
//         | 3 |
//         +---+
//     bottom
//
//     pop -> 2, top moves down; next pop -> 7
//
// COMPLEXITY
//     push / pop / top / size / empty ... O(1) amortized (push may grow)
// ============================================================================

#include <cstddef>      // std::size_t
#include <utility>      // std::move

namespace ds {

template <typename T>
class Stack {
public:
    // ---- construction / destruction ----------------------------------------

    Stack() : data_(nullptr), top_(0), capacity_(0) {}

    Stack(const Stack& other)
        : data_(other.capacity_ ? new T[other.capacity_] : nullptr),
          top_(other.top_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i < top_; ++i) data_[i] = other.data_[i];
    }

    Stack& operator=(Stack other) {
        swap(other);
        return *this;
    }

    ~Stack() { delete[] data_; }

    // ---- size / access -----------------------------------------------------

    std::size_t size() const { return top_; }
    bool empty() const { return top_ == 0; }

    // Top of the stack (most recently pushed).
    T& top() { return data_[top_ - 1]; }
    const T& top() const { return data_[top_ - 1]; }

    // ---- modifiers ---------------------------------------------------------

    // Push onto the top, growing (doubling) if the buffer is full.
    void push(const T& value) {
        if (top_ == capacity_) grow();
        data_[top_++] = value;
    }

    // Remove the top element.
    void pop() {
        if (top_ > 0) --top_;
    }

    void swap(Stack& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(top_, other.top_);
        std::swap(capacity_, other.capacity_);
    }

private:
    void grow() {
        std::size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
        T* fresh = new T[new_cap];
        for (std::size_t i = 0; i < top_; ++i) fresh[i] = std::move(data_[i]);
        delete[] data_;
        data_ = fresh;
        capacity_ = new_cap;
    }

    T* data_;
    std::size_t top_;       // number of elements (index of next free slot)
    std::size_t capacity_;
};

}  // namespace ds

#endif  // DS_STACK_HPP
