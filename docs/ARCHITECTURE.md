/**
 * @file ARCHITECTURE.md
 * @brief Visual diagrams and architecture overview of unique_ptr
 */

# unique_ptr Architecture

## Class Hierarchy

```
UniquePtr<T, Deleter>
├── Primary Template (for single objects)
│   ├── Members:
│   │   ├── T* ptr_
│   │   └── Deleter deleter_
│   ├── Provides:
│   │   ├── operator*()
│   │   ├── operator->()
│   │   └── Type conversions (Derived → Base)
│   └── Uses: delete
│
└── Array Specialization UniquePtr<T[], Deleter>
    ├── Members:
    │   ├── T* ptr_
    │   └── Deleter deleter_
    ├── Provides:
    │   └── operator[]()
    ├── Does NOT provide:
    │   ├── operator*()
    │   ├── operator->()
    │   └── Type conversions
    └── Uses: delete[]
```

## Memory Layout

### Single Object
```
Stack:                          Heap:
┌─────────────────┐            ┌─────────────────┐
│   UniquePtr     │            │     Object      │
│  ┌───────────┐  │            │                 │
│  │ ptr_ ─────┼──┼───────────>│ member1         │
│  ├───────────┤  │            │ member2         │
│  │ deleter_  │  │            │ ...             │
│  │ (size: 0*)│  │            └─────────────────┘
│  └───────────┘  │    * With empty base optimization
└─────────────────┘      for stateless deleters

Total size: 8 bytes (on 64-bit)
```

### Array
```
Stack:                          Heap:
┌─────────────────┐            ┌─────────────────┐
│ UniquePtr<T[]>  │            │ Array Elements  │
│  ┌───────────┐  │            │                 │
│  │ ptr_ ─────┼──┼───────────>│ [0] element     │
│  ├───────────┤  │            │ [1] element     │
│  │ deleter_  │  │            │ [2] element     │
│  │ (size: 0*)│  │            │ ...             │
│  └───────────┘  │            └─────────────────┘
└─────────────────┘

Access: arr[i] → *(ptr_ + i)
```

## Lifetime Flow

### Construction → Usage → Destruction
```
1. Construction                 2. Usage                    3. Destruction
┌──────────────┐               ┌──────────────┐             ┌──────────────┐
│ new Object() │               │ ptr->method()│             │    ~Ptr()    │
└──────┬───────┘               └──────┬───────┘             └──────┬───────┘
       │                              │                            │
       v                              v                            v
┌──────────────┐               ┌──────────────┐            ┌──────────────┐
│  Wrap in     │               │ Dereference  │            │if (ptr_)     │
│ UniquePtr    │               │  ptr_ to     │            │  deleter_()  │
│              │               │ access object│            │              │
└──────────────┘               └──────────────┘            └──────────────┘
       │                              │                            │
       v                              v                            v
  Object owned              Object still owned           Object deleted
  by unique_ptr             by unique_ptr                Memory freed
```

## Move Semantics Flow

### Moving from ptr1 to ptr2
```
Before Move:
┌─────────────┐                    ┌─────────────┐
│    ptr1     │                    │    ptr2     │
│  ┌───────┐  │                    │  ┌───────┐  │
│  │ ptr_  │  │────> [Object]      │  │nullptr│  │
│  └───────┘  │                    │  └───────┘  │
└─────────────┘                    └─────────────┘

Move: ptr2 = std::move(ptr1)

After Move:
┌─────────────┐                    ┌─────────────┐
│    ptr1     │                    │    ptr2     │
│  ┌───────┐  │                    │  ┌───────┐  │
│  │nullptr│  │      [Object] <────┼──│ ptr_  │  │
│  └───────┘  │                    │  └───────┘  │
└─────────────┘                    └─────────────┘
```

## Operation State Transitions

```
                    ┌──────────────┐
                    │   nullptr    │  ← Default constructed
                    │  (no object) │
                    └──────┬───────┘
                           │ reset(new T)
                           │
                    ┌──────▼───────┐
         release()  │              │  reset()
        ┌───────────┤   Owning     ├─────────┐
        │           │   (has obj)  │         │
        │           └──────┬───────┘         │
        │                  │                 │
        │                  │ std::move       │
        │                  │                 │
        ▼                  ▼                 ▼
┌──────────────┐    ┌──────────────┐  ┌──────────────┐
│   Manual     │    │   Moved-from │  │   nullptr    │
│   delete     │    │   (nullptr)  │  │  (no object) │
│   required   │    └──────────────┘  └──────────────┘
└──────────────┘
```

## Deleter Dispatch

### Compile-time selection
```
                    UniquePtr<T, Deleter>
                            │
                            │
              ┌─────────────┴─────────────┐
              │                           │
              ▼                           ▼
    T is single object              T is array (T[])
              │                           │
              ▼                           ▼
   DefaultDeleter<T>::                DefaultDeleter<T[]>::
    operator()(T* ptr)                 operator()(T* ptr)
              │                           │
              ▼                           ▼
        delete ptr;                  delete[] ptr;
```

### Custom deleter example
```
UniquePtr<FILE, FileDeleter>
         │
         │ destructor called
         ▼
  deleter_(ptr_)
         │
         ▼
FileDeleter::operator()(FILE* f)
         │
         ▼
     fclose(f)
```

## Type Conversion (Polymorphism)

```
Class Hierarchy:          unique_ptr Conversion:
                         
    Base                  UniquePtr<Derived>
      ▲                           │
      │                           │ std::move
      │                           ▼
   Derived                UniquePtr<Base>
                                  │
                                  │ destructor
                                  ▼
                          delete (Base*) ptr
                                  │
                                  ▼
                          Calls ~Derived() [virtual]
                          then  ~Base()
```

## Function Parameter Patterns

### 1. Sink (Take Ownership)
```
Caller                          Function
┌─────────────┐                ┌─────────────┐
│ UniquePtr   │                │ UniquePtr   │
│   owns      │─────move──────>│   owns      │
│             │                │             │
└─────────────┘                └──────┬──────┘
     Empty                            │
                                      ▼
                               Object deleted
```

### 2. Borrow (Reference)
```
Caller                          Function
┌─────────────┐                ┌─────────────┐
│ UniquePtr   │                │const Ptr&   │
│   owns      │───reference───>│  borrows    │
│             │                │             │
└──────┬──────┘                └─────────────┘
       │
       │ Still owns
       ▼
Object persists
```

### 3. In-Out (Modify Pointer)
```
Caller                          Function
┌─────────────┐                ┌─────────────┐
│ UniquePtr   │                │   Ptr&      │
│   owns      │←───reference───┤  modifies   │
│             │                │             │
└──────┬──────┘                └─────────────┘
       │
       │ May be reset/modified
       ▼
New or same object
```

## Construction Patterns

### Direct Construction
```
new Object()  ──>  UniquePtr<Object>(...)  ──>  Owns object
```

### makeUnique (Preferred)
```
makeUnique<Object>(args)
         │
         ▼
    new Object(args)  ──>  return UniquePtr<Object>(...)
         │
         └──> Exception-safe wrapping
```

### From Factory
```
Factory::create()
         │
         ▼
    return makeUnique<Object>()
         │
         ▼
    Caller receives UniquePtr<Object>
```

## Exception Safety

### With unique_ptr (Safe)
```
try {
    UniquePtr<A> a = makeUnique<A>();  ← A allocated
    UniquePtr<B> b = makeUnique<B>();  ← B allocated
    doWork();                          ← May throw
}
catch (...) {
    // Both A and B automatically deleted by destructors
}
```

### Without unique_ptr (Unsafe)
```
try {
    A* a = new A();                    ← A allocated
    B* b = new B();                    ← B allocated
    doWork();                          ← May throw
    delete a;                          ← Never reached if throw!
    delete b;                          ← Never reached if throw!
}
catch (...) {
    // Memory leak! A and B not deleted
}
```

## Comparison with Raw Pointer

```
Raw Pointer:                    unique_ptr:

┌─────────────┐                ┌─────────────┐
│   T* ptr    │                │ UniquePtr<T>│
└──────┬──────┘                └──────┬──────┘
       │                              │
Manual │                     Automatic│
       │                              │
       ▼                              ▼
┌─────────────┐                ┌─────────────┐
│new/delete   │                │Constructor/ │
│   manual    │                │ Destructor  │
└─────────────┘                └─────────────┘
       │                              │
       ▼                              ▼
   Error-prone                  Exception-safe
   No ownership                 Clear ownership
   May leak                     Cannot leak
   Copyable                     Move-only
```

## Memory Safety Guarantees

```
╔═══════════════════════════════════════════════════╗
║  GUARANTEES PROVIDED BY unique_ptr                ║
╠═══════════════════════════════════════════════════╣
║                                                   ║
║  ✓ Object deleted exactly once                    ║
║  ✓ Deleted when last owner destroyed              ║
║  ✓ No dangling pointers (after move)              ║
║  ✓ No double-deletion                             ║
║  ✓ Exception-safe cleanup                         ║
║  ✓ Clear ownership semantics                      ║
║  ✓ Zero runtime overhead                          ║
║                                                   ║
╠═══════════════════════════════════════════════════╣
║  DOES NOT GUARANTEE                               ║
╠═══════════════════════════════════════════════════╣
║                                                   ║
║  ✗ Thread-safety (use external sync)              ║
║  ✗ Null pointer safety (check before use)         ║
║  ✗ Protection from raw pointer misuse             ║
║  ✗ Shared ownership (use shared_ptr)              ║
║                                                   ║
╚═══════════════════════════════════════════════════╝
```

## Performance Characteristics

```
Operation           Raw Pointer    unique_ptr     Overhead
─────────────────────────────────────────────────────────
Construction        O(1)           O(1)           0%
Destruction         O(1)           O(1)           0%
Move                O(1)           O(1)           0%
Dereference         O(1)           O(1)           0%
Memory              8 bytes        8 bytes*       0%
                                   *with EBO

* EBO = Empty Base Optimization for stateless deleters
```

## Integration Patterns

### With STL Containers
```
std::vector<UniquePtr<Widget>>
         │
         ├──> [UniquePtr] ──> Widget1
         │
         ├──> [UniquePtr] ──> Widget2
         │
         └──> [UniquePtr] ──> Widget3

Operations:
- push_back(std::move(ptr))   ← Transfer ownership to vector
- vec.erase(it)                ← Deletes object automatically
- vec.clear()                  ← Deletes all objects
```

### Pimpl Pattern
```
Public Header:              Implementation File:
┌─────────────┐            ┌─────────────┐
│   Class     │            │  Class::    │
│  ┌───────┐  │            │   Impl      │
│  │ Impl* │──┼───────────>│  ┌───────┐  │
│  └───────┘  │            │  │ data  │  │
└─────────────┘            │  │methods│  │
                           │  └───────┘  │
                           └─────────────┘
Benefits:
- Hide implementation details
- Reduce compilation dependencies
- Binary compatibility
```

## Summary

The unique_ptr architecture is built on:
1. **RAII** - Automatic resource management
2. **Move semantics** - Exclusive ownership transfer
3. **Template specialization** - Correct deletion for arrays
4. **Zero overhead** - No runtime cost vs raw pointers
5. **Type safety** - Compile-time error prevention

This design makes unique_ptr the default choice for dynamic memory management in modern C++.

