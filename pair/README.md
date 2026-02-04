# Pair Implementation

Two-element tuple for storing pairs of values.

## Features
- **Two Values**: Stores (first, second)
- **Used by Map/Set**: Foundation for associative containers
- **Comparison Operators**: ==, !=, <, <=, >, >=
- **make_pair Helper**: Type deduction
- **Move Semantics**: Efficient value handling

## Quick Example

```cpp
Pair<int, std::string> p(42, "answer");
std::cout << p.first << ", " << p.second;  // 42, answer

auto p2 = make_pair(3.14, "pi");  // Type deduction
```

## Operations

- `first` - Access first element
- `second` - Access second element
- `make_pair(a, b)` - Create pair with type deduction
- Comparison operators for sorting

## Use Cases

### 1. Return Multiple Values
```cpp
Pair<bool, int> divide(int a, int b) {
    if (b == 0) return {false, 0};
    return {true, a / b};
}
```

### 2. Map Entries
```cpp
Pair<std::string, int> entry("key", 100);
// Used internally by map
```

### 3. Coordinates
```cpp
Pair<double, double> point(10.5, 20.3);
std::cout << "(" << point.first << ", " << point.second << ")";
```

### 4. Key-Value
```cpp
auto config = make_pair("timeout", 30);
```

## Performance

- All operations O(1)
- No dynamic allocation
- Stored by value
- Trivially copyable for simple types

---

**Lines of Code**: 40 (implementation) + 70 (examples) + 60 (tests)  
**Test Coverage**: 10/10 tests passing ✅  
**Complexity**: O(1) all operations

