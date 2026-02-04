# Bitset - Fixed-Size Bit Array

A C++ implementation of `std::bitset` - a container for efficiently storing and manipulating fixed-size sequences of bits.

## 📋 Overview

`Bitset<N>` is a fixed-size array of N bits, stored compactly and efficiently. It provides convenient operations for bit manipulation, including bitwise logical operations, individual bit access, and counting. Unlike `vector<bool>`, the size is fixed at compile-time.

## 🎯 Key Features

- **Compact Storage**: Multiple bits packed into single words (typically 64 bits)
- **Fast Operations**: Bitwise operations on entire words at once
- **Compile-time Size**: Size known at compile-time, no allocation
- **Rich API**: Set, reset, flip, test, count operations
- **Bitwise Operators**: AND, OR, XOR, NOT supported
- **Query Methods**: `all()`, `any()`, `none()` for easy checking

## 🏗️ Implementation Details

### Data Structure
```cpp
template<std::size_t N>
class Bitset {
private:
    static constexpr size_t BITS_PER_WORD = sizeof(unsigned long) * 8;
    static constexpr size_t NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;
    unsigned long words_[NUM_WORDS];  // Packed storage
};
```

### Storage Efficiency
- N=8: 1 word (64 bits), uses 8 bits → 12.5% utilized
- N=64: 1 word (64 bits), uses 64 bits → 100% utilized
- N=128: 2 words (128 bits), uses 128 bits → 100% utilized

### Complexity
- **Set/Reset/Flip**: O(1) - Direct bit manipulation
- **Test/Access**: O(1) - Direct lookup
- **Count**: O(W) where W = number of words (N / 64)
- **Bitwise ops**: O(W) - Word-by-word operations
- **Space**: ⌈N / 64⌉ × 8 bytes

## 📚 Core Operations

### Construction
```cpp
Bitset<8> bs1;              // All zeros: 00000000
Bitset<8> bs2(42);          // From integer: 00101010
Bitset<8> bs3(0b11110000);  // Binary literal: 11110000
```

### Setting Bits
```cpp
bs.set(0);           // Set bit 0 to 1
bs.set(3, true);     // Set bit 3 to 1
bs.set(5, false);    // Set bit 5 to 0
bs.reset(2);         // Clear bit 2
bs.flip(7);          // Toggle bit 7
bs.reset();          // Clear all bits
```

### Testing Bits
```cpp
bool bit = bs.test(3);       // Safe test
bool bit = bs[3];            // Array-style access
```

### Querying
```cpp
size_t n = bs.count();       // Number of 1s
bool full = bs.all();        // All bits set?
bool some = bs.any();        // At least one bit set?
bool empty = bs.none();      // No bits set?
size_t sz = bs.size();       // Total number of bits
```

### Bitwise Operations
```cpp
auto result1 = bs1 & bs2;    // AND
auto result2 = bs1 | bs2;    // OR
auto result3 = bs1 ^ bs2;    // XOR
auto result4 = ~bs1;         // NOT
```

## 🚀 Usage Examples

### Basic Bit Manipulation
```cpp
Bitset<8> flags;
flags.set(0);       // Set bit 0
flags.set(7);       // Set bit 7
// flags = 10000001

if (flags.test(0)) {
    std::cout << "Bit 0 is set\n";
}
```

### Flags and Permissions
```cpp
const size_t READ = 0;
const size_t WRITE = 1;
const size_t EXECUTE = 2;

Bitset<8> permissions;
permissions.set(READ).set(WRITE);  // Chaining

if (permissions[READ] && permissions[WRITE]) {
    // Can read and write
}
```

### Bitwise Logic
```cpp
Bitset<8> a(0b11110000);
Bitset<8> b(0b10101010);

auto intersection = a & b;  // 10100000
auto union_bits = a | b;    // 11111010
auto xor_bits = a ^ b;      // 01011010
auto complement = ~a;       // 00001111
```

### Masking
```cpp
Bitset<8> data(0b11011010);
Bitset<8> mask(0b11110000);  // Upper nibble mask

auto upper = data & mask;    // Extract upper 4 bits: 11010000
auto lower = data & ~mask;   // Extract lower 4 bits: 00001010
```

### Set Operations
```cpp
// Think of bits as set membership
Bitset<8> set_a(0b00001111);  // Elements {0, 1, 2, 3}
Bitset<8> set_b(0b00110011);  // Elements {0, 1, 4, 5}

auto union_ab = set_a | set_b;        // A ∪ B
auto intersection = set_a & set_b;    // A ∩ B
auto difference = set_a & ~set_b;     // A - B
auto symmetric = set_a ^ set_b;       // A △ B
```

## 🔍 When to Use

### Use Bitset When:
- ✅ Need compact boolean array
- ✅ Size known at compile-time
- ✅ Frequent bitwise operations
- ✅ Implementing flags/permissions
- ✅ Set operations on fixed universe
- ✅ Bit masks and filters

### Use Other Containers When:
- ❌ Size needs to change → Use `vector<bool>` or custom
- ❌ Need individual addressability → Use `vector<bool>`
- ❌ Very large, sparse sets → Use hash set
- ❌ Need efficient iteration over set bits → Use specialized structure

## 🆚 Comparison with Alternatives

| Feature | Bitset<N> | vector<bool> | bool[N] | int flags |
|---------|-----------|--------------|---------|-----------|
| **Size** | Compile-time | Runtime | Compile-time | Fixed |
| **Packed** | ✅ Yes | ✅ Yes | ❌ No | ⚠️ Manual |
| **Operations** | Rich API | Limited | Manual | Manual |
| **Space** | N/8 bytes | N/8 bytes | N bytes | 4-8 bytes |
| **Type Safe** | ✅ Yes | ✅ Yes | ✅ Yes | ❌ No |

## 🎓 Special Features

### Chaining Operations
```cpp
Bitset<8> bs;
bs.set(0).set(2).set(4).flip(1);  // Chain multiple ops
```

### Word-level Efficiency
```cpp
// For Bitset<64>, operations work on single 64-bit word
Bitset<64> a, b;
auto result = a & b;  // Single AND instruction!
```

### Compile-time Optimization
```cpp
// Compiler knows exact size, can optimize aggressively
constexpr size_t N = 64;
Bitset<N> bs;  // Size baked into type
```

## 💡 Best Practices

1. **Use Meaningful Constants**: Define named constants for bit positions
2. **Chain Operations**: Use `.set().set()` for readability
3. **Prefer test() for Safety**: Use `test()` over `[]` for bounds checking
4. **Use Bitwise Ops**: Leverage `&`, `|`, `^` for complex logic
5. **Size Appropriately**: Round up to multiples of 64 for efficiency

## 📊 Common Use Cases

### 1. Feature Flags
```cpp
enum Feature { DARK_MODE = 0, NOTIFICATIONS = 1, ANALYTICS = 2 };
Bitset<8> enabled_features;
enabled_features.set(DARK_MODE).set(NOTIFICATIONS);
```

### 2. Game State
```cpp
Bitset<256> level_completed;  // 256 levels
level_completed.set(42);      // Beat level 42
if (level_completed.all()) {
    unlock_secret();
}
```

### 3. Bloom Filter
```cpp
Bitset<1024> bloom;
// Insert: set multiple hash positions
bloom.set(hash1(item)).set(hash2(item)).set(hash3(item));
// Query: check all hash positions
bool maybe_present = bloom[hash1(item)] && bloom[hash2(item)] && bloom[hash3(item)];
```

### 4. Graph Adjacency
```cpp
template<size_t N>
using AdjMatrix = std::array<Bitset<N>, N>;

AdjMatrix<100> graph;
graph[5].set(10);  // Edge from vertex 5 to 10
```

### 5. Sudoku Solver
```cpp
Bitset<9> row_used;     // Which digits used in row
Bitset<9> col_used;     // Which digits used in col
Bitset<9> box_used;     // Which digits used in box
auto available = ~(row_used | col_used | box_used);
```

## 🔗 API Reference

| Method | Description | Time |
|--------|-------------|------|
| `Bitset()` | Construct all zeros | O(W) |
| `Bitset(val)` | Construct from integer | O(1) |
| `set(pos)` | Set bit to 1 | O(1) |
| `reset(pos)` | Clear bit to 0 | O(1) |
| `flip(pos)` | Toggle bit | O(1) |
| `test(pos)` | Test bit value | O(1) |
| `operator[]` | Access bit | O(1) |
| `count()` | Count set bits | O(W) |
| `all()` | All bits set? | O(W) |
| `any()` | Any bit set? | O(W) |
| `none()` | No bits set? | O(W) |
| `size()` | Number of bits | O(1) |
| `operator&` | Bitwise AND | O(W) |
| `operator\|` | Bitwise OR | O(W) |
| `operator^` | Bitwise XOR | O(W) |
| `operator~` | Bitwise NOT | O(W) |

*W = number of words = ⌈N / 64⌉*

## 🏃 Building and Running

```bash
# Compile example
make bitset-example

# Run example
make run-bitset

# Run tests
make test-bitset
```

## 🌟 Advantages

- **Memory Efficient**: 8x more compact than `bool[]`
- **Fast**: Word-level operations, cache-friendly
- **Type Safe**: Size checked at compile-time
- **Expressive**: Rich API for common patterns
- **Zero Overhead**: No runtime indirection

## 📖 See Also

- **Array**: For non-boolean fixed-size arrays
- **Vector**: For dynamic-size containers
- **Set**: For sparse bit sets with large universe

