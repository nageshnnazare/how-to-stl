# Tuple Implementation

Fixed-size collection of heterogeneous values (2-3 elements).

## Features
- **Multiple Values**: Store 2 or 3 different types
- **get<N>()**: Access by index at compile time
- **Type Safe**: Each element has its own type
- **Return Multiple**: Perfect for functions returning multiple values

## Quick Example

```cpp
Tuple<int, double, std::string> t(42, 3.14, "hello");

std::cout << get<0>(t);  // 42
std::cout << get<1>(t);  // 3.14
std::cout << get<2>(t);  // "hello"
```

## Operations

- `get<N>(tuple)` - Access element N
- Constructor with 2-3 arguments
- Equality comparison
- Copy and move semantics

## Use Cases

### 1. Return Multiple Values
```cpp
Tuple<bool, int, std::string> divide(int a, int b) {
    if (b == 0) return {false, 0, "Error"};
    return {true, a/b, "Success"};
}

auto result = divide(10, 2);
if (get<0>(result)) {
    std::cout << "Result: " << get<1>(result);
}
```

### 2. 3D Coordinates
```cpp
Tuple<double, double, double> point(x, y, z);
```

### 3. Parse Results
```cpp
Tuple<bool, Token, std::string> parse(input);
```

## Performance

- All operations O(1)
- No dynamic allocation
- Compile-time type checking
- Zero overhead abstraction

---

**Lines of Code**: 100+ (implementation) + 80 (examples) + 90 (tests)  
**Test Coverage**: 10/10 tests passing ✅  
**C++ Version**: Requires C++17 for if constexpr

