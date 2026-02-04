#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <cstddef>
#include <utility>

/**
 * @brief Tuple implementation for 2-3 elements
 * Simplified version of std::tuple
 */

// Base case
template<typename... Types>
class Tuple;

// Specialization for 2 elements
template<typename T1, typename T2>
class Tuple<T1, T2> {
private:
    T1 first_;
    T2 second_;

public:
    Tuple() : first_(), second_() {}
    Tuple(const T1& t1, const T2& t2) : first_(t1), second_(t2) {}
    Tuple(T1&& t1, T2&& t2) : first_(std::move(t1)), second_(std::move(t2)) {}
    
    template<size_t I>
    auto& get() {
        if constexpr (I == 0) return first_;
        else if constexpr (I == 1) return second_;
    }
    
    template<size_t I>
    const auto& get() const {
        if constexpr (I == 0) return first_;
        else if constexpr (I == 1) return second_;
    }
    
    bool operator==(const Tuple& other) const {
        return first_ == other.first_ && second_ == other.second_;
    }
};

// Specialization for 3 elements
template<typename T1, typename T2, typename T3>
class Tuple<T1, T2, T3> {
private:
    T1 first_;
    T2 second_;
    T3 third_;

public:
    Tuple() : first_(), second_(), third_() {}
    Tuple(const T1& t1, const T2& t2, const T3& t3) 
        : first_(t1), second_(t2), third_(t3) {}
    Tuple(T1&& t1, T2&& t2, T3&& t3) 
        : first_(std::move(t1)), second_(std::move(t2)), third_(std::move(t3)) {}
    
    template<size_t I>
    auto& get() {
        if constexpr (I == 0) return first_;
        else if constexpr (I == 1) return second_;
        else if constexpr (I == 2) return third_;
    }
    
    template<size_t I>
    const auto& get() const {
        if constexpr (I == 0) return first_;
        else if constexpr (I == 1) return second_;
        else if constexpr (I == 2) return third_;
    }
    
    bool operator==(const Tuple& other) const {
        return first_ == other.first_ && 
               second_ == other.second_ && 
               third_ == other.third_;
    }
};

// Helper to get element
template<size_t I, typename... Types>
auto& get(Tuple<Types...>& t) {
    return t.template get<I>();
}

template<size_t I, typename... Types>
const auto& get(const Tuple<Types...>& t) {
    return t.template get<I>();
}

// make_tuple helper
template<typename T1, typename T2>
Tuple<T1, T2> make_tuple(T1&& t1, T2&& t2) {
    return Tuple<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

template<typename T1, typename T2, typename T3>
Tuple<T1, T2, T3> make_tuple(T1&& t1, T2&& t2, T3&& t3) {
    return Tuple<T1, T2, T3>(std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3));
}

#endif // TUPLE_HPP
