# Optional - Maybe-value Type Container

A C++ implementation of `std::optional` (C++17) - a container that may or may not contain a value.

## 📋 Overview

`Optional<T>` represents a value that might be absent, providing a type-safe alternative to using pointers or sentinel values (like -1, nullptr) to indicate "no value". It's a modern C++ idiom for handling potentially missing data without exceptions or null pointer dereferences.

## 🎯 Key Features

- **Type-Safe Null Handling**: No need for nullptr or sentinel values
- **Explicit Empty State**: Clear distinction between "no value" and "value is default"
- **Value or Default**: Easy fallback with `value_or()`
- **In-place Storage**: No heap allocation
- **Exception Safety**: Strong exception guarantees
- **Move Semantics**: Efficient transfer of values

## 🏗️ Implementation Details

### Data Structure
```cpp
template<typename T>
class Optional {
private:
    alignas(T) unsigned char storage_[sizeof(T)];  // Aligned raw storage
    bool has_value_;                                // State flag
    
    // Placement new for construction
    // Manual destructor calls for destruction
};
```

### Key Techniques
- **Placement New**: Construct objects in pre-allocated storage
- **Aligned Storage**: Ensure proper alignment for type T
- **Manual Lifetime Management**: Explicit construction/destruction

### Complexity
- **Construction**: O(1)
- **Access**: O(1)
- **Destruction**: O(1)
- **Space**: sizeof(T) + sizeof(bool) + alignment padding

## 📚 Core Operations

### Construction
```cpp
Optional<int> opt1;              // Empty optional
Optional<int> opt2(42);          // Optional with value
Optional<std::string> opt3("hi"); // Works with any type
```

### Checking for Value
```cpp
if (opt.has_value()) { }         // Explicit check
if (opt) { }                     // Implicit bool conversion
```

### Accessing Value
```cpp
int val = opt.value();           // Throws if empty
int val = *opt;                  // Unsafe (like raw pointer)
int val = opt.value_or(99);      // Safe with default
```

### Resetting
```cpp
opt.reset();                     // Clear the value
```

## 🚀 Usage Examples

### Basic Usage
```cpp
Optional<int> divide(int a, int b) {
    if (b == 0) return Optional<int>();  // Return empty
    return Optional<int>(a / b);
}

auto result = divide(10, 2);
if (result) {
    std::cout << "Result: " << *result << "\n";
}
```

### Safe Default Values
```cpp
Optional<std::string> get_username();

std::string name = get_username().value_or("Guest");
std::cout << "Welcome, " << name << "\n";
```

### Configuration Pattern
```cpp
struct Config {
    Optional<int> port;
    Optional<std::string> host;
};

Config config;  // User doesn't provide config

int port = config.port.value_or(8080);        // Use default
std::string host = config.host.value_or("localhost");
```

### Complex Types
```cpp
struct User {
    std::string name;
    int age;
};

Optional<User> find_user(int id);

if (auto user = find_user(123)) {
    std::cout << user->name;  // Use arrow operator
}
```

## 🔍 When to Use

### Use Optional When:
- ✅ Function might not return a value (alternative to exceptions)
- ✅ Field might be uninitialized or absent
- ✅ You want to avoid sentinel values (-1, nullptr, "")
- ✅ Explicit "no value" is semantically important
- ✅ You want type-safe null handling

### Use Other Approaches When:
- ❌ Value is always present → Use T directly
- ❌ Multiple error conditions → Use exceptions or Result<T, E>
- ❌ Performance critical path with always-present values → Use T
- ❌ Need to distinguish multiple empty states → Use enum or variant

## 🆚 Comparison with Alternatives

| Approach | Type Safety | Semantics | Overhead |
|----------|-------------|-----------|----------|
| **Optional<T>** | ✅ Strong | Clear "maybe" | bool flag |
| **T\*** | ❌ Weak | Unclear lifetime | Pointer size |
| **Sentinel (-1)** | ❌ None | Ambiguous | None |
| **Exception** | ✅ Strong | Error handling | Call stack |
| **Pair<bool, T>** | ⚠️ Manual | Verbose | bool + T |

## 🎓 Special Features

### value() vs value_or()
```cpp
Optional<int> opt;

// value() - throws if empty (use when you expect a value)
try {
    int val = opt.value();
} catch (...) { }

// value_or() - returns default if empty (use for fallback)
int val = opt.value_or(42);  // No exception, returns 42
```

### Bool Conversion
```cpp
Optional<int> opt(0);  // Value is 0

if (opt) {
    // This executes! Optional is not empty even though value is 0
    std::cout << *opt;  // Prints: 0
}
```

### Pointer-like Access
```cpp
struct Point { int x, y; };
Optional<Point> opt(Point{10, 20});

opt->x = 30;           // Modify through arrow operator
std::cout << (*opt).y; // Dereference and access
```

## 💡 Best Practices

1. **Prefer value_or() for Defaults**: More readable than if-else
2. **Check Before Dereferencing**: Use `if (opt)` before `*opt`
3. **Use value() When You Expect Value**: Makes bugs visible
4. **Return Optional from Functions**: Better than sentinel values
5. **Don't Overuse**: If value is always present, use T directly

## 📊 Common Patterns

### Safe Division
```cpp
Optional<double> safe_divide(double a, double b) {
    if (b == 0.0) return Optional<double>();
    return Optional<double>(a / b);
}
```

### Database Lookup
```cpp
Optional<User> find_by_id(int id) {
    if (/* found in DB */) return Optional<User>(user);
    return Optional<User>();
}
```

### Configuration with Defaults
```cpp
class Server {
    int port = config.port.value_or(8080);
    int timeout = config.timeout.value_or(30);
    std::string host = config.host.value_or("localhost");
};
```

### Optional Chaining
```cpp
Optional<User> user = get_user();
Optional<Address> addr = user ? get_address(*user) : Optional<Address>();
std::string city = addr ? addr->city : "Unknown";
```

## 🔗 API Reference

| Method | Description | Throws | Time |
|--------|-------------|--------|------|
| `Optional()` | Construct empty | No | O(1) |
| `Optional(T)` | Construct with value | No | O(1) |
| `has_value()` | Check if contains value | No | O(1) |
| `operator bool()` | Implicit bool check | No | O(1) |
| `value()` | Get value (throw if empty) | Yes | O(1) |
| `value_or(T)` | Get value or default | No | O(1) |
| `operator*()` | Dereference (unsafe) | No | O(1) |
| `operator->()` | Arrow access (unsafe) | No | O(1) |
| `reset()` | Clear value | No | O(1) |

## 🏃 Building and Running

```bash
# Compile example
make optional-example

# Run example
make run-optional

# Run tests
make test-optional
```

## 🌟 Advantages Over Raw Pointers

| Feature | Optional<T> | T* |
|---------|-------------|-----|
| **Ownership** | Value semantics | Unclear |
| **Null Safety** | Explicit check | Easy to forget |
| **Allocation** | Stack | Often heap |
| **Copying** | Deep copy | Shallow (pointer) |
| **Lifetime** | Automatic | Manual |
| **Intent** | "Maybe has value" | "Points to something" |

## 📖 See Also

- **Pair**: For two-value tuples
- **Tuple**: For multiple values
- **Variant**: For one-of-many types (like Rust's enum)

