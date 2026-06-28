#ifndef BITSET_HPP
#define BITSET_HPP

#include <cstddef>      // for size_t
#include <string>         // (reserved for future to_string; included for API parity)

// ============================================================================
//  Bitset<N> -- a hand-rolled std::bitset (fixed-size packed bit sequence)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Bitset stores exactly N bits (N is a compile-time template parameter).
// Instead of N separate bools (1 byte each on most platforms), bits are packed
// into an array of unsigned long "words" — typically 64 bits per word on
// 64-bit Linux/macOS.  All single-bit operations reduce to one array index,
// one shift, and one bitwise mask on a machine word.
//
// THE STORAGE FIELDS (compile-time constants + runtime array)
// -----------------------------------------------------------
//     BITS_PER_WORD  -> bits in one unsigned long (usually 64)
//     NUM_WORDS      -> ceil(N / BITS_PER_WORD) words needed
//     words_         -> unsigned long[NUM_WORDS] holding all N bits
//
// BIT INDEXING — how logical bit i maps to a word
// -----------------------------------------------
//
//     word_index = i / BITS_PER_WORD     (which word)
//     bit_offset = i % BITS_PER_WORD     (position inside that word)
//     mask       = 1UL << bit_offset     (single-bit bitmask)
//
//     Example: N=10, BITS_PER_WORD=64  →  NUM_WORDS=1  (one word, 54 unused)
//
//     words_[0]  (64 bits, only [0..9] are logical bitset bits):
//     ┌────────────────────────────────────────────────────────────────┐
//     │ b9 b8 b7 b6 b5 b4 b3 b2 b1 b0 │ ... unused high bits ...      │
//     └────────────────────────────────────────────────────────────────┘
//       ↑                              ↑
//       bit 9: word=0, offset=9        bits 10..63 not part of Bitset<10>
//
//     Example: N=100, BITS_PER_WORD=64  →  NUM_WORDS=2
//
//     words_[0] : bits  0 .. 63
//     words_[1] : bits 64 .. 99  (+ 28 unused bits in high end of word 1)
//
//     bit i=70:
//         word = 70 / 64 = 1
//         bit  = 70 % 64 = 6
//         mask = 1 << 6
//         test: (words_[1] & mask) != 0
//
// SINGLE-BIT OPERATIONS (same word math for set / reset / test / flip)
// --------------------------------------------------------------------
//
//     SET bit i:     words_[w] |=  (1UL << b)
//     RESET bit i:   words_[w] &= ~(1UL << b)
//     FLIP bit i:    words_[w] ^=  (1UL << b)
//     TEST bit i:    (words_[w] & (1UL << b)) != 0
//
// WORD-LEVEL OPERATIONS (count, &, |, ^, ~)
// -----------------------------------------
// Bitwise ops loop over all NUM_WORDS — one machine instruction per word.
// count() walks each set bit in each word (shift-and-test loop).
// operator~ masks off unused high bits in the last word so N stays exact.
//
// Key characteristics:
// - Fixed size at compile time; stack allocation only
// - O(1) single-bit set/reset/test/flip; O(W) for W = NUM_WORDS bulk ops
// - Space: NUM_WORDS * sizeof(unsigned long) bytes (not N bytes)
// ============================================================================

/**
 * @brief Fixed-size sequence of N bits packed into unsigned long words.
 *
 * @tparam N Number of bits (compile-time constant).
 */
template<std::size_t N>
class Bitset {
private:
    static constexpr size_t BITS_PER_WORD = sizeof(unsigned long) * 8;  ///< Bits per machine word (typically 64)
    static constexpr size_t NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;  ///< Word count = ceil(N / word_bits)
    unsigned long words_[NUM_WORDS];  ///< Packed bit storage; words_[k] holds bits [k*W .. k*W+W-1]

public:
    /**
     * @brief Zero-initialize every word (all N bits clear).
     *
     *     words_[0..NUM_WORDS-1] = 0
     */
    Bitset() {
        for (size_t i = 0; i < NUM_WORDS; ++i) words_[i] = 0;
    }

    /**
     * @brief Construct with low bits of val loaded into words_[0].
     *
     * Higher words stay zero.  Bits above N in words_[0] are not masked here
     * (callers should not rely on out-of-range bits).
     */
    Bitset(unsigned long val) : Bitset() {
        if (NUM_WORDS > 0) words_[0] = val;
    }

    /**
     * @brief Set or clear bit at position pos.
     *
     * Indexing:
     *
     *     pos ──▶ word = pos / BITS_PER_WORD
     *             bit  = pos % BITS_PER_WORD
     *
     *     words_[word]:
     *         set:   |= (1UL << bit)
     *         reset: &= ~(1UL << bit)
     *
     * Out-of-range pos (pos >= N) is silently ignored (no-op), matching
     * std::bitset defensive behavior for single-bit mutators.
     *
     * @return *this for chaining (.set(0).set(3).flip(1)).
     */
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

    /** @brief Clear bit pos (set(pos, false)). */
    Bitset& reset(size_t pos) { return set(pos, false); }

    /**
     * @brief Clear all N bits in every word.
     *
     *     for each word: words_[i] = 0
     */
    Bitset& reset() {
        for (size_t i = 0; i < NUM_WORDS; ++i) words_[i] = 0;
        return *this;
    }

    /**
     * @brief Toggle bit pos with XOR.
     *
     *     words_[word] ^= (1UL << bit)
     *
     * 0→1 and 1→0 in one instruction; out-of-range pos is a no-op.
     */
    Bitset& flip(size_t pos) {
        if (pos >= N) return *this;
        size_t word = pos / BITS_PER_WORD;
        size_t bit = pos % BITS_PER_WORD;
        words_[word] ^= (1UL << bit);
        return *this;
    }

    /**
     * @brief Test whether bit pos is set.
     *
     *     return (words_[word] & (1UL << bit)) != 0
     *
     * Out-of-range pos returns false (safe read).
     */
    bool test(size_t pos) const {
        if (pos >= N) return false;
        size_t word = pos / BITS_PER_WORD;
        size_t bit = pos % BITS_PER_WORD;
        return (words_[word] & (1UL << bit)) != 0;
    }

    /** @brief Unchecked-style access; delegates to test() (const, no reference). */
    bool operator[](size_t pos) const { return test(pos); }

    /**
     * @brief Count number of 1-bits across all words.
     *
     * Per-word popcount via shift loop (not hardware popcnt intrinsic):
     *
     *     w = words_[i]
     *     while w:  count += w & 1;  w >>= 1
     *
     * Complexity O(W * BITS_PER_WORD) worst case; O(W) words touched.
     */
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

    /** @brief Compile-time constant N. */
    constexpr size_t size() const { return N; }

    /** @brief True iff every one of the N bits is set. */
    bool all() const { return count() == N; }

    /** @brief True iff at least one bit is set. */
    bool any() const { return count() > 0; }

    /** @brief True iff no bits are set. */
    bool none() const { return count() == 0; }

    /**
     * @brief Bitwise AND word-by-word.
     *
     *     result.words_[i] = words_[i] & other.words_[i]
     *
     * Intersection of two bit patterns (set bits in common).
     */
    Bitset operator&(const Bitset& other) const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = words_[i] & other.words_[i];
        }
        return result;
    }

    /**
     * @brief Bitwise OR word-by-word (union of set bits).
     */
    Bitset operator|(const Bitset& other) const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = words_[i] | other.words_[i];
        }
        return result;
    }

    /**
     * @brief Bitwise XOR word-by-word (symmetric difference).
     */
    Bitset operator^(const Bitset& other) const {
        Bitset result;
        for (size_t i = 0; i < NUM_WORDS; ++i) {
            result.words_[i] = words_[i] ^ other.words_[i];
        }
        return result;
    }

    /**
     * @brief Bitwise NOT with mask on the last partial word.
     *
     * When N is not a multiple of BITS_PER_WORD, high "padding" bits in the
     * last word must stay 0 — otherwise ~ would flip bits beyond N.
     *
     *     result.words_[i] = ~words_[i]
     *     if partial last word:
     *         result.words_[NUM_WORDS-1] &= (1UL << used_bits) - 1
     *
     *     Bitset<10>: only low 10 bits of words_[0] are meaningful after ~
     */
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
