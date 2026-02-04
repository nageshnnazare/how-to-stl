# Custom unique_ptr Implementation

This directory contains a complete, production-quality implementation of `std::unique_ptr` in C++.

## Overview

`unique_ptr` is a smart pointer that provides:
- **Exclusive ownership**: Only one `unique_ptr` can own an object at any time
- **Automatic cleanup**: RAII-based resource management (Resource Acquisition Is Initialization)
- **Move semantics**: Ownership can be transferred but not copied
- **Zero overhead**: When optimized, has no runtime cost compared to raw pointers
- **Custom deleters**: Support for custom cleanup logic

## Files

- **`unique_ptr.hpp`**: Complete implementation of `UniquePtr` class template
  - Main template for single objects
  - Array specialization for `T[]`
  - Custom deleter support
  - Helper functions (`makeUnique`, etc.)

- **`example.cpp`**: Comprehensive examples demonstrating:
  - Basic usage and RAII
  - Move semantics
  - Array support
  - Custom deleters
  - Polymorphism
  - Function parameters and return values
  - Comparison operations

- **`Makefile`**: Build system for compiling and running examples

## Building

### Compile the example:
```bash
make
```

### Run the example:
```bash
make run
```

### Build with debug symbols:
```bash
make debug
```

### Check for memory leaks (requires valgrind):
```bash
make valgrind
```

### Clean build artifacts:
```bash
make clean
```

## Key Features Implemented

### 1. Constructors
- Default constructor (creates empty pointer)
- Constructor from raw pointer
- Constructor with custom deleter
- Move constructor (transfers ownership)
- Deleted copy constructor (enforces exclusive ownership)

### 2. Destructors
- Automatic cleanup using RAII
- Calls appropriate deleter (`delete` or `delete[]`)

### 3. Assignment Operators
- Move assignment (transfers ownership)
- Assignment from `nullptr` (resets pointer)
- Deleted copy assignment (enforces exclusive ownership)

### 4. Modifiers
- `reset()`: Replace managed object
- `release()`: Release ownership without deleting
- `swap()`: Exchange contents with another `unique_ptr`

### 5. Observers
- `get()`: Access raw pointer
- `get_deleter()`: Access deleter object
- `operator bool()`: Check if managing an object

### 6. Dereference Operators
- `operator*()`: Dereference to managed object
- `operator->()`: Access members of managed object
- `operator[]()`: Array subscript (array specialization only)

### 7. Comparison Operators
- Equality and inequality with other `unique_ptr`
- Comparison with `nullptr`

### 8. Helper Functions
- `makeUnique<T>(args...)`: Create `unique_ptr` (like `std::make_unique`)
- `makeUniqueArray<T>(size)`: Create array `unique_ptr`

## Implementation Details

### Memory Management
The implementation uses RAII to ensure automatic cleanup:
```cpp
{
    UniquePtr<Resource> ptr(new Resource());
    // Use the resource...
} // Destructor automatically called, resource cleaned up
```

### Move Semantics
Ownership transfer is explicit through `std::move`:
```cpp
UniquePtr<Resource> ptr1(new Resource());
UniquePtr<Resource> ptr2 = std::move(ptr1);
// ptr1 is now empty, ptr2 owns the resource
```

### Array Support
Special handling for arrays using `delete[]`:
```cpp
UniquePtr<int[]> arr(new int[10]);
arr[0] = 42;  // Can use subscript operator
// delete[] automatically called
```

### Custom Deleters
Support for custom cleanup logic:
```cpp
struct FileDeleter {
    void operator()(FILE* f) const { fclose(f); }
};

UniquePtr<FILE, FileDeleter> file(fopen("test.txt", "r"));
// File automatically closed when unique_ptr destroyed
```

## Design Decisions

1. **Deleted Copy Operations**: Copy constructor and copy assignment are deleted to enforce exclusive ownership semantics.

2. **Template Specialization**: Separate specialization for arrays (`T[]`) to use `delete[]` and provide `operator[]`.

3. **Empty Base Optimization**: The deleter is stored as a member, allowing for empty base optimization when using stateless deleters.

4. **noexcept Specifications**: Move operations and other non-throwing functions are marked `noexcept` for optimization opportunities.

5. **Explicit Constructors**: Constructor from raw pointer is `explicit` to prevent accidental conversions.

## Differences from std::unique_ptr

This implementation closely mirrors `std::unique_ptr`, with minor simplifications:
- Simplified template constraints (production code would use SFINAE/concepts for better type checking)
- Limited support for converting constructors
- Simplified deleter handling (production code would handle reference deleters)

## Usage Examples

### Basic Usage
```cpp
auto ptr = makeUnique<MyClass>(arg1, arg2);
ptr->method();
```

### Transferring Ownership
```cpp
void takeOwnership(UniquePtr<MyClass> ptr) {
    // Function owns the object now
}

auto ptr = makeUnique<MyClass>();
takeOwnership(std::move(ptr));
// ptr is now empty
```

### Returning from Functions
```cpp
UniquePtr<MyClass> createObject() {
    return makeUnique<MyClass>();
}

auto obj = createObject();  // Ownership transferred to caller
```

## Performance

The implementation is designed for zero overhead:
- No virtual functions
- No dynamic allocation beyond the managed object
- Move operations are simple pointer swaps
- With optimization enabled (`-O2` or `-O3`), performance is identical to raw pointers

## Thread Safety

Like `std::unique_ptr`, this implementation is **not thread-safe** for concurrent access to the same instance. However:
- Multiple threads can safely own separate `unique_ptr` instances
- Ownership transfer via move must be synchronized by the caller

## Memory Leak Testing

Run with valgrind to verify no memory leaks:
```bash
make valgrind
```

Expected output should show:
```
All heap blocks were freed -- no leaks are possible
```

## Learning Resources

This implementation demonstrates several important C++ concepts:
- RAII (Resource Acquisition Is Initialization)
- Move semantics
- Template specialization
- Perfect forwarding
- Type traits
- Custom deleters
- Operator overloading

## License

This is an educational implementation for learning purposes.

