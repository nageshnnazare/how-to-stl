#ifndef PAIR_HPP
#define PAIR_HPP

#include <utility>

/**
 * @brief Pair of two values
 * Used extensively in map/set
 */
template<typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;
    
    Pair() : first(), second() {}
    Pair(const T1& f, const T2& s) : first(f), second(s) {}
    Pair(T1&& f, T2&& s) : first(std::move(f)), second(std::move(s)) {}
    
    Pair(const Pair&) = default;
    Pair(Pair&&) = default;
    Pair& operator=(const Pair&) = default;
    Pair& operator=(Pair&&) = default;
    
    bool operator==(const Pair& other) const {
        return first == other.first && second == other.second;
    }
    
    bool operator!=(const Pair& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Pair& other) const {
        return first < other.first || (!(other.first < first) && second < other.second);
    }
};

template<typename T1, typename T2>
Pair<T1, T2> make_pair(T1&& t1, T2&& t2) {
    return Pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

#endif // PAIR_HPP
