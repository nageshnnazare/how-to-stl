#ifndef DS_DYNAMIC_ARRAY_HPP
#define DS_DYNAMIC_ARRAY_HPP

// ============================================================================
//  DynamicArray<T> -- a textbook resizable array ("how std::vector works")
// ============================================================================
//
// THE IDEA
// --------
// A plain C array has a fixed size chosen at creation. A *dynamic* array hides
// a fixed C array behind a friendlier face: when it fills up, we quietly
// allocate a bigger array, copy the elements across, and throw the old one
// away. To the caller it looks like an array that simply grows on demand.
//
// THREE NUMBERS DESCRIBE IT
//
//     data_      ->  pointer to a heap array of `capacity_` slots
//     size_      ->  how many slots currently hold real elements
//     capacity_  ->  how many slots exist before we must grow
//
//     size=4, capacity=8
//     index:   0    1    2    3    4    5    6    7
//            +----+----+----+----+----+----+----+----+
//     data_->| 10 | 20 | 30 | 40 |    |    |    |    |
//            +----+----+----+----+----+----+----+----+
//             \________ used ____/ \_______ spare ___/
//
// GROWTH: WHY DOUBLE?
// -------------------
// When size_ == capacity_ and one more push_back arrives, we grow. We *double*
// the capacity rather than add a constant. Doubling means that across N pushes
// the total copying work is N + N/2 + N/4 + ... < 2N, so each push costs O(1)
// "on average" (amortized), even though the occasional resize is O(N).
//
//     full: [a][b]            (cap 2)
//            |  |  copy
//            v  v
//     new:  [a][b][ ][ ]      (cap 4)   then write c -> [a][b][c][ ]
//
// COMPLEXITY
//     push_back / pop_back .... O(1) amortized
//     operator[] / at ......... O(1)
//     insert / erase (middle) . O(N)   (shift the tail)
//     find .................... O(N)
// ============================================================================

#include <cstddef>      // std::size_t
#include <stdexcept>    // std::out_of_range
#include <utility>      // std::move

namespace ds {

template <typename T>
class DynamicArray {
public:
    // ---- construction / destruction ----------------------------------------

    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicArray(std::size_t count, const T& value = T())
        : data_(new T[count]), size_(count), capacity_(count) {
        for (std::size_t i = 0; i < count; ++i) data_[i] = value;
    }

    // Deep copy: a new array with copies of the other's elements.
    DynamicArray(const DynamicArray& other)
        : data_(other.capacity_ ? new T[other.capacity_] : nullptr),
          size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    // Move: steal the buffer, leave the source empty.
    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }

    DynamicArray& operator=(DynamicArray other) {  // copy-and-swap
        swap(other);
        return *this;
    }

    ~DynamicArray() { delete[] data_; }

    // ---- capacity ----------------------------------------------------------

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    // Ensure room for at least `min_cap` elements (never shrinks).
    void reserve(std::size_t min_cap) {
        if (min_cap <= capacity_) return;
        reallocate(min_cap);
    }

    // ---- element access ----------------------------------------------------

    // Unchecked: fast, but reading out of range is undefined behaviour.
    T& operator[](std::size_t i) { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    // Checked: throws std::out_of_range on a bad index.
    T& at(std::size_t i) {
        if (i >= size_) throw std::out_of_range("DynamicArray::at");
        return data_[i];
    }
    const T& at(std::size_t i) const {
        if (i >= size_) throw std::out_of_range("DynamicArray::at");
        return data_[i];
    }

    T& front() { return data_[0]; }
    T& back() { return data_[size_ - 1]; }

    // ---- modifiers ---------------------------------------------------------

    // Append to the end, growing (doubling) if the buffer is full.
    void push_back(const T& value) {
        if (size_ == capacity_) grow();
        data_[size_++] = value;
    }

    void pop_back() {
        if (size_ > 0) --size_;
    }

    // Insert `value` at position `index`, shifting the tail one slot right.
    //
    //   insert(2, X):  [a][b][c][d]
    //                        ^ make a hole here
    //                  [a][b][X][c][d]
    void insert(std::size_t index, const T& value) {
        if (index > size_) throw std::out_of_range("DynamicArray::insert");
        if (size_ == capacity_) grow();
        for (std::size_t i = size_; i > index; --i) data_[i] = std::move(data_[i - 1]);
        data_[index] = value;
        ++size_;
    }

    // Remove the element at `index`, shifting the tail one slot left.
    void erase(std::size_t index) {
        if (index >= size_) throw std::out_of_range("DynamicArray::erase");
        for (std::size_t i = index; i + 1 < size_; ++i) data_[i] = std::move(data_[i + 1]);
        --size_;
    }

    void clear() { size_ = 0; }

    // ---- search ------------------------------------------------------------

    // Linear scan; returns the index of the first match or size() if absent.
    std::size_t find(const T& value) const {
        for (std::size_t i = 0; i < size_; ++i)
            if (data_[i] == value) return i;
        return size_;
    }

    // ---- iteration (raw pointers act as iterators) -------------------------

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

    void swap(DynamicArray& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    // Double the capacity (start at 1 when empty).
    void grow() { reallocate(capacity_ == 0 ? 1 : capacity_ * 2); }

    // Move into a fresh buffer of exactly `new_cap` slots.
    void reallocate(std::size_t new_cap) {
        T* fresh = new T[new_cap];
        for (std::size_t i = 0; i < size_; ++i) fresh[i] = std::move(data_[i]);
        delete[] data_;
        data_ = fresh;
        capacity_ = new_cap;
    }

    T* data_;
    std::size_t size_;
    std::size_t capacity_;
};

}  // namespace ds

#endif  // DS_DYNAMIC_ARRAY_HPP
