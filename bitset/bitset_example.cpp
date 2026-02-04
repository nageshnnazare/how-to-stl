#include "bitset.hpp"
#include <iostream>
#include <iomanip>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

template<size_t N>
void print_bitset(const Bitset<N>& bs, const char* label = "") {
    if (label[0] != '\0') {
        std::cout << label << ": ";
    }
    for (int i = N - 1; i >= 0; --i) {
        std::cout << (bs[i] ? '1' : '0');
        if (i > 0 && i % 8 == 0) std::cout << " ";
    }
    std::cout << "\n";
}

// Example 1: Basic Construction and Setting
void example_basic() {
    print_divider("Basic Construction and Setting");
    
    Bitset<8> bs1;              // All zeros
    print_bitset(bs1, "Default");
    
    Bitset<8> bs2(0b10101010);  // Binary literal
    print_bitset(bs2, "Binary");
    
    Bitset<8> bs3(255);         // All ones (for 8 bits)
    print_bitset(bs3, "All ones");
    
    // Set individual bits
    bs1.set(0);
    bs1.set(2);
    bs1.set(7);
    print_bitset(bs1, "After set");
    
    std::cout << "Count: " << bs1.count() << " bits set\n";
}

// Example 2: Bit Manipulation
void example_bit_manipulation() {
    print_divider("Bit Manipulation");
    
    Bitset<8> bs(0b00110011);
    print_bitset(bs, "Original");
    
    bs.set(0, false);  // Clear bit 0
    print_bitset(bs, "After set(0, false)");
    
    bs.reset(1);       // Clear bit 1
    print_bitset(bs, "After reset(1)");
    
    bs.flip(7);        // Flip bit 7
    print_bitset(bs, "After flip(7)");
    
    // Test individual bits
    std::cout << "Bit 5 is: " << (bs.test(5) ? "set" : "clear") << "\n";
    std::cout << "Bit 0 is: " << (bs.test(0) ? "set" : "clear") << "\n";
}

// Example 3: Bitwise Operations
void example_bitwise_operations() {
    print_divider("Bitwise Operations");
    
    Bitset<8> bs1(0b11110000);
    Bitset<8> bs2(0b10101010);
    
    print_bitset(bs1, "bs1");
    print_bitset(bs2, "bs2");
    
    auto and_result = bs1 & bs2;
    print_bitset(and_result, "bs1 & bs2");
    
    auto or_result = bs1 | bs2;
    print_bitset(or_result, "bs1 | bs2");
    
    auto xor_result = bs1 ^ bs2;
    print_bitset(xor_result, "bs1 ^ bs2");
    
    auto not_result = ~bs1;
    print_bitset(not_result, "~bs1");
}

// Example 4: Querying State
void example_querying() {
    print_divider("Querying State");
    
    Bitset<8> bs1(0);
    Bitset<8> bs2(255);
    Bitset<8> bs3(0b00001000);
    
    print_bitset(bs1, "bs1");
    std::cout << "  all(): " << bs1.all() << ", any(): " << bs1.any() << ", none(): " << bs1.none() << "\n";
    
    print_bitset(bs2, "bs2");
    std::cout << "  all(): " << bs2.all() << ", any(): " << bs2.any() << ", none(): " << bs2.none() << "\n";
    
    print_bitset(bs3, "bs3");
    std::cout << "  all(): " << bs3.all() << ", any(): " << bs3.any() << ", none(): " << bs3.none() << "\n";
    std::cout << "  count(): " << bs3.count() << " / " << bs3.size() << "\n";
}

// Example 5: Flags and Permissions
void example_flags() {
    print_divider("Flags and Permissions");
    
    // File permissions example
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
    
    print_bitset(permissions, "Bits");
}

// Example 6: Masking and Filtering
void example_masking() {
    print_divider("Masking and Filtering");
    
    Bitset<8> data(0b11011010);
    Bitset<8> mask(0b11110000);  // Mask for upper 4 bits
    
    print_bitset(data, "Data");
    print_bitset(mask, "Mask");
    
    auto masked = data & mask;
    print_bitset(masked, "Data & Mask (extract upper)");
    
    auto inverted_mask = ~mask;
    auto lower = data & inverted_mask;
    print_bitset(lower, "Data & ~Mask (extract lower)");
}

// Example 7: Set Operations
void example_set_operations() {
    print_divider("Set Operations");
    
    // Think of bits as set membership
    Bitset<8> set_a(0b00001111);  // Elements {0, 1, 2, 3}
    Bitset<8> set_b(0b00110011);  // Elements {0, 1, 4, 5}
    
    print_bitset(set_a, "Set A");
    print_bitset(set_b, "Set B");
    
    auto union_ab = set_a | set_b;
    print_bitset(union_ab, "A ∪ B (union)");
    
    auto intersection = set_a & set_b;
    print_bitset(intersection, "A ∩ B (intersection)");
    
    auto symmetric_diff = set_a ^ set_b;
    print_bitset(symmetric_diff, "A △ B (symmetric diff)");
}

// Example 8: Large Bitsets
void example_large_bitsets() {
    print_divider("Large Bitsets");
    
    Bitset<64> bs;
    
    // Set some bits
    bs.set(0).set(15).set(31).set(47).set(63);
    
    std::cout << "64-bit bitset with 5 bits set\n";
    std::cout << "Count: " << bs.count() << " / " << bs.size() << "\n";
    std::cout << "Positions set: 0, 15, 31, 47, 63\n";
    
    // Print in chunks
    std::cout << "Binary (MSB first):\n  ";
    for (int i = 63; i >= 0; --i) {
        std::cout << (bs[i] ? '1' : '0');
        if (i > 0 && i % 8 == 0) std::cout << " ";
        if (i > 0 && i % 32 == 0) std::cout << "\n  ";
    }
    std::cout << "\n";
}

// Example 9: Bloom Filter Simulation
void example_bloom_filter() {
    print_divider("Bloom Filter Simulation");
    
    Bitset<32> bloom;
    
    // Simple hash functions (just for demonstration)
    auto hash1 = [](int val) { return val % 32; };
    auto hash2 = [](int val) { return (val * 17) % 32; };
    auto hash3 = [](int val) { return (val * 31) % 32; };
    
    // Insert element 42
    bloom.set(hash1(42)).set(hash2(42)).set(hash3(42));
    std::cout << "Inserted 42\n";
    
    // Check if 42 might be in set
    bool might_contain_42 = bloom[hash1(42)] && bloom[hash2(42)] && bloom[hash3(42)];
    std::cout << "Might contain 42: " << (might_contain_42 ? "yes" : "no") << "\n";
    
    // Check if 99 might be in set
    bool might_contain_99 = bloom[hash1(99)] && bloom[hash2(99)] && bloom[hash3(99)];
    std::cout << "Might contain 99: " << (might_contain_99 ? "yes" : "no") << "\n";
    
    std::cout << "Bloom filter bits: " << bloom.count() << " / " << bloom.size() << " set\n";
}

// Example 10: Reset All
void example_reset_all() {
    print_divider("Reset All");
    
    Bitset<8> bs(0b11111111);
    print_bitset(bs, "Before reset");
    std::cout << "Count: " << bs.count() << "\n";
    
    bs.reset();
    print_bitset(bs, "After reset()");
    std::cout << "Count: " << bs.count() << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║         Bitset Container Examples              ║\n";
    std::cout << "║      Fixed-size Bit Array (std::bitset)       ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_basic();
    example_bit_manipulation();
    example_bitwise_operations();
    example_querying();
    example_flags();
    example_masking();
    example_set_operations();
    example_large_bitsets();
    example_bloom_filter();
    example_reset_all();
    
    std::cout << "\n✅ All Bitset examples completed successfully!\n";
    return 0;
}

