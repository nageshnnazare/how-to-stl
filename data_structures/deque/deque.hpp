#ifndef DS_DEQUE_HPP
#define DS_DEQUE_HPP

// ============================================================================
//  Deque<T> -- a textbook double-ended queue (circular buffer)
// ============================================================================
//
// THE IDEA
// --------
// A deque (double-ended queue) lets you push and pop at *both* ends in O(1).
// We store elements in a circular buffer. `head_` points to the front element;
// `tail_` points one past the back. Pushing front decrements `head_` (with
// wrap); pushing back writes at `tail_` and advances it.
//
// PICTURE (capacity=8, size=4, elements 10..40)
//
//     index:  0    1    2    3    4    5    6    7
//           +----+----+----+----+----+----+----+----+
//           |    | 20 | 30 | 40 |    |    | 10 |    |
//           +----+----+----+----+----+----+----+----+
//                    ^head          ^tail
//                    front=20       back=40
//
//     push_front(5):  head moves left (wraps), write 5
//     push_back(50):  write at tail, tail advances
//
// GROWTH
// --------
// When full, allocate a bigger buffer and unwrap the ring contiguously.
//
// COMPLEXITY
//     push_front / push_back / pop_front / pop_back ... O(1) amortized
//     front / back / operator[] / size / empty ........ O(1)
// ============================================================================

#include <cstddef>      // std::size_t
#include <stdexcept>    // std::out_of_range
#include <utility>      // std::move

namespace ds {

template <typename T>
class Deque {
public:
    // ---- construction / destruction ----------------------------------------

    Deque() : data_(nullptr), head_(0), tail_(0), size_(0), capacity_(0) {}

    Deque(const Deque& other)
        : data_(other.capacity_ ? new T[other.capacity_] : nullptr),
          head_(0), tail_(other.size_), size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i < size_; ++i)
            data_[i] = other.data_[(other.head_ + i) % other.capacity_];
    }

    Deque& operator=(Deque other) {
        swap(other);
        return *this;
    }

    ~Deque() { delete[] data_; }

    // ---- size / access -----------------------------------------------------

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T& front() { return data_[head_]; }
    const T& front() const { return data_[head_]; }

    T& back() { return data_[(tail_ + capacity_ - 1) % capacity_]; }
    const T& back() const { return data_[(tail_ + capacity_ - 1) % capacity_]; }

    // Random access by logical index (0 = front).
    T& operator[](std::size_t i) { return data_[(head_ + i) % capacity_]; }
    const T& operator[](std::size_t i) const { return data_[(head_ + i) % capacity_]; }

    // ---- modifiers ---------------------------------------------------------

    void push_front(const T& value) {
        if (size_ == capacity_) grow();
        head_ = (head_ + capacity_ - 1) % capacity_;
        data_[head_] = value;
        ++size_;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) grow();
        data_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
    }

    void pop_front() {
        if (size_ == 0) return;
        head_ = (head_ + 1) % capacity_;
        --size_;
    }

    void pop_back() {
        if (size_ == 0) return;
        tail_ = (tail_ + capacity_ - 1) % capacity_;
        --size_;
    }

    void swap(Deque& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    void grow() {
        std::size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
        T* fresh = new T[new_cap];
        for (std::size_t i = 0; i < size_; ++i)
            fresh[i] = std::move(data_[(head_ + i) % capacity_]);
        delete[] data_;
        data_ = fresh;
        head_ = 0;
        tail_ = size_;
        capacity_ = new_cap;
    }

    T* data_;
    std::size_t head_;      // index of front element
    std::size_t tail_;      // index of next slot after back
    std::size_t size_;
    std::size_t capacity_;
};

}  // namespace ds

#endif  // DS_DEQUE_HPP
