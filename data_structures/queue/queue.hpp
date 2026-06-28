#ifndef DS_QUEUE_HPP
#define DS_QUEUE_HPP

// ============================================================================
//  Queue<T> -- a textbook FIFO queue (circular buffer)
// ============================================================================
//
// THE IDEA
// --------
// A queue is "first in, first out": enqueue adds at the back, dequeue removes
// from the front. A *circular buffer* stores elements in a fixed array but
// treats it as a ring: `head_` points to the front, `tail_` to the next free
// slot at the back. When an index reaches the end it wraps to 0.
//
// PICTURE (capacity=8, size=4)
//
//     index:  0    1    2    3    4    5    6    7
//           +----+----+----+----+----+----+----+----+
//           |    | 20 | 30 | 40 |    |    | 10 |    |
//           +----+----+----+----+----+----+----+----+
//                    ^head          ^tail
//                    dequeue here   enqueue here
//
//     enqueue(50): write at tail (4), tail becomes 5
//     dequeue():   return data[head], head becomes 2
//
// GROWTH
// --------
// When size_ == capacity_, we allocate a bigger array and *unwrap* the ring
// so elements sit contiguously from index 0 (simplifies indexing).
//
// COMPLEXITY
//     enqueue / dequeue / front / size / empty ... O(1) amortized
// ============================================================================

#include <cstddef>      // std::size_t
#include <utility>      // std::move

namespace ds {

template <typename T>
class Queue {
public:
    // ---- construction / destruction ----------------------------------------

    Queue() : data_(nullptr), head_(0), tail_(0), size_(0), capacity_(0) {}

    Queue(const Queue& other)
        : data_(other.capacity_ ? new T[other.capacity_] : nullptr),
          head_(other.head_), tail_(other.tail_),
          size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i < size_; ++i)
            data_[i] = other.data_[(other.head_ + i) % other.capacity_];
        head_ = 0;
        tail_ = size_;
    }

    Queue& operator=(Queue other) {
        swap(other);
        return *this;
    }

    ~Queue() { delete[] data_; }

    // ---- size / access -----------------------------------------------------

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // Front element (next to dequeue).
    T& front() { return data_[head_]; }
    const T& front() const { return data_[head_]; }

    // ---- modifiers ---------------------------------------------------------

    // Add at the back, growing if the ring is full.
    void enqueue(const T& value) {
        if (size_ == capacity_) grow();
        data_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
    }

    // Remove from the front.
    void dequeue() {
        if (size_ == 0) return;
        head_ = (head_ + 1) % capacity_;
        --size_;
    }

    void swap(Queue& other) noexcept {
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
    std::size_t tail_;      // index of next enqueue slot
    std::size_t size_;
    std::size_t capacity_;
};

}  // namespace ds

#endif  // DS_QUEUE_HPP
