#ifndef BITSET_HPP
#define BITSET_HPP

#include <cstddef>
#include <string>

/**
 * @brief Fixed-size sequence of bits
 * Efficient bit manipulation
 */
template<std::size_t N>
class Bitset {
private:
    static constexpr size_t BITS_PER_WORD = sizeof(unsigned long) * 8;
    static constexpr size_t NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;
    unsigned long words_[NUM_WORDS];
    
public:
    Bitset() {
        for (size_t i = 0; i < NUM_WORDS; ++i) words_[i] = 0;
    }
    
    Bitset(unsigned long val) : Bitset() {
        if (NUM_WORDS > 0) words_[0] = val;
    }
    
    Bitset& set(size_t pos, bool value = true) {
        if (pos >= N) return *this;
        size_t word = pos / BITS_PER_WORD;
        size_t bit = pos % BITS_PER_WORD;
        if (value) {
            words_[word] |= (1UL << bit);
        } else {
            words_[word] &= ~(1UL << bit);
        }
        return *this;
    }
    
    Bitset& reset(size_t pos) { return set(pos, false); }
    Bitset& reset() {
        for (size_t i = 0; i < NUM_WORDS; ++i) words_[i] = 0;
        return *this;
    }
    
    Bitset& flip(size_t pos) {
        if (pos >= N) return *this;
        size_t word = pos / BITS_PER_WORD;
        size_t bit = pos % BITS_PER_WORD;
        words_[word] ^= (1UL << bit);
        return *this;
    }
    
    bool test(size_t pos) const {
        if (pos >= N) return false;
        size_t word = pos / BITS_PER_WORD;
        size_t bit = pos % BITS_PER_WORD;
        return (words_[word] & (1UL << bit)) != 0;
    }
    
    bool operator[](size_t pos) const { return test(pos); }
    
    size_t count() const {
        size_t cnt = 0;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            unsigned long w = words_[i];
            while (w) {
                cnt += w & 1;
                w >>= 1;
            }
        }
        return cnt;
    }
    
    constexpr size_t size() const { return N; }
    
    bool all() const { return count() == N; }
    bool any() const { return count() > 0; }
    bool none() const { return count() == 0; }
    
    Bitset operator&(const Bitset& other) const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = words_[i] & other.words_[i];
        }
        return result;
    }
    
    Bitset operator|(const Bitset& other) const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = words_[i] | other.words_[i];
        }
        return result;
    }
    
    Bitset operator^(const Bitset& other) const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = words_[i] ^ other.words_[i];
        }
        return result;
    }
    
    Bitset operator~() const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = ~words_[i];
        }
        // Clear unused bits in last word
        if (N % BITS_PER_WORD != 0) {
            size_t used_bits = N % BITS_PER_WORD;
            result.words_[NUM_WORDS-1] &= (1UL << used_bits) - 1;
        }
        return result;
    }
};

#endif // BITSET_HPP
