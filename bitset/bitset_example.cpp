// bitset_example.cpp — runnable tour of Bitset<N> (packed fixed-size bits)
//
// Demonstrates: word packing, bit index → (word, offset, mask), set/reset/flip/test,
// count/all/any/none, bitwise ops, flags, masking, and multi-word layouts.
// Build from repo root:
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -I. bitset/bitset_example.cpp -o /tmp/x_bitset

#include "bitset/bitset.hpp"
#include <iostream>
#include <iomanip>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Print bits MSB-first (index N-1 down to 0) with optional byte spacing
template<size_t N>
void print_bitset(const Bitset<N>& bs, const char* label = "") {
    if (label[0] != '\0') {
        std::cout << label << ": ";
    }
    for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
        std::cout << (bs[static_cast<size_t>(i)] ? '1' : '0');
        if (i > 0 && i % 8 == 0) std::cout << " ";
    }
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Example 1: Construction — all-zero default and val in words_[0]
// ---------------------------------------------------------------------------
void example_basic() {
    print_divider("Basic Construction and Setting");

    Bitset<8> bs1;
    print_bitset(bs1, "Default (all 0)");

    Bitset<8> bs2(0b10101010);
    print_bitset(bs2, "From literal 0b10101010");

    Bitset<8> bs3(255);
    print_bitset(bs3, "From 255 (all 8 bits set)");

    bs1.set(0).set(2).set(7);
    print_bitset(bs1, "After set(0), set(2), set(7)");

    std::cout << "count(): " << bs1.count() << " bits set\n";
    std::cout << "size():  " << bs1.size() << " total bits\n";
}

// ---------------------------------------------------------------------------
// Example 2: set / reset / flip — word = pos/64, bit = pos%64
// ---------------------------------------------------------------------------
void example_bit_manipulation() {
    print_divider("Bit Manipulation");

    Bitset<8> bs(0b00110011);
    print_bitset(bs, "Original");

    bs.set(0, false);
    print_bitset(bs, "set(0, false)");

    bs.reset(1);
    print_bitset(bs, "reset(1)");

    bs.flip(7);
    print_bitset(bs, "flip(7)");

    std::cout << "test(5): " << (bs.test(5) ? "1" : "0") << "\n";
    std::cout << "test(0): " << (bs.test(0) ? "1" : "0") << "\n";
}

// ---------------------------------------------------------------------------
// Example 3: Word-level bitwise AND, OR, XOR, NOT
// ---------------------------------------------------------------------------
void example_bitwise_operations() {
    print_divider("Bitwise Operations");

    Bitset<8> bs1(0b11110000);
    Bitset<8> bs2(0b10101010);

    print_bitset(bs1, "bs1");
    print_bitset(bs2, "bs2");

    print_bitset(bs1 & bs2, "bs1 & bs2");
    print_bitset(bs1 | bs2, "bs1 | bs2");
    print_bitset(bs1 ^ bs2, "bs1 ^ bs2");
    print_bitset(~bs1, "~bs1 (masked to N=8)");
}

// ---------------------------------------------------------------------------
// Example 4: all / any / none via count()
// ---------------------------------------------------------------------------
void example_querying() {
    print_divider("Querying State");

    Bitset<8> empty;
    Bitset<8> full(255);
    Bitset<8> one_bit(0b00001000);

    print_bitset(empty, "empty");
    std::cout << "  all=" << empty.all() << " any=" << empty.any()
              << " none=" << empty.none() << "\n";

    print_bitset(full, "full");
    std::cout << "  all=" << full.all() << " count=" << full.count() << "\n";

    print_bitset(one_bit, "one bit");
    std::cout << "  count=" << one_bit.count() << " / " << one_bit.size() << "\n";
}

// ---------------------------------------------------------------------------
// Example 5: Named flag positions — chaining set()
// ---------------------------------------------------------------------------
void example_flags() {
    print_divider("Flags and Permissions");

    const size_t READ = 0;
    const size_t WRITE = 1;
    const size_t EXECUTE = 2;
    const size_t OWNER = 3;
    const size_t GROUP = 4;
    const size_t OTHERS = 5;

    Bitset<8> permissions;
    permissions.set(READ).set(WRITE).set(OWNER);

    std::cout << "File permissions:\n";
    std::cout << "  Read:    " << (permissions[READ] ? "yes" : "no") << "\n";
    std::cout << "  Write:   " << (permissions[WRITE] ? "yes" : "no") << "\n";
    std::cout << "  Execute: " << (permissions[EXECUTE] ? "yes" : "no") << "\n";
    std::cout << "  Owner:   " << (permissions[OWNER] ? "yes" : "no") << "\n";
    std::cout << "  Group:   " << (permissions[GROUP] ? "yes" : "no") << "\n";
    std::cout << "  Others:  " << (permissions[OTHERS] ? "yes" : "no") << "\n";

    print_bitset(permissions, "Raw bits");
}

// ---------------------------------------------------------------------------
// Example 6: Masking — extract nibbles with & and ~
// ---------------------------------------------------------------------------
void example_masking() {
    print_divider("Masking and Filtering");

    Bitset<8> data(0b11011010);
    Bitset<8> mask(0b11110000);

    print_bitset(data, "Data");
    print_bitset(mask, "Upper nibble mask");

    print_bitset(data & mask, "Upper nibble");
    print_bitset(data & ~mask, "Lower nibble");
}

// ---------------------------------------------------------------------------
// Example 7: Set algebra — bits as set membership
// ---------------------------------------------------------------------------
void example_set_operations() {
    print_divider("Set Operations");

    Bitset<8> set_a(0b00001111);
    Bitset<8> set_b(0b00110011);

    print_bitset(set_a, "Set A");
    print_bitset(set_b, "Set B");

    print_bitset(set_a | set_b, "Union");
    print_bitset(set_a & set_b, "Intersection");
    print_bitset(set_a ^ set_b, "Symmetric difference");
}

// ---------------------------------------------------------------------------
// Example 8: N=64 — single word on typical LP64 platforms
// ---------------------------------------------------------------------------
void example_large_bitsets() {
    print_divider("64-Bit Single Word");

    Bitset<64> bs;
    bs.set(0).set(15).set(31).set(47).set(63);

    std::cout << "Bitset<64>: " << bs.count() << " bits set at corners\n";
    std::cout << "Indices: 0, 15, 31, 47, 63 (word=index/64, bit=index%64)\n";

    std::cout << "MSB-first (truncated display):\n  ";
    for (int i = 63; i >= 0; --i) {
        std::cout << (bs[static_cast<size_t>(i)] ? '1' : '0');
        if (i > 0 && i % 8 == 0) std::cout << " ";
        if (i > 0 && i % 32 == 0) std::cout << "\n  ";
    }
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Example 9: Multi-word — Bitset<100> uses ceil(100/64)=2 words
// ---------------------------------------------------------------------------
void example_multi_word() {
    print_divider("Multi-Word Layout (N=100)");

    Bitset<100> bs;
    bs.set(0);    // word 0, bit 0
    bs.set(63);   // word 0, bit 63
    bs.set(64);   // word 1, bit 0  (64/64=1, 64%64=0)
    bs.set(99);   // word 1, bit 35 (99/64=1, 99%64=35)

    std::cout << "Set bits at 0, 63, 64, 99\n";
    std::cout << "count=" << bs.count() << "\n";
    std::cout << "test(63)=" << bs.test(63) << " test(64)=" << bs.test(64) << "\n";
}

// ---------------------------------------------------------------------------
// Example 10: Bloom filter sketch — several hash positions per key
// ---------------------------------------------------------------------------
void example_bloom_filter() {
    print_divider("Bloom Filter Simulation");

    Bitset<32> bloom;

    auto hash1 = [](int val) { return static_cast<size_t>(val % 32); };
    auto hash2 = [](int val) { return static_cast<size_t>((val * 17) % 32); };
    auto hash3 = [](int val) { return static_cast<size_t>((val * 31) % 32); };

    bloom.set(hash1(42)).set(hash2(42)).set(hash3(42));
    std::cout << "Inserted key 42 at 3 bit positions\n";

    bool maybe_42 = bloom.test(hash1(42)) && bloom.test(hash2(42)) && bloom.test(hash3(42));
    bool maybe_99 = bloom.test(hash1(99)) && bloom.test(hash2(99)) && bloom.test(hash3(99));

    std::cout << "Maybe contains 42: " << (maybe_42 ? "yes" : "no") << "\n";
    std::cout << "Maybe contains 99: " << (maybe_99 ? "yes" : "no") << " (likely false positive-free here)\n";
    std::cout << "Bits set: " << bloom.count() << " / " << bloom.size() << "\n";
}

// ---------------------------------------------------------------------------
// Example 11: reset() clears every word
// ---------------------------------------------------------------------------
void example_reset_all() {
    print_divider("Reset All");

    Bitset<8> bs(0b11111111);
    print_bitset(bs, "Before reset");
    std::cout << "count=" << bs.count() << "\n";

    bs.reset();
    print_bitset(bs, "After reset()");
    std::cout << "count=" << bs.count() << " none=" << bs.none() << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║         Bitset Container Examples              ║\n";
    std::cout << "║   N bits packed into unsigned long words       ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    example_basic();
    example_bit_manipulation();
    example_bitwise_operations();
    example_querying();
    example_flags();
    example_masking();
    example_set_operations();
    example_large_bitsets();
    example_multi_word();
    example_bloom_filter();
    example_reset_all();

    std::cout << "\n✅ All Bitset examples completed successfully!\n";
    return 0;
}

/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║         Bitset Container Examples              ║
║   N bits packed into unsigned long words       ║
╚════════════════════════════════════════════════╝

=== Basic Construction and Setting ===
Default (all 0): 00000000
From literal 0b10101010: 10101010
From 255 (all 8 bits set): 11111111
After set(0), set(2), set(7): 10000101
count(): 3 bits set
size():  8 total bits

=== Bit Manipulation ===
Original: 00110011
set(0, false): 00110010
reset(1): 00110000
flip(7): 10110000
test(5): 1
test(0): 0

=== Bitwise Operations ===
bs1: 11110000
bs2: 10101010
bs1 & bs2: 10100000
bs1 | bs2: 11111010
bs1 ^ bs2: 01011010
~bs1 (masked to N=8): 00001111

=== Querying State ===
empty: 00000000
  all=0 any=0 none=1
full: 11111111
  all=1 count=8
one bit: 00001000
  count=1 / 8

=== Flags and Permissions ===
File permissions:
  Read:    yes
  Write:   yes
  Execute: no
  Owner:   yes
  Group:   no
  Others:  no
Raw bits: 00001011

=== Masking and Filtering ===
Data: 11011010
Upper nibble mask: 11110000
Upper nibble: 11010000
Lower nibble: 00001010

=== Set Operations ===
Set A: 00001111
Set B: 00110011
Union: 00111111
Intersection: 00000011
Symmetric difference: 00111100

=== 64-Bit Single Word ===
Bitset<64>: 5 bits set at corners
Indices: 0, 15, 31, 47, 63 (word=index/64, bit=index%64)
MSB-first (truncated display):
  10000000 00000000 10000000 00000000 
  10000000 00000000 10000000 00000001

=== Multi-Word Layout (N=100) ===
Set bits at 0, 63, 64, 99
count=4
test(63)=1 test(64)=1

=== Bloom Filter Simulation ===
Inserted key 42 at 3 bit positions
Maybe contains 42: yes
Maybe contains 99: no (likely false positive-free here)
Bits set: 2 / 32

=== Reset All ===
Before reset: 11111111
count=8
After reset(): 00000000
count=0 none=1

✅ All Bitset examples completed successfully!
 * ============================================================================ */
