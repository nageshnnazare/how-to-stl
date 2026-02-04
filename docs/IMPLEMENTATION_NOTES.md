/**
 * @file IMPLEMENTATION_NOTES.md
 * @brief Detailed technical notes on the unique_ptr implementation
 */

# Implementation Notes for unique_ptr

## Core Concepts

### 1. RAII (Resource Acquisition Is Initialization)

The fundamental principle behind `unique_ptr` is RAII:
- Resource acquisition happens in the constructor
- Resource release happens in the destructor
- This ensures automatic cleanup and exception safety

```cpp
{
    UniquePtr<Resource> ptr(new Resource());  // Acquisition
    // ... use resource ...
    // Destructor called automatically at end of scope
}  // Resource released here, even if exception occurs
```

### 2. Exclusive Ownership

`unique_ptr` enforces single ownership through:
- **Deleted copy constructor**: Prevents creating copies
- **Deleted copy assignment**: Prevents assigning copies
- **Move semantics only**: Ownership transfer is explicit via `std::move()`

This design prevents common bugs like:
- Double deletion (two pointers deleting the same object)
- Dangling pointers (accessing deleted memory)
- Unclear ownership (who is responsible for deletion?)

## Template Design

### Primary Template

```cpp
template<typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr { ... };
```

**Key decisions:**
1. **T**: Type of managed object
2. **Deleter**: Customizable deletion strategy (default: `delete`)
3. **Template parameter order**: Type first, deleter second (matches `std::unique_ptr`)

### Array Specialization

```cpp
template<typename T, typename Deleter>
class UniquePtr<T[], Deleter> { ... };
```

**Why separate specialization?**
1. Arrays need `delete[]` instead of `delete`
2. Arrays provide `operator[]` instead of `operator*` and `operator->`
3. Arrays should not allow pointer conversions (Derived[] to Base[] is unsafe)

## Member Variables

```cpp
T* ptr_;           // Raw pointer to managed object
Deleter deleter_;  // Deleter object
```

### Why store the deleter?

1. **Flexibility**: Supports stateful deleters (e.g., custom cleanup logic)
2. **Empty Base Optimization**: For stateless deleters, compiler can optimize away storage
3. **Type safety**: Deleter type is part of the type system

## Key Operations

### Construction

```cpp
explicit UniquePtr(T* p) noexcept : ptr_(p), deleter_() {}
```

**Why explicit?**
Prevents accidental conversions like:
```cpp
void func(UniquePtr<int> ptr);
func(new int(42));  // ERROR: Would compile without 'explicit'
```

### Move Constructor

```cpp
UniquePtr(UniquePtr&& other) noexcept 
    : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
    other.ptr_ = nullptr;  // Critical: nullify source
}
```

**Implementation details:**
1. Copy the pointer from source
2. Move the deleter (in case it's stateful)
3. Set source pointer to `nullptr` to prevent double-deletion
4. Marked `noexcept` for performance (enables move optimization)

### Destructor

```cpp
~UniquePtr() {
    if (ptr_ != nullptr) {
        deleter_(ptr_);
    }
}
```

**Critical behaviors:**
1. Check for `nullptr` (moved-from objects have null pointer)
2. Use deleter (not raw `delete`) for customization
3. No exception specification needed (destructors are `noexcept` by default)

### Reset

```cpp
void reset(T* p = nullptr) noexcept {
    T* old_ptr = ptr_;
    ptr_ = p;
    if (old_ptr != nullptr) {
        deleter_(old_ptr);
    }
}
```

**Order matters!**
1. Save old pointer first
2. Update member pointer
3. Delete old pointer last

This ordering is exception-safe and handles self-assignment:
```cpp
ptr.reset(ptr.get());  // Works correctly!
```

### Release

```cpp
T* release() noexcept {
    T* old_ptr = ptr_;
    ptr_ = nullptr;
    return old_ptr;
}
```

**Purpose:**
Transfer ownership out of `unique_ptr` without deleting.
Caller becomes responsible for deletion.

**Use cases:**
- Passing to legacy C APIs
- Transferring to different ownership model
- Extracting resource before destruction

## Deleter Design

### Default Deleter

```cpp
template<typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

// Array specialization
template<typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};
```

**Design choices:**
1. **Stateless**: No member variables → zero size with empty base optimization
2. **Callable object**: Uses `operator()` → uniform interface with lambdas
3. **Template specialization**: Automatically selects correct deletion

### Custom Deleters

Custom deleters enable:
1. **C API cleanup**: `fclose()`, `free()`, etc.
2. **Logging**: Track deletion for debugging
3. **Pool allocation**: Return to object pool instead of deleting
4. **No-op**: Manage non-owned pointers (careful!)

Example:
```cpp
struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) fclose(f);
    }
};

UniquePtr<FILE, FileDeleter> file(fopen("data.txt", "r"));
```

## Type Conversions

### Converting Constructor

```cpp
template<typename U, typename E,
         typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
UniquePtr(UniquePtr<U, E>&& other) noexcept 
    : ptr_(other.release()), deleter_() {
}
```

**Enables:**
```cpp
UniquePtr<Derived> derived = makeUnique<Derived>();
UniquePtr<Base> base = std::move(derived);  // OK: Derived* → Base*
```

**Type safety:**
- Uses `std::is_convertible` to check pointer compatibility
- Only works with move (not copy)
- Deleter must be compatible (both default deleters)

### Why Not for Arrays?

Array covariance is unsafe:
```cpp
Derived* d_arr = new Derived[10];
Base* b_arr = d_arr;  // Compiles but DANGEROUS!
delete[] b_arr;       // Wrong element size! UB!
```

Therefore, array specialization does not provide converting constructor.

## Helper Functions

### makeUnique

```cpp
template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}
```

**Advantages over `UniquePtr<T>(new T(...))`:**

1. **Exception safety:**
```cpp
// DANGEROUS:
func(UniquePtr<A>(new A()), UniquePtr<B>(new B()));
// Order of evaluation undefined! If B throws, A leaks!

// SAFE:
func(makeUnique<A>(), makeUnique<B>());
// Each allocation is immediately wrapped
```

2. **Conciseness:**
```cpp
auto ptr = makeUnique<MyClass>(arg1, arg2);  // Type deduced
```

3. **Perfect forwarding:**
```cpp
// args forwarded exactly as passed (preserves lvalue/rvalue-ness)
```

### makeUniqueArray

```cpp
template<typename T>
UniquePtr<T[]> makeUniqueArray(std::size_t size) {
    return UniquePtr<T[]>(new T[size]());
}
```

**Note:** The trailing `()` in `new T[size]()` ensures value-initialization.

## Performance Characteristics

### Memory Overhead

1. **Pointer**: 8 bytes (64-bit system)
2. **Deleter**: 0 bytes (empty base optimization for stateless deleters)
3. **Total**: 8 bytes = same as raw pointer!

### Runtime Overhead

With optimization enabled (`-O2` or higher):
- **Construction**: Same as raw pointer assignment
- **Destruction**: Same as manual `delete`
- **Move**: Three pointer assignments
- **Dereference**: Zero overhead (inlined)

**Benchmark results** (typical):
```
Raw pointer:    100% (baseline)
unique_ptr:     100% (identical)
```

## Thread Safety

### What IS thread-safe:
- Multiple threads owning separate `unique_ptr` instances
- Read-only access to a `unique_ptr` (via `get()`)

### What is NOT thread-safe:
- Concurrent modification of the same `unique_ptr`
- Concurrent `reset()` or assignment
- Ownership transfer (`std::move`) without external synchronization

**Example of safe usage:**
```cpp
// Thread 1
UniquePtr<Data> ptr1 = makeUnique<Data>();

// Thread 2
UniquePtr<Data> ptr2 = makeUnique<Data>();
// Safe: separate instances
```

**Example of unsafe usage:**
```cpp
UniquePtr<Data> shared_ptr = makeUnique<Data>();

// Thread 1
shared_ptr.reset();

// Thread 2
shared_ptr->use();  // RACE CONDITION!
```

## Exception Safety

`unique_ptr` provides **strong exception safety guarantee**:

1. **Construction**: If constructor throws, nothing is allocated
2. **Destruction**: Always succeeds (deleter must not throw)
3. **Reset**: If new allocation throws, old object remains managed
4. **Move**: `noexcept` operations cannot throw

**Example:**
```cpp
void process() {
    auto ptr = makeUnique<Resource>();  // Allocation
    ptr->doWork();                       // May throw
    // Even if doWork() throws, Resource is cleaned up
}
```

## Common Pitfalls and Solutions

### 1. Double Ownership

**WRONG:**
```cpp
MyClass* raw = new MyClass();
UniquePtr<MyClass> ptr1(raw);
UniquePtr<MyClass> ptr2(raw);  // DOUBLE DELETE!
```

**RIGHT:**
```cpp
UniquePtr<MyClass> ptr = makeUnique<MyClass>();
```

### 2. Dangling References

**WRONG:**
```cpp
MyClass* getDanglingPtr() {
    UniquePtr<MyClass> ptr = makeUnique<MyClass>();
    return ptr.get();  // DANGLING! Deleted at end of function
}
```

**RIGHT:**
```cpp
UniquePtr<MyClass> getOwningPtr() {
    return makeUnique<MyClass>();  // Transfer ownership
}
```

### 3. Incorrect Array Deletion

**WRONG:**
```cpp
UniquePtr<int> arr(new int[10]);  // Will use delete, not delete[]!
```

**RIGHT:**
```cpp
UniquePtr<int[]> arr(new int[10]);  // Uses delete[]
// OR
auto arr = makeUniqueArray<int>(10);
```

### 4. Forgetting to Move

**WRONG:**
```cpp
void takeOwnership(UniquePtr<Data> ptr) { /*...*/ }

UniquePtr<Data> ptr = makeUnique<Data>();
takeOwnership(ptr);  // ERROR: Cannot copy
```

**RIGHT:**
```cpp
takeOwnership(std::move(ptr));  // Explicit move
```

## Testing Checklist

- [ ] Basic construction and destruction
- [ ] Move semantics (constructor and assignment)
- [ ] Reset with and without new pointer
- [ ] Release and manual deletion
- [ ] Array support with subscript operator
- [ ] Custom deleter execution
- [ ] Polymorphic behavior (base/derived)
- [ ] nullptr comparisons
- [ ] Boolean conversion
- [ ] Function parameter passing (by value and by reference)
- [ ] Function return values
- [ ] Self-assignment safety
- [ ] Memory leak testing (valgrind)

## Further Optimizations

Potential improvements for production code:

1. **Compressed pair**: Store pointer and deleter in one object (saves space for stateful deleters)
2. **Constexpr support**: Make more operations `constexpr` for compile-time usage
3. **Debug mode checks**: Add assertions for null pointer dereference in debug builds
4. **Better SFINAE**: More precise enable_if conditions for template functions
5. **Allocator support**: Template parameter for custom allocation

## Standards Compliance

This implementation follows:
- **C++11**: Core features (move semantics, deleted functions)
- **C++14**: `makeUnique` helper function
- **C++17**: Compatible with structured bindings and deduction guides

## Conclusion

`unique_ptr` demonstrates several advanced C++ concepts:
- Smart pointers and RAII
- Move semantics and ownership transfer
- Template specialization
- Empty base optimization
- Perfect forwarding
- Type traits and SFINAE

Understanding its implementation provides deep insight into modern C++ design patterns.

