# Memory Allocator - Custom Allocation Strategies

A comprehensive implementation of **4 custom memory allocators** demonstrating different allocation strategies for various use cases.

## 📋 Overview

Custom memory allocators can significantly improve performance for specific workloads by:
- Reducing allocation overhead
- Minimizing fragmentation
- Improving cache locality
- Enabling bulk deallocation

This implementation provides 4 distinct allocator types, each optimized for different scenarios.

## 🎯 Allocator Types

### 1. Linear Allocator (Bump Allocator)
**Strategy**: Bump a pointer forward, no individual deallocation  
**Complexity**: O(1) allocation, O(1) bulk reset  
**Use Case**: Frame allocations, temporary data, parsers

**Characteristics:**
- ✅ Extremely fast allocation (just increment pointer)
- ✅ Zero fragmentation
- ✅ Minimal bookkeeping
- ❌ No individual deallocation
- ❌ Must reset entire allocator

### 2. Pool Allocator
**Strategy**: Fixed-size blocks with free list  
**Complexity**: O(1) allocation and deallocation  
**Use Case**: Object pools, entity systems, particle systems

**Characteristics:**
- ✅ Constant-time operations
- ✅ Zero fragmentation (all blocks same size)
- ✅ Excellent cache locality
- ✅ Predictable memory usage
- ❌ Only one size per pool
- ❌ Fixed capacity

### 3. Stack Allocator
**Strategy**: LIFO allocation with markers  
**Complexity**: O(1) allocation, O(1) marker-based deallocation  
**Use Case**: Scope-based allocation, recursive algorithms

**Characteristics:**
- ✅ Fast allocation/deallocation
- ✅ Bulk deallocation via markers
- ✅ Natural scope-based usage
- ❌ Must deallocate in LIFO order
- ❌ No arbitrary deallocation

### 4. Free List Allocator
**Strategy**: Free list with coalescing  
**Complexity**: O(n) allocation/deallocation (n = free blocks)  
**Use Case**: General purpose, long-lived objects

**Characteristics:**
- ✅ Arbitrary allocation sizes
- ✅ Any deallocation order
- ✅ Block coalescing reduces fragmentation
- ✅ General purpose (like malloc)
- ❌ Slower than specialized allocators
- ❌ Can still fragment

## 🏗️ Implementation Details

### Linear Allocator
```cpp
class LinearAllocator {
    char* buffer_;      // Memory block
    size_t offset_;     // Current position
    
public:
    void* allocate(size_t size, size_t alignment);
    void reset();       // Reclaim all memory
};
```

**Algorithm:**
1. Align current offset
2. Check if space available
3. Return pointer, advance offset
4. Reset: offset = 0

### Pool Allocator
```cpp
class PoolAllocator {
    struct FreeNode { FreeNode* next; };
    FreeNode* free_list_;    // Linked list of free blocks
    
public:
    void* allocate();        // Pop from free list
    void deallocate(void*);  // Push to free list
};
```

**Algorithm:**
1. Initialize: Link all blocks in free list
2. Allocate: Pop from free list
3. Deallocate: Push to free list

### Stack Allocator
```cpp
class StackAllocator {
    struct Header { size_t size; size_t padding; };
    
public:
    void* allocate(size_t size, size_t alignment);
    Marker get_marker();
    void free_to_marker(Marker m);
};
```

**Algorithm:**
1. Store header before each allocation
2. Markers save current offset
3. Free to marker: offset = marker

### Free List Allocator
```cpp
class FreeListAllocator {
    struct Header { size_t size; Header* next; };
    Header* free_list_;
    
    void coalesce();  // Merge adjacent blocks
    
public:
    void* allocate(size_t size);        // First-fit
    void deallocate(void*);             // Add to free list + coalesce
};
```

**Algorithm:**
1. First-fit search through free list
2. Split block if too large
3. On deallocation, insert sorted by address
4. Coalesce adjacent free blocks

## 🚀 Usage Examples

### Linear Allocator - Frame Allocations
```cpp
LinearAllocator frame_alloc(1024 * 1024);  // 1 MB per frame

void game_frame() {
    // Allocate temporary data
    int* temp_data = static_cast<int*>(frame_alloc.allocate(1000 * sizeof(int)));
    
    // Use data...
    
    // At end of frame
    frame_alloc.reset();  // Instant cleanup!
}
```

### Pool Allocator - Object Pool
```cpp
struct Particle {
    float x, y, z;
    float vx, vy, vz;
};

PoolAllocator particle_pool(sizeof(Particle), 10000);

Particle* spawn_particle() {
    Particle* p = static_cast<Particle*>(particle_pool.allocate());
    new (p) Particle();  // Placement new
    return p;
}

void destroy_particle(Particle* p) {
    p->~Particle();
    particle_pool.deallocate(p);
}
```

### Stack Allocator - Scope-based
```cpp
StackAllocator stack_alloc(64 * 1024);  // 64 KB

void function() {
    auto marker = stack_alloc.get_marker();
    
    // Allocate in this scope
    int* data = static_cast<int*>(stack_alloc.allocate(100 * sizeof(int)));
    
    // Use data...
    
    // Automatic cleanup
    stack_alloc.free_to_marker(marker);
}
```

### Free List Allocator - General Purpose
```cpp
FreeListAllocator general_alloc(1024 * 1024);  // 1 MB

void* ptr1 = general_alloc.allocate(100);
void* ptr2 = general_alloc.allocate(500);
void* ptr3 = general_alloc.allocate(250);

general_alloc.deallocate(ptr2);  // Any order!
general_alloc.deallocate(ptr1);
general_alloc.deallocate(ptr3);
```

## 📊 Performance Comparison

| Allocator | Allocation | Deallocation | Fragmentation | Best For |
|-----------|-----------|--------------|---------------|----------|
| **Linear** | O(1) ⚡ | N/A (bulk reset) | None | Frame data |
| **Pool** | O(1) ⚡ | O(1) ⚡ | None | Same-size objects |
| **Stack** | O(1) ⚡ | O(1) (marker) ⚡ | Low | Scoped data |
| **Free List** | O(n) | O(n) + coalesce | Can occur | General use |
| **malloc** | O(n) | O(n) | Can occur | Baseline |

### Benchmark Results
From `allocator_example.cpp` (10,000 allocations):

- **Linear**: ~0 μs average (too fast to measure!)
- **Pool**: ~0.0049 μs average
- **malloc**: ~0.023 μs average (baseline)

**Linear and Pool allocators are ~5-10x faster than malloc!**

## 🔍 When to Use Each

### Use Linear Allocator When:
- ✅ Allocations have same lifetime
- ✅ All data freed at once
- ✅ Ultra-fast allocation needed
- ✅ Examples: per-frame data, parser temp data

### Use Pool Allocator When:
- ✅ Objects all same size
- ✅ Frequent alloc/dealloc
- ✅ Predictable capacity
- ✅ Examples: particles, entities, bullets

### Use Stack Allocator When:
- ✅ Scope-based lifetimes
- ✅ LIFO allocation pattern
- ✅ Natural nesting
- ✅ Examples: recursion, local temp data

### Use Free List Allocator When:
- ✅ Variable sizes needed
- ✅ Arbitrary deallocation order
- ✅ Long-lived objects
- ✅ Examples: general allocations, caches

## 💡 Best Practices

### 1. Choose the Right Allocator
Match allocator to usage pattern for best performance.

### 2. Pre-allocate Capacity
Size allocators appropriately upfront to avoid out-of-memory.

### 3. Combine Allocators
Use different allocators for different subsystems:
```cpp
LinearAllocator frame_alloc(1MB);       // Per-frame temps
PoolAllocator entity_pool(sizeof(Entity), 10000);  // Entities
FreeListAllocator level_alloc(100MB);   // Level data
```

### 4. Profile Memory Usage
Measure actual allocation patterns before optimizing.

### 5. RAII Wrappers
Wrap allocators in RAII types for automatic cleanup:
```cpp
struct FrameScope {
    LinearAllocator& alloc;
    FrameScope(LinearAllocator& a) : alloc(a) {}
    ~FrameScope() { alloc.reset(); }
};
```

## 🎓 Advanced Topics

### Alignment
All allocators support custom alignment:
```cpp
void* ptr = allocator.allocate(size, 16);  // 16-byte aligned
```

### Memory Tracking
Add instrumentation for debugging:
```cpp
class TrackedLinearAllocator : public LinearAllocator {
    size_t peak_used_ = 0;
    
public:
    void* allocate(size_t size, size_t align) {
        void* ptr = LinearAllocator::allocate(size, align);
        peak_used_ = std::max(peak_used_, used());
        return ptr;
    }
    
    size_t peak_usage() const { return peak_used_; }
};
```

### Thread Safety
Current implementations are NOT thread-safe. For multi-threading:
- Use per-thread allocators (see Arena Allocator)
- Add mutex protection
- Use atomic operations for counters

### Memory Debugging
Add guards and magic numbers:
```cpp
struct Header {
    uint32_t magic = 0xDEADBEEF;
    size_t size;
    // ... allocation data ...
};
```

## 🐛 Common Pitfalls

### 1. Using Wrong Allocator
```cpp
// ❌ DON'T: Use pool for variable sizes
PoolAllocator pool(64, 100);
void* big = pool.allocate();  // Can't allocate 128 bytes!

// ✅ DO: Match allocator to need
FreeListAllocator alloc(1024);
void* big = alloc.allocate(128);  // Works!
```

### 2. Forgetting to Reset Linear Allocator
```cpp
// ❌ DON'T: Forget to reset
for (int frame = 0; frame < 1000; ++frame) {
    frame_alloc.allocate(1000);  // Will run out!
}

// ✅ DO: Reset each frame
for (int frame = 0; frame < 1000; ++frame) {
    frame_alloc.allocate(1000);
    frame_alloc.reset();  // Reclaim memory
}
```

### 3. Pool Exhaustion
```cpp
// ❌ DON'T: Allocate without limit
PoolAllocator pool(64, 100);  // Only 100 blocks!
for (int i = 0; i < 200; ++i) {
    pool.allocate();  // Fails at 101!
}

// ✅ DO: Check for nullptr
void* ptr = pool.allocate();
if (!ptr) {
    // Handle out of memory
}
```

## 📈 Real-World Applications

### Game Engines
```cpp
// Per-frame temporary allocations
LinearAllocator frame_alloc(10MB);

// Entity pool
PoolAllocator entity_pool(sizeof(Entity), MAX_ENTITIES);

// Level data
FreeListAllocator level_alloc(100MB);
```

### Parsers/Compilers
```cpp
// AST nodes
PoolAllocator ast_pool(sizeof(ASTNode), 100000);

// String intern table
FreeListAllocator string_alloc(1MB);

// Per-file temp data
LinearAllocator file_alloc(64KB);
```

### Physics Engines
```cpp
// Contact points (per frame)
LinearAllocator contact_alloc(1MB);

// Rigid bodies (persistent)
PoolAllocator body_pool(sizeof(RigidBody), 10000);
```

## 🔗 API Reference

### LinearAllocator
| Method | Description | Time |
|--------|-------------|------|
| `LinearAllocator(size)` | Create with capacity | O(1) |
| `allocate(size, align)` | Allocate aligned memory | O(1) |
| `reset()` | Reclaim all memory | O(1) |
| `used()` | Bytes allocated | O(1) |
| `available()` | Bytes remaining | O(1) |

### PoolAllocator
| Method | Description | Time |
|--------|-------------|------|
| `PoolAllocator(block_size, count)` | Create pool | O(n) |
| `allocate()` | Allocate block | O(1) |
| `deallocate(ptr)` | Free block | O(1) |

### StackAllocator
| Method | Description | Time |
|--------|-------------|------|
| `StackAllocator(size)` | Create with capacity | O(1) |
| `allocate(size, align)` | Allocate aligned | O(1) |
| `get_marker()` | Save position | O(1) |
| `free_to_marker(m)` | Bulk free | O(1) |

### FreeListAllocator
| Method | Description | Time |
|--------|-------------|------|
| `FreeListAllocator(size)` | Create with capacity | O(1) |
| `allocate(size)` | First-fit allocation | O(n) |
| `deallocate(ptr)` | Free + coalesce | O(n) |

## 🏃 Building and Running

```bash
# Compile examples
g++ -std=c++14 -O2 allocator/allocator_example.cpp -o allocator_example

# Run examples
./allocator_example

# Run tests
g++ -std=c++14 -O2 tests/allocator_test.cpp -o allocator_test
./allocator_test
```

## 📖 See Also

- **Arena Allocator**: Thread-local memory pools (next implementation)
- **Stack/Queue**: Containers that can use custom allocators
- **Smart Pointers**: RAII for automatic memory management

---

**Summary**: Custom allocators provide 5-10x performance improvements for specific workloads. Choose the allocator that matches your allocation pattern for maximum benefit!

