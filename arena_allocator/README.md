# Thread Arena Allocator

A high-performance, per-thread memory pool allocator designed for fast, lock-free allocation in multi-threaded applications.

## Overview

The Thread Arena Allocator provides each thread with its own memory arena, eliminating contention and enabling blazing-fast allocations. Perfect for scenarios where temporary memory is needed during request processing, frame rendering, or compilation.

## Key Features

- **Per-thread Arenas**: Each thread gets its own memory pool
- **Lock-free Allocation**: No locks for thread's own arena
- **Automatic Management**: Arenas created on first use
- **RAII Support**: Scoped arena with automatic reset
- **Type Safety**: Typed allocator wrapper
- **Arena Containers**: Custom containers using arena allocation
- **Statistics**: Per-thread and global memory tracking

## Architecture

```
Thread 1 → Arena 1 (1 MB)  ────┐
Thread 2 → Arena 2 (1 MB)  ────┤
Thread 3 → Arena 3 (1 MB)  ────┼─→ ThreadArenaAllocator
Thread 4 → Arena 4 (1 MB)  ────┘
     ↓
[Fast, lock-free allocation for owning thread]
```

### Components

1. **Arena**: Single thread's memory pool
   - Fixed-size buffer
   - Bump pointer allocation
   - Atomic offset counter
   - Reset capability

2. **ThreadArenaAllocator**: Manages per-thread arenas
   - Creates arenas on demand
   - Thread-safe arena management
   - Global statistics
   - Reset all arenas

3. **ScopedArena**: RAII helper
   - Automatic reset on destruction
   - Convenient for per-request allocation

4. **ArenaVector**: Custom vector using arena
   - No individual deallocations
   - Fast growth
   - Iterator support

## Usage

### Basic Allocation

```cpp
#include "arena_allocator.hpp"

ThreadArenaAllocator allocator(1024 * 1024);  // 1 MB per thread

// Allocate memory
int* numbers = static_cast<int*>(allocator.allocate(10 * sizeof(int)));
for (int i = 0; i < 10; ++i) {
    numbers[i] = i * 10;
}

// Check statistics
auto stats = allocator.get_thread_stats();
std::cout << "Used: " << stats.used << " bytes\n";
std::cout << "Available: " << stats.available << " bytes\n";
```

### Multi-threaded Allocation

```cpp
ThreadArenaAllocator allocator(1024 * 128);  // 128 KB per thread

std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&]() {
        // Each thread gets its own arena
        for (int j = 0; j < 100; ++j) {
            void* ptr = allocator.allocate(64);
            // Use ptr...
        }
    });
}

for (auto& t : threads) t.join();

// Check global stats
auto global = allocator.get_global_stats();
std::cout << "Total arenas: " << global.num_arenas << "\n";
std::cout << "Total used: " << global.total_used << " bytes\n";
```

### Scoped Arena (RAII)

```cpp
ThreadArenaAllocator allocator(1024 * 64);

{
    ScopedArena scope(allocator);
    
    // Allocate during request/frame
    int* data = static_cast<int*>(scope.allocate(100 * sizeof(int)));
    
    // Or use create for construction
    MyObject* obj = scope.create<MyObject>(arg1, arg2);
    
}  // Arena automatically resets here!

// Memory is reclaimed, ready for next request/frame
```

### Arena Vector

```cpp
ThreadArenaAllocator allocator(1024 * 64);

ArenaVector<int> vec(allocator);

for (int i = 0; i < 100; ++i) {
    vec.push_back(i * 2);
}

// Access elements
std::cout << vec[50] << "\n";

// Iterate
for (int val : vec) {
    std::cout << val << " ";
}
```

### Typed Allocator

```cpp
ThreadArenaAllocator allocator(1024);
TypedArenaAllocator<MyClass> typed(allocator);

// Allocate array of objects
MyClass* objects = typed.allocate(10);

// Construct in place
for (int i = 0; i < 10; ++i) {
    typed.construct(&objects[i], arg1, arg2);
}

// Use objects...

// Destroy (but memory stays in arena)
for (int i = 0; i < 10; ++i) {
    typed.destroy(&objects[i]);
}
```

## API Reference

### ThreadArenaAllocator

```cpp
// Constructor
explicit ThreadArenaAllocator(size_t arena_size = 1024 * 1024);

// Allocation
void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

// Reset
void reset_thread_arena();  // Reset current thread's arena
void reset_all();           // Reset all arenas

// Statistics
struct ThreadStats {
    size_t used;
    size_t available;
    size_t capacity;
};
ThreadStats get_thread_stats() const;

struct GlobalStats {
    size_t num_arenas;
    size_t total_capacity;
    size_t total_used;
    size_t total_available;
};
GlobalStats get_global_stats() const;

size_t num_arenas() const;
```

### Arena

```cpp
Arena(size_t size);

void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));
void reset();

size_t used() const;
size_t available() const;
size_t capacity() const;
```

### ScopedArena

```cpp
explicit ScopedArena(ThreadArenaAllocator& allocator);
~ScopedArena();  // Resets arena

void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

template<typename T, typename... Args>
T* create(Args&&... args);
```

### ArenaVector<T>

```cpp
explicit ArenaVector(ThreadArenaAllocator& arena, size_t initial_capacity = 16);

void push_back(const T& value);
T& operator[](size_t index);
size_t size() const;
size_t capacity() const;
bool empty() const;

T* begin();
T* end();
```

## Performance

**Benchmark**: 8 threads, 10,000 allocations each (64 bytes)

| Allocator Type      | Time    | Throughput        |
|---------------------|---------|-------------------|
| Arena Allocator     | 4 ms    | 20,000,000 ops/s  |
| malloc (baseline)   | 1 ms    | 80,000,000 ops/s  |

**Note**: Arena allocator shows its strength when:
- Allocations are temporary (per-request/frame)
- Bulk deallocation is acceptable
- Thread contention would be high with malloc

**Memory Usage**: Each thread uses fixed arena size (e.g., 1 MB). No overhead per allocation.

## Use Cases

### 1. Web Server Request Handler
```cpp
void handle_request(ThreadArenaAllocator& allocator, Request& req) {
    ScopedArena scope(allocator);
    
    // Allocate request-specific data
    auto* parse_buffer = scope.allocate(4096);
    auto* response_data = scope.allocate(8192);
    
    // Process request...
    
}  // All memory reclaimed automatically
```

### 2. Game Engine Frame
```cpp
void render_frame(ThreadArenaAllocator& allocator) {
    ScopedArena scope(allocator);
    
    // Temporary frame data
    ArenaVector<RenderCommand> commands(allocator);
    
    // Build render list...
    
}  // Frame memory reclaimed
```

### 3. Compiler/Parser
```cpp
void compile_file(ThreadArenaAllocator& allocator, const char* filename) {
    ScopedArena scope(allocator);
    
    // Allocate AST nodes, symbol tables, etc.
    ASTNode* root = scope.create<ASTNode>();
    
    // Parse and compile...
    
}  // All compilation temporaries freed
```

### 4. Database Query Processing
```cpp
QueryResult execute_query(ThreadArenaAllocator& allocator, Query& q) {
    ScopedArena scope(allocator);
    
    // Intermediate buffers
    auto* row_buffer = scope.allocate(query.row_size * 1000);
    
    // Process query...
    
    return result;  // Intermediate data freed
}
```

## Design Decisions

### Why Per-Thread Arenas?

1. **No Locks**: Each thread allocates from its own arena without synchronization
2. **Cache Locality**: Thread's allocations stay in its own cache lines
3. **Simple**: No complex memory management

### Why Not Individual Deallocation?

1. **Speed**: Bump pointer allocation is extremely fast
2. **Simplicity**: No free lists, no fragmentation
3. **Use Case**: Perfect for temporary allocations with bulk deallocation

### When NOT to Use

- Long-lived allocations
- Unpredictable allocation patterns
- When individual deallocation is required
- Memory is extremely constrained

## Implementation Details

### Allocation Strategy

```
Arena Buffer: [----used----|----available----]
                            ↑
                         offset
```

1. Calculate alignment padding
2. Check if space available
3. Bump offset
4. Return pointer

### Thread Safety

- Arena creation: Protected by mutex
- Allocation: Lock-free for owning thread
- Global operations (reset_all, stats): Protected by mutex

### Memory Layout

```
Allocated Objects:
[Object1][padding][Object2][padding][Object3]...
```

Alignment padding ensures proper alignment for each allocation.

## Testing

Run the test suite:
```bash
make test-arena-allocator
```

Run examples:
```bash
make run-arena-allocator
```

**Test Coverage**: 45 tests covering:
- Basic allocation
- Alignment (16, 32, 64 bytes)
- Arena exhaustion
- Reset functionality
- Multi-threaded allocation
- Scoped arena
- Arena vector
- Typed allocator
- Global statistics
- Concurrent access

## Comparison with Other Allocators

| Feature                  | Arena | Pool | FreeList | malloc |
|-------------------------|-------|------|----------|--------|
| Per-thread              | ✅    | ❌   | ❌       | ❌     |
| Lock-free (same thread) | ✅    | ❌   | ❌       | ❌     |
| Individual dealloc      | ❌    | ✅   | ✅       | ✅     |
| Fragmentation           | None  | None | Some     | Some   |
| Speed (allocation)      | Fast  | Fast | Medium   | Medium |
| Bulk deallocation       | ✅    | ❌   | ❌       | ❌     |

## Advanced Usage

### Custom Arena Size

```cpp
// Small arenas for low-memory systems
ThreadArenaAllocator small_allocator(64 * 1024);  // 64 KB

// Large arenas for high-throughput systems
ThreadArenaAllocator large_allocator(16 * 1024 * 1024);  // 16 MB
```

### Nested Scopes

```cpp
ScopedArena outer(allocator);
{
    ScopedArena inner(allocator);
    // Allocations...
}  // Inner scope resets arena
// Outer scope allocations lost!
```

**Warning**: Only use one ScopedArena at a time per thread.

### Arena Reuse Pattern

```cpp
ThreadArenaAllocator allocator(1024 * 1024);

while (process_requests()) {
    ScopedArena scope(allocator);
    // Process request using scope
    // Arena resets after each request
}
```

## Limitations

1. **No Individual Deallocation**: Cannot free individual allocations
2. **Fixed Arena Size**: Arena size set at creation
3. **Memory Overhead**: Each thread reserves full arena size
4. **Arena Exhaustion**: Allocation fails when arena full

## Best Practices

1. **Choose Appropriate Size**: Based on typical usage patterns
2. **Use Scoped Arenas**: For automatic cleanup
3. **Monitor Statistics**: Track arena utilization
4. **Reset Regularly**: In per-request/frame scenarios
5. **Avoid Long-Lived Data**: Arena not suitable for persistent data

## Thread Safety Guarantees

- **Allocation (same thread)**: Lock-free ✅
- **Allocation (different threads)**: Thread-safe (separate arenas) ✅
- **Reset current arena**: Thread-safe ✅
- **Reset all arenas**: Thread-safe (mutex protected) ✅
- **Statistics**: Thread-safe ✅

## License

Part of the Custom STL Implementation Project

