#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cstddef>
#include <stdexcept>

// ============================================================================
//  Array<T, N> -- a hand-rolled std::array (fixed-size stack wrapper)
// ============================================================================
//
// WHAT IT IS
// ----------
// Array is a thin, zero-overhead wrapper around a C-style array T[N]. The size
// N is a compile-time template parameter — it never changes at runtime and
// there is NO heap allocation. The entire object (metadata + elements) lives
// on the stack (or inside a parent object).
//
// THE ONE DATA MEMBER
// -------------------
//
//     data_[N]   -> inline array of exactly N objects of type T
//
// Unlike Vector, there is no size_ or capacity_ field: size() returns N via
// constexpr. Unlike List, nothing is on the heap.
//
// MEMORY LAYOUT (Array<int, 5> on the stack)
// --------------------------------------------
//
//     Array<int, 5> object (entirely on stack — no pointers, no heap)
//     ┌─────────────────────────────────────────────────────────────┐
//     │ data_[0] │ data_[1] │ data_[2] │ data_[3] │ data_[4] │      │
//     │    1     │    2     │    3     │    4     │    5     │      │
//     └─────────────────────────────────────────────────────────────┘
//       index 0     1         2         3         4
//       └──────────── size() == 5 == N (compile-time constant) ─────┘
//
// COMPARISON AT A GLANCE
// ----------------------
//
//     Array<T,N>     Vector<T>        List<T>
//     stack T[N]     heap block       scattered Nodes
//     N fixed        grows            grows
//     contiguous     contiguous       linked
//
// Key characteristics:
// - Zero dynamic allocation — predictable stack footprint
// - O(1) random access by index (same as raw array)
// - Knows its own size (unlike T[] that decays and loses length)
// - Aggregate type — brace initialization like a struct
// ============================================================================

template<typename T, std::size_t N>
struct Array {
    // Inline storage: N elements, no heap, no size field. The `N ? N : 1`
    // guards the documented Array<T,0> edge case: a true zero-length C array is
    // a non-standard extension (warns under -Wpedantic), so for N==0 we reserve
    // one unused slot. size() still reports N, so begin()==end() and the array
    // iterates as empty — the phantom slot is never observed.
    T data_[N ? N : 1];

    using value_type = T;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;

    /**
     * @brief Unchecked element access — same as raw data_[i].
     */
    T& operator[](size_type i) { return data_[i]; }
    const T& operator[](size_type i) const { return data_[i]; }

    /**
     * @brief Bounds-checked access — throws std::out_of_range if i >= N.
     */
    T& at(size_type i) {
        if (i >= N) throw std::out_of_range("Array::at");
        return data_[i];
    }

    const T& at(size_type i) const {
        if (i >= N) throw std::out_of_range("Array::at");
        return data_[i];
    }

    T& front() { return data_[0]; }
    const T& front() const { return data_[0]; }
    T& back() { return data_[N-1]; }
    const T& back() const { return data_[N-1]; }

    iterator begin() { return data_; }
    const_iterator begin() const { return data_; }
    iterator end() { return data_ + N; }
    const_iterator end() const { return data_ + N; }

    constexpr size_type size() const { return N; }
    constexpr bool empty() const { return N == 0; }

    /**
     * @brief Assign every slot — O(N) loop, no allocation.
     */
    void fill(const T& value) {
        for (size_type i = 0; i < N; ++i) data_[i] = value;
    }
};

#endif // ARRAY_HPP
