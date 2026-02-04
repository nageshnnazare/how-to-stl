# C++ Container Implementations & Advanced Systems Programming

A comprehensive, production-quality implementation of **20 essential C++ containers** plus **4 advanced systems programming components** (Memory Allocators, Locks, Thread Pool, Arena Allocator) with extensive documentation, examples, and comprehensive test coverage.

## 🚀 Quick Start

### Build Everything
```bash
make all
```

### Run Examples
```bash
make run-unique       # Run unique_ptr examples
make run-shared       # Run shared_ptr examples
make run-string       # Run String examples
make run-vector       # Run Vector examples
make run-deque        # Run Deque examples
make run-list         # Run List examples
make run-array        # Run Array examples
make run-set          # Run Set examples
make run-map          # Run Map examples
make run-stack        # Run Stack examples
make run-queue        # Run Queue examples
make run-optional     # Run Optional examples
make run-bitset       # Run Bitset examples
make run-allocator    # Run Memory Allocator examples
make run-locks        # Run Locks examples
make run-arena-allocator  # Run Arena Allocator examples
make run-thread-pool  # Run Thread Pool examples
make run-all          # Run everything
```

### Run Tests
```bash
make test             # Run all test suites
make test-unique      # Test unique_ptr
make test-shared      # Test shared_ptr
make test-string      # Test String
make test-allocator   # Test Memory Allocator
make test-locks       # Test Locks
make test-arena-allocator  # Test Arena Allocator
make test-thread-pool # Test Thread Pool
make test-advanced    # Test all advanced systems
# ... and more
```

### Check for Memory Leaks
```bash
make valgrind-unique  # Check unique_ptr
make valgrind-shared  # Check shared_ptr
```

## 📚 What's Included

### Smart Pointers (2)

#### unique_ptr (Exclusive Ownership)
- **Size**: 8 bytes (zero overhead!)
- **Features**: Move-only, array support, custom deleters
- **Use Case**: Clear single owner
- **Thread-Safe**: Individual instances (not shared)

#### shared_ptr (Shared Ownership)
- **Size**: 16 bytes + control block
- **Features**: Reference counting, weak_ptr, thread-safe counting
- **Use Case**: Multiple/unclear owners
- **Thread-Safe**: Reference count operations (atomic)

### Sequence Containers (5)

#### String (Dynamic String)
- **Size**: 40 bytes (with SSO buffer)
- **Features**: Small String Optimization, full std::string API
- **Use Case**: Text manipulation
- **Optimization**: No heap allocation for strings ≤ 15 chars

#### Vector (Dynamic Array)
- **Size**: 24 bytes (ptr, size, capacity)
- **Features**: Geometric growth, random access, full std::vector API
- **Use Case**: Dynamic array of elements
- **Growth**: Doubles capacity when full (amortized O(1) push)

#### Deque (Double-Ended Queue)
- **Size**: Variable (chunk-based)
- **Features**: O(1) push/pop at both ends, O(1) random access
- **Use Case**: Queue, buffer, sliding window
- **Storage**: Chunk-based, no reallocation needed

#### List (Doubly-Linked List)
- **Size**: Variable (node-based)
- **Features**: O(1) insertion/deletion at any position
- **Use Case**: Frequent insertions/deletions
- **Storage**: Nodes with prev/next pointers

#### Array (Fixed-Size Array)
- **Size**: N × sizeof(T) (stack allocated)
- **Features**: Compile-time size, zero overhead, STL compatible
- **Use Case**: Fixed-size collections
- **Storage**: Stack array wrapper

### Associative Containers - Ordered (4)

#### Set (Red-Black Tree)
- **Features**: Unique sorted elements, O(log n) operations
- **Use Case**: Ordered unique values, range queries
- **Implementation**: Full Red-Black Tree with rotations

#### Map (Red-Black Tree)
- **Features**: Key-value pairs, O(log n) operations
- **Use Case**: Ordered mappings, range queries
- **Implementation**: Full Red-Black Tree

#### Multiset (Sorted Vector)
- **Features**: Sorted with duplicates, O(log n) find
- **Use Case**: Sorted collections with duplicates
- **Implementation**: Sorted vector

#### Multimap (Sorted Vector)
- **Features**: Key-value with duplicate keys
- **Use Case**: One-to-many mappings
- **Implementation**: Sorted vector of pairs

### Associative Containers - Unordered (2)

#### UnorderedSet (Hash Table)
- **Features**: O(1) average operations, auto-rehashing
- **Use Case**: Fast lookups, no ordering needed
- **Implementation**: Separate chaining

#### UnorderedMap (Hash Table)
- **Features**: O(1) average key-value operations
- **Use Case**: Caching, fast lookups
- **Implementation**: Separate chaining

### Container Adapters (3)

#### Stack (LIFO)
- **Features**: Last-In-First-Out, push/pop/top
- **Use Case**: DFS, expression evaluation, undo
- **Implementation**: Adapter (uses deque)

#### Queue (FIFO)
- **Features**: First-In-First-Out, push/pop/front
- **Use Case**: BFS, task queue, buffering
- **Implementation**: Adapter (uses deque)

#### PriorityQueue (Binary Heap)
- **Features**: Max-heap, O(log n) push/pop
- **Use Case**: Dijkstra's, scheduling, top-K
- **Implementation**: Binary heap with heapify

### Utility Types (4)

#### Pair (Two-Element Tuple)
- **Features**: Aggregate type, make_pair
- **Use Case**: Key-value, return multiple values
- **Implementation**: Simple aggregate

#### Tuple (N-Element Tuple)
- **Features**: Variadic template, any number of elements
- **Use Case**: Multiple return values, generic programming
- **Implementation**: Recursive variadic template

#### Optional (Maybe-Value Type)
- **Features**: Type-safe null, value_or(), C++17 style
- **Use Case**: Optional values, safe lookups
- **Implementation**: Placement new, aligned storage

#### Bitset (Fixed-Size Bit Array)
- **Features**: Compact storage, bitwise operations
- **Use Case**: Flags, permissions, bloom filters
- **Implementation**: Word-packed bit storage

### Reading Order

**Beginner:**
1. `unique_ptr/README.md` - Start here
2. `unique_ptr/unique_ptr_example.cpp` - Study examples
3. `string/README.md` - Learn string handling
4. `vector/README.md` - Learn dynamic arrays
5. `array/README.md` - Learn fixed arrays

**Intermediate:**
6. `shared_ptr/README.md` - Learn shared ownership
7. `list/README.md` - Learn linked structures
8. `deque/README.md` - Learn chunk-based storage
9. `optional/README.md` - Learn type-safe nulls
10. `docs/QUICK_REFERENCE.md` - Practical patterns

**Advanced:**
11. `set/README.md` - Learn Red-Black Trees
12. `map/README.md` - Learn tree-based maps
13. `unordered_map/README.md` - Learn hash tables
14. `priority_queue/README.md` - Learn binary heaps
15. `docs/IMPLEMENTATION_NOTES.md` - Deep dive
16. Study all .hpp implementations

## 🎯 Key Features

### unique_ptr
```cpp
auto ptr = makeUnique<MyClass>(args...);
ptr->method();
// Automatically deleted - no memory leaks!
```

**Features:**
- RAII (Resource Acquisition Is Initialization)
- Move semantics (no copying)
- Zero runtime overhead
- Array support with `operator[]`
- Custom deleters

### shared_ptr
```cpp
SharedPtr<MyClass> ptr1 = makeShared<MyClass>();
SharedPtr<MyClass> ptr2 = ptr1;  // Share ownership
std::cout << ptr1.use_count();   // 2
```

**Features:**
- Shared ownership (reference counting)
- Thread-safe counting (atomic operations)
- weak_ptr for breaking circular references
- Automatic cleanup when last owner destroyed
- Polymorphism and dynamic casting

### Additional Container Features

**String**: Small String Optimization (SSO)
**Vector**: Geometric growth strategy
**Deque**: Chunk-based, stable references
**List**: Bidirectional iteration, O(1) splice
**Set/Map**: Red-Black Tree balancing
**UnorderedSet/Map**: Dynamic rehashing
**PriorityQueue**: Floyd's heap-building
**Optional**: Placement new, aligned storage
**Bitset**: Word-level operations

## 🔧 Build System

### Makefile Targets

**Building:**
- `make all` - Build everything
- `make debug` - Build with debug symbols
- `make clean` - Remove build artifacts

**Running:**
- `make run-unique` - Run unique_ptr examples
- `make run-shared` - Run shared_ptr examples
- `make run-string` - Run String examples
- `make run-vector` - Run Vector examples
- `make run-deque` - Run Deque examples
- `make run-list` - Run List examples
- `make run-array` - Run Array examples
- `make run-optional` - Run Optional examples
- `make run-bitset` - Run Bitset examples
- `make test` - Run all test suites
- `make run-all` - Run all examples and tests

**Memory Checking:**
- `make valgrind-unique` - Memory leak check
- `make valgrind-shared` - Memory leak check
- `make valgrind-test` - Test leak check

**Utilities:**
- `make format` - Format code (needs clang-format)
- `make tree` - Show directory structure
- `make sizes` - Show file sizes
- `make help` - Show all targets

## 💡 Common Patterns

### Factory Pattern (unique_ptr)
```cpp
UniquePtr<Widget> createWidget(WidgetType type) {
    switch (type) {
        case TYPE_A: return makeUnique<WidgetA>();
        case TYPE_B: return makeUnique<WidgetB>();
    }
}
```

### Cache Pattern (shared_ptr + weak_ptr)
```cpp
std::map<std::string, WeakPtr<Resource>> cache;

SharedPtr<Resource> getResource(const std::string& key) {
    auto locked = cache[key].lock();
    if (!locked) {
        locked = makeShared<Resource>(key);
        cache[key] = locked;
    }
    return locked;
}
```

### Safe Lookup (Optional)
```cpp
Optional<User> findUser(int id) {
    if (/* found */) return Optional<User>(user);
    return Optional<User>();  // Empty
}

auto user = findUser(123);
std::string name = user ? user->name : "Guest";
```

### Flags (Bitset)
```cpp
Bitset<8> permissions;
permissions.set(READ).set(WRITE);
if (permissions[READ] && permissions[WRITE]) {
    // Can read and write
}
```

Run all tests:
```bash
make test
```

## 📈 Performance

### Smart Pointers
- **unique_ptr**: ~2 cycles construction, zero overhead
- **shared_ptr**: ~20 cycles construction, ~30 cycles atomic ops

### Sequences
- **String (SSO)**: ~5 cycles, no allocation for ≤15 chars
- **Vector**: O(1) amortized push_back, 2x growth
- **Deque**: O(1) both ends, no reallocation
- **List**: O(1) insertion/deletion, cache-unfriendly
- **Array**: Zero overhead, compile-time size

### Associative
- **Set/Map**: O(log n), Red-Black Tree
- **UnorderedSet/Map**: O(1) average, hash table

### Adapters & Utilities
- **Stack/Queue**: O(1) operations
- **PriorityQueue**: O(log n) push/pop
- **Optional**: O(1) all operations
- **Bitset**: O(1) bit operations, O(W) for counting

## 🎓 Educational Value

This implementation teaches:
- Modern C++ (C++11/14/17)
- RAII and resource management
- Move semantics and perfect forwarding
- Template metaprogramming
- Variadic templates
- Atomic operations and thread safety
- Memory management patterns
- Red-Black Tree balancing
- Hash table implementation
- Binary heap algorithms
- Placement new and aligned storage
- Small String Optimization (SSO)
- Geometric growth strategies
- Iterator design patterns
- Container adapter pattern

## 📝 Standards Compliance

Implements features from:
- **C++11**: Core smart pointer functionality, move semantics
- **C++14**: make_unique, make_shared
- **C++17**: Compatible with modern STL, optional

## ⚡ Quick Examples

### unique_ptr Example
```cpp
#include "unique_ptr/unique_ptr.hpp"

auto ptr = makeUnique<std::string>("Hello");
std::cout << *ptr << "\n";
// Automatically deleted when ptr goes out of scope
```

### shared_ptr Example
```cpp
#include "shared_ptr/shared_ptr.hpp"

auto ptr1 = makeShared<int>(42);
auto ptr2 = ptr1;  // Share ownership
std::cout << ptr1.use_count();  // 2
```

### String Example
```cpp
#include "string/string.hpp"

String s = "Hello";
s += " World";  // SSO: no heap allocation!
std::cout << s << "\n";  // "Hello World"
```

### Vector Example
```cpp
#include "vector/vector.hpp"

Vector<int> vec = {1, 2, 3};
vec.push_back(4);
for (int x : vec) {
    std::cout << x << " ";
}
```

### Optional Example
```cpp
#include "optional/optional.hpp"

Optional<int> divide(int a, int b) {
    if (b == 0) return Optional<int>();
    return Optional<int>(a / b);
}

auto result = divide(10, 2);
if (result) {
    std::cout << "Result: " << *result << "\n";
}
```

### Bitset Example
```cpp
#include "bitset/bitset.hpp"

Bitset<8> flags;
flags.set(0).set(7);  // Set bits 0 and 7
std::cout << "Count: " << flags.count() << "\n";  // 2
```

