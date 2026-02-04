/**
 * @file QUICK_REFERENCE.md
 * @brief Quick reference guide for unique_ptr usage
 */

# unique_ptr Quick Reference

## Basic Operations

### Create a unique_ptr
```cpp
// Method 1: Using makeUnique (RECOMMENDED)
auto ptr = makeUnique<MyClass>(arg1, arg2);

// Method 2: From raw pointer
UniquePtr<MyClass> ptr(new MyClass(arg1, arg2));

// Method 3: Empty pointer
UniquePtr<MyClass> ptr;  // or UniquePtr<MyClass> ptr(nullptr);
```

### Access the object
```cpp
ptr->method();        // Call member function
(*ptr).method();      // Alternative syntax
MyClass& ref = *ptr;  // Get reference
```

### Check if valid
```cpp
if (ptr) {
    // ptr owns an object
}

if (ptr != nullptr) {
    // ptr owns an object
}

if (ptr.get() != nullptr) {
    // ptr owns an object
}
```

### Transfer ownership
```cpp
UniquePtr<MyClass> ptr2 = std::move(ptr);
// ptr is now empty, ptr2 owns the object
```

### Reset (replace managed object)
```cpp
ptr.reset();                           // Delete current, become empty
ptr.reset(new MyClass());              // Delete current, manage new object
ptr.reset(nullptr);                    // Same as reset()
```

### Release ownership
```cpp
MyClass* raw = ptr.release();
// ptr is now empty, you must delete raw manually
delete raw;
```

### Get raw pointer (without releasing)
```cpp
MyClass* raw = ptr.get();
// ptr still owns the object, don't delete raw!
```

## Arrays

### Create array
```cpp
// Method 1: Using makeUniqueArray (RECOMMENDED)
auto arr = makeUniqueArray<int>(10);

// Method 2: From raw pointer
UniquePtr<int[]> arr(new int[10]);
```

### Access array elements
```cpp
arr[0] = 42;
arr[5] = arr[0] + 1;

int value = arr[3];
```

### Arrays don't support -> and *
```cpp
arr->method();  // ERROR: Arrays don't have operator->
*arr;           // ERROR: Arrays don't have operator*
```

## Custom Deleters

### Define a custom deleter
```cpp
struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) fclose(f);
    }
};
```

### Use custom deleter
```cpp
// Method 1: In type
UniquePtr<FILE, FileDeleter> file(fopen("test.txt", "r"));

// Method 2: Pass to constructor
FILE* f = fopen("test.txt", "r");
FileDeleter deleter;
UniquePtr<FILE, FileDeleter> file(f, deleter);
```

### Lambda as deleter
```cpp
auto deleter = [](MyClass* p) { 
    std::cout << "Deleting\n";
    delete p;
};

UniquePtr<MyClass, decltype(deleter)> ptr(new MyClass(), deleter);
```

## Function Parameters

### Take ownership (by value)
```cpp
void process(UniquePtr<MyClass> ptr) {
    ptr->method();
    // Object deleted when function returns
}

// Call with:
auto ptr = makeUnique<MyClass>();
process(std::move(ptr));  // Must use std::move
```

### Borrow (by const reference)
```cpp
void examine(const UniquePtr<MyClass>& ptr) {
    if (ptr) {
        ptr->method();
    }
    // Caller still owns the object
}

// Call with:
auto ptr = makeUnique<MyClass>();
examine(ptr);  // No move needed
```

### Modify pointer (by non-const reference)
```cpp
void reset_ptr(UniquePtr<MyClass>& ptr) {
    ptr.reset(new MyClass());
}

// Call with:
auto ptr = makeUnique<MyClass>();
reset_ptr(ptr);  // No move needed
```

### Use raw pointer (borrow without smart pointer)
```cpp
void use(MyClass* obj) {
    if (obj) {
        obj->method();
    }
}

// Call with:
auto ptr = makeUnique<MyClass>();
use(ptr.get());  // Pass raw pointer
```

## Return Values

### Return by value
```cpp
UniquePtr<MyClass> createObject() {
    return makeUnique<MyClass>();  // No std::move needed (RVO)
}

// Use:
auto obj = createObject();
```

### Conditional return
```cpp
UniquePtr<MyClass> createIfNeeded(bool condition) {
    if (condition) {
        return makeUnique<MyClass>();
    }
    return nullptr;  // Return empty pointer
}
```

### Return from local variable
```cpp
UniquePtr<MyClass> process() {
    auto ptr = makeUnique<MyClass>();
    ptr->setup();
    return ptr;  // Automatic move, no std::move needed
}
```

## Polymorphism

### Store derived class in base pointer
```cpp
class Base { virtual ~Base() = default; };
class Derived : public Base { };

UniquePtr<Base> ptr = makeUnique<Derived>();
ptr->virtualMethod();  // Calls Derived version
```

### Convert derived to base
```cpp
UniquePtr<Derived> derived = makeUnique<Derived>();
UniquePtr<Base> base = std::move(derived);  // OK
```

### Cannot convert base to derived
```cpp
UniquePtr<Base> base = makeUnique<Base>();
UniquePtr<Derived> derived = std::move(base);  // ERROR
```

## Common Patterns

### Factory function
```cpp
UniquePtr<Widget> createWidget(WidgetType type) {
    switch (type) {
        case TYPE_A: return makeUnique<WidgetA>();
        case TYPE_B: return makeUnique<WidgetB>();
        default: return nullptr;
    }
}
```

### Pimpl idiom (pointer to implementation)
```cpp
// header.hpp
class MyClass {
    struct Impl;
    UniquePtr<Impl> pimpl_;
public:
    MyClass();
    ~MyClass();
    void method();
};

// source.cpp
struct MyClass::Impl {
    // Implementation details
};

MyClass::MyClass() : pimpl_(makeUnique<Impl>()) {}
MyClass::~MyClass() = default;
void MyClass::method() { pimpl_->...  }
```

### RAII wrapper
```cpp
class FileRAII {
    UniquePtr<FILE, FileDeleter> file_;
public:
    FileRAII(const char* filename) 
        : file_(fopen(filename, "r")) {
        if (!file_) throw std::runtime_error("Failed to open");
    }
    
    FILE* get() { return file_.get(); }
};
```

### Optional ownership
```cpp
class Resource {
    UniquePtr<Data> owned_data_;    // May be null
public:
    void setData(UniquePtr<Data> data) {
        owned_data_ = std::move(data);
    }
    
    bool hasData() const {
        return owned_data_ != nullptr;
    }
};
```

## Anti-Patterns (DON'T DO THESE)

### ❌ Creating from existing unique_ptr
```cpp
UniquePtr<MyClass> ptr1 = makeUnique<MyClass>();
UniquePtr<MyClass> ptr2(ptr1.get());  // WRONG: Double delete!
```

### ❌ Storing raw pointer from unique_ptr
```cpp
MyClass* raw;
{
    auto ptr = makeUnique<MyClass>();
    raw = ptr.get();
}
raw->method();  // WRONG: Dangling pointer!
```

### ❌ Forgetting to move
```cpp
void takeOwnership(UniquePtr<MyClass> ptr);

auto ptr = makeUnique<MyClass>();
takeOwnership(ptr);  // WRONG: Won't compile
```

### ❌ Moving when you want to borrow
```cpp
void examine(const UniquePtr<MyClass>& ptr);

auto ptr = makeUnique<MyClass>();
examine(std::move(ptr));  // WRONG: Unnecessary move
```

### ❌ Wrong array deletion
```cpp
UniquePtr<int> arr(new int[10]);  // WRONG: Will call delete, not delete[]
```

### ❌ Manual memory management mix
```cpp
MyClass* raw = new MyClass();
UniquePtr<MyClass> ptr(raw);
delete raw;  // WRONG: unique_ptr will try to delete again!
```

## Cheat Sheet

| Operation | Code | Notes |
|-----------|------|-------|
| Create | `auto ptr = makeUnique<T>(args)` | Preferred way |
| Access | `ptr->member` | Like raw pointer |
| Check | `if (ptr)` | Test if non-null |
| Move | `ptr2 = std::move(ptr1)` | Transfer ownership |
| Reset | `ptr.reset()` | Delete and clear |
| Release | `T* raw = ptr.release()` | Give up ownership |
| Get | `T* raw = ptr.get()` | Peek at pointer |
| Array | `auto arr = makeUniqueArray<T>(n)` | For arrays |
| Index | `arr[i]` | Array access |
| Custom delete | `UniquePtr<T, Deleter>` | Custom cleanup |

## Compilation

### Compile with C++14
```bash
g++ -std=c++14 -Wall -Wextra your_file.cpp
```

### Compile with C++11
```bash
g++ -std=c++11 -Wall -Wextra your_file.cpp
# Note: makeUnique requires C++14
```

## Memory Leak Detection

### With valgrind
```bash
valgrind --leak-check=full ./your_program
```

### With Address Sanitizer
```bash
g++ -std=c++14 -fsanitize=address -g your_file.cpp
./a.out
```

## When to Use unique_ptr

✅ **Use unique_ptr when:**
- You need automatic cleanup
- Ownership is clear and exclusive
- Resource is dynamically allocated
- You want exception safety
- You're building a factory function
- Implementing RAII wrappers

❌ **Don't use unique_ptr when:**
- You need shared ownership (use shared_ptr)
- Object has static/automatic storage
- You're working with existing APIs that manage memory
- Performance of move is critical and measured to be a bottleneck

## Performance Tips

1. **Pass by reference when borrowing** - avoids unnecessary moves
2. **Use makeUnique** - exception-safe and concise
3. **Prefer unique_ptr over shared_ptr** - lower overhead when sharing not needed
4. **Return by value** - compiler will optimize (RVO/NRVO)
5. **Empty base optimization** - stateless deleters have zero size cost

## Debugging Tips

1. **Check for null before dereferencing**
   ```cpp
   if (ptr) { ptr->method(); }
   ```

2. **Use assertions in debug mode**
   ```cpp
   assert(ptr != nullptr);
   ptr->method();
   ```

3. **Enable compiler warnings**
   ```bash
   g++ -Wall -Wextra -Wpedantic
   ```

4. **Use sanitizers during development**
   ```bash
   g++ -fsanitize=address,undefined
   ```

5. **Run valgrind regularly**
   ```bash
   valgrind --leak-check=full ./program
   ```

