# Array - Fixed-Size Array Container

A C++ implementation of `std::array` - a container that encapsulates fixed-size arrays.

## 📋 Overview

`Array` is an aggregate type with the same semantics as a struct holding a C-style array `T[N]` as its only non-static data member. It combines the performance and accessibility of a C array with the benefits of a standard container, such as knowing its own size, supporting assignment, random access iterators, etc.

## 🎯 Key Features

- **Fixed Size**: Size determined at compile-time, no dynamic allocation
- **Stack Allocated**: Efficient memory usage, no heap overhead
- **Aggregate Type**: Can be initialized with aggregate initialization
- **STL Compatible**: Works with all STL algorithms and iterators
- **Zero Overhead**: No space or time overhead compared to raw arrays
- **Bounds Checking**: Safe element access via `at()` method

## 🏗️ Implementation Details

### Data Structure
```cpp
template<typename T, std::size_t N>
struct Array {
    T data_[N];  // Raw C array
};
```

### Complexity
- **Access**: O(1) - Direct array indexing
- **Iteration**: O(n) - Linear traversal
- **Space**: O(N) - Fixed size on stack

## 📚 Core Operations

### Construction & Initialization
```cpp
Array<int, 5> arr1 = {1, 2, 3, 4, 5};      // Full initialization
Array<int, 5> arr2 = {1, 2};                // Partial (rest zero-initialized)
Array<int, 5> arr3;                         // Uninitialized elements
arr3.fill(42);                              // Fill all elements
```

### Element Access
```cpp
arr[0];           // Unchecked access (fast)
arr.at(0);        // Bounds-checked access (safe)
arr.front();      // First element
arr.back();       // Last element
```

### Iteration
```cpp
for (auto& elem : arr) { }                  // Range-based for
for (auto it = arr.begin(); it != arr.end(); ++it) { }  // Iterators
```

## 🚀 Usage Examples

### Basic Usage
```cpp
Array<int, 5> scores = {95, 87, 92, 88, 90};
std::cout << "First score: " << scores.front() << "\n";
std::cout << "Average: " << std::accumulate(scores.begin(), scores.end(), 0) / 5.0 << "\n";
```

### With STL Algorithms
```cpp
Array<int, 6> nums = {3, 1, 4, 1, 5, 9};
std::sort(nums.begin(), nums.end());
auto it = std::find(nums.begin(), nums.end(), 5);
```

### Multi-dimensional Arrays
```cpp
Array<Array<int, 3>, 3> matrix = {{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
}};
std::cout << matrix[1][2];  // Access element at row 1, col 2
```

## 🔍 When to Use

### Use Array When:
- ✅ Size is known at compile-time
- ✅ You want stack allocation
- ✅ You need STL algorithm compatibility
- ✅ You want zero overhead abstraction
- ✅ You need bounds checking via `at()`

### Use Vector Instead When:
- ❌ Size needs to change at runtime
- ❌ Size is very large (stack overflow risk)
- ❌ You need dynamic memory allocation

## 🆚 Comparison with Alternatives

| Feature | Array | C Array | Vector |
|---------|-------|---------|--------|
| **Size** | Compile-time | Compile-time | Runtime |
| **Memory** | Stack | Stack | Heap |
| **Knows Size** | ✅ Yes | ❌ No | ✅ Yes |
| **STL Compatible** | ✅ Yes | ❌ No | ✅ Yes |
| **Bounds Checking** | ✅ Yes (at) | ❌ No | ✅ Yes (at) |
| **Dynamic Growth** | ❌ No | ❌ No | ✅ Yes |
| **Overhead** | None | None | Capacity tracking |

## 🎓 Special Features

### Zero-sized Arrays
```cpp
Array<int, 0> empty;
empty.empty();  // Returns true
empty.size();   // Returns 0
// Note: front(), back(), operator[] are undefined for N=0
```

### Aggregate Initialization
```cpp
Array<int, 3> arr = {1, 2, 3};  // Aggregate init
// Remaining elements zero-initialized if not specified
Array<int, 5> arr2 = {1, 2};    // {1, 2, 0, 0, 0}
```

## 💡 Best Practices

1. **Prefer `at()` for Safety**: Use `at()` when bounds checking is needed
2. **Use `fill()` for Initialization**: Cleaner than loops
3. **Range-based For**: Most readable iteration method
4. **Const Correctness**: Mark arrays `const` when not modifying
5. **Stack Size Awareness**: Be careful with large N values

## 📊 Example Use Cases

- **Fixed-size buffers**: `Array<char, 256> buffer;`
- **Coordinate systems**: `Array<double, 3> point;`  // x, y, z
- **Small lookup tables**: `Array<const char*, 7> days = {"Mon", "Tue", ...};`
- **Matrix operations**: `Array<Array<double, 3>, 3> matrix;`
- **Game boards**: `Array<Array<char, 8>, 8> chessboard;`

## 🔗 API Reference

| Method | Description | Time |
|--------|-------------|------|
| `operator[]` | Access element (unchecked) | O(1) |
| `at()` | Access element (bounds-checked) | O(1) |
| `front()` | First element | O(1) |
| `back()` | Last element | O(1) |
| `size()` | Number of elements | O(1) |
| `empty()` | Check if N == 0 | O(1) |
| `fill()` | Fill all elements with value | O(N) |
| `begin()/end()` | Iterators | O(1) |

## 🏃 Building and Running

```bash
# Compile example
make array-example

# Run example
make run-array

# Run tests
make test-array
```

## 📖 See Also

- **Vector**: For dynamic-size arrays
- **String**: For character sequences
- **Deque**: For double-ended queues

