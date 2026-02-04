# Deque Class Implementation

A complete implementation of a double-ended queue (deque) similar to `std::deque`.

## Overview

The `Deque` class template provides a dynamic array with efficient insertion and deletion at both ends, plus random access.

### Key Features

- **Double-Ended**: O(1) operations at both front and back
- **Random Access**: O(1) element access by index
- **Chunk-Based**: Memory organized in fixed-size chunks (no reallocation of existing elements)
- **RAII**: Automatic cleanup, no memory leaks
- **Move Semantics**: Efficient transfers of ownership
- **Exception Safety**: Strong exception guarantees
- **Rich API**: Compatible with std::deque interface

## Quick Start

```cpp
#include "deque/deque.hpp"

// Create and populate deque
Deque<int> numbers = {1, 2, 3, 4, 5};

// Add to both ends
numbers.push_back(6);     // Add to back
numbers.push_front(0);    // Add to front

// Access elements
int first = numbers.front();
int last = numbers.back();
int middle = numbers[3];

// Remove from both ends
numbers.pop_front();
numbers.pop_back();

// Iterate
for (int n : numbers) {
    std::cout << n << " ";
}
```

## Construction

### Default Constructor
```cpp
Deque<int> dq;  // Empty deque
```
- Size: 0
- No memory allocated except initial chunk map

### Size Constructor
```cpp
Deque<int> dq(10);  // 10 default-constructed elements
```
- Creates n default-constructed elements
- Efficient for pre-sizing

### Size + Value Constructor
```cpp
Deque<int> dq(5, 42);  // {42, 42, 42, 42, 42}
```
- Creates n copies of value
- Useful for initialization

### Initializer List
```cpp
Deque<int> dq = {1, 2, 3, 4, 5};
```
- Convenient syntax for literals
- Elements added in order

### Copy Constructor
```cpp
Deque<int> dq2(dq1);  // Deep copy
```
- Creates independent copy
- O(n) time complexity

### Move Constructor
```cpp
Deque<int> dq2(std::move(dq1));  // Steals resources
```
- O(1) time complexity
- Original deque becomes empty

## Architecture

### Chunk-Based Storage

```
Map (Array of Chunk Pointers):
┌────┬────┬────┬────┬────┬────┬────┐
│ ∅  │ ∅  │ C0 │ C1 │ C2 │ ∅  │ ∅  │
└────┴────┴────┴────┴────┴────┴────┘
           ↓    ↓    ↓
        Chunk  Chunk  Chunk
        (16 T) (16 T) (16 T)
        
Elements stored in chunks of CHUNK_SIZE (16)
Map grows when needed (no element reallocation)
```

### Memory Layout
- **Map**: Array of pointers to chunks
- **Chunks**: Fixed-size arrays (16 elements each)
- **No Reallocation**: Existing elements never move
- **Growth**: Add new chunks as needed

## Element Access

### Subscript Operator []
```cpp
int x = dq[2];     // No bounds checking
dq[0] = 100;       // Direct access
```
- **Fast**: Calculated access to correct chunk
- **Unsafe**: No bounds checking
- Use when index is guaranteed valid

### at() Method
```cpp
try {
    int x = dq.at(100);  // Throws if out of range
} catch (const std::out_of_range& e) {
    // Handle error
}
```
- **Safe**: Throws on invalid index
- **Slower**: Has bounds check
- Use when safety matters

### Front and Back
```cpp
int first = dq.front();
int last = dq.back();

dq.front() = 10;
dq.back() = 20;
```
- Access first/last element
- O(1) performance
- Undefined if deque is empty

## Double-Ended Operations

### Push Operations
```cpp
dq.push_front(1);              // Add to front - O(1)
dq.push_back(2);               // Add to back - O(1)
dq.push_front(std::move(obj)); // Move version
```
- Both ends are O(1)
- No reallocation of existing elements
- New chunks allocated only when needed

### Pop Operations
```cpp
dq.pop_front();  // Remove from front - O(1)
dq.pop_back();   // Remove from back - O(1)
```
- Both ends are O(1)
- Elements properly destroyed
- No automatic shrinking

### Emplace Operations
```cpp
dq.emplace_front(args...);  // Construct in-place at front
dq.emplace_back(args...);   // Construct in-place at back
```
- Constructs element directly
- Avoids temporary objects
- More efficient than push

## Capacity Management

### Size vs Chunks

```cpp
Deque<int> dq;
dq.push_back(1);   // May allocate chunk
dq.push_back(2);   // Uses existing chunk
```

- **Size**: Number of elements
- **Chunks**: Allocated in blocks of 16
- No "capacity" concept (unlike vector)

### empty() and size()
```cpp
if (dq.empty()) {
    // Deque is empty
}

size_t count = dq.size();  // Current element count
```

## Modifiers

### resize()
```cpp
dq.resize(10);      // Resize to 10, default-construct new elements
dq.resize(20, 99);  // Resize to 20, fill new elements with 99
```
- Grows or shrinks to specified size
- Can add at either end
- O(n) where n is the change in size

### clear()
```cpp
dq.clear();  // Remove all elements
```
- Destroys all elements
- Size becomes 0
- Chunks remain allocated

### swap()
```cpp
dq1.swap(dq2);  // Swap contents
```
- Exchanges contents
- O(1) - just swaps internal pointers
- No element copies

## Iterators

### Basic Iteration
```cpp
// Forward iteration
for (auto it = dq.begin(); it != dq.end(); ++it) {
    std::cout << *it << " ";
}

// Range-based for
for (int x : dq) {
    std::cout << x << " ";
}

// Modify via iterator
for (int& x : dq) {
    x *= 2;
}
```

### Random Access
```cpp
auto it = dq.begin();
it += 5;                    // Jump forward 5 positions
auto mid = dq.begin() + dq.size() / 2;  // Middle element
```

### Iterator Arithmetic
```cpp
auto it1 = dq.begin();
auto it2 = dq.end();
auto distance = it2 - it1;  // Number of elements
```

## Comparisons

### Equality
```cpp
Deque<int> dq1 = {1, 2, 3};
Deque<int> dq2 = {1, 2, 3};
Deque<int> dq3 = {1, 2, 4};

bool b1 = (dq1 == dq2);  // true
bool b2 = (dq1 != dq3);  // true
```
- Element-wise comparison
- Must have same size and elements

### Relational
```cpp
bool b1 = (dq1 < dq3);   // Lexicographic comparison
bool b2 = (dq1 <= dq2);
bool b3 = (dq3 > dq1);
bool b4 = (dq2 >= dq1);
```
- Lexicographic ordering
- Like string comparison

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Construction | O(n) | Where n is initial size |
| Element access [] | O(1) | Simple calculation |
| push_front | O(1) | May allocate chunk |
| push_back | O(1) | May allocate chunk |
| pop_front | O(1) | |
| pop_back | O(1) | |
| insert at middle | O(n) | Must shift elements |
| erase | O(n) | Must shift elements |
| resize | O(n) | If changing size |
| clear | O(n) | Must destroy elements |
| swap | O(1) | Just swaps pointers |

## Common Patterns

### Queue (FIFO)
```cpp
Deque<Task> queue;

// Enqueue
queue.push_back(task);

// Dequeue
Task t = queue.front();
queue.pop_front();
```

### Stack (LIFO)
```cpp
Deque<int> stack;

// Push
stack.push_back(item);

// Pop
int item = stack.back();
stack.pop_back();
```

### Sliding Window
```cpp
Deque<int> window;
const int WINDOW_SIZE = 3;

for (int x : data) {
    window.push_back(x);
    
    if (window.size() > WINDOW_SIZE) {
        window.pop_front();
    }
    
    // Process current window
}
```

### Double-Ended Buffer
```cpp
Deque<Event> buffer;

// Add to appropriate end based on priority
if (high_priority) {
    buffer.push_front(event);
} else {
    buffer.push_back(event);
}
```

## Comparison with Vector

### Deque Advantages
- ✅ O(1) insertion/deletion at front
- ✅ No reallocation of existing elements
- ✅ Better for large objects (no copies during growth)
- ✅ Good for queue/buffer use cases

### Vector Advantages
- ✅ Contiguous memory (better cache locality)
- ✅ Smaller memory footprint
- ✅ Simpler implementation
- ✅ Slightly faster random access

### When to Use Deque
- Need frequent front insertions/deletions
- Want to avoid reallocation overhead
- Implementing queue or double-ended buffer
- Large objects that are expensive to move

### When to Use Vector
- Only need back operations
- Want maximum cache efficiency
- Small to medium sized containers
- Random access is critical

## Memory Management

### Chunk Allocation
```cpp
// Chunks allocated as needed
Deque<int> dq;
// Map allocated, one chunk created

dq.push_back(1);  // Uses existing chunk
// ... (15 more push_back calls)
dq.push_back(17); // Allocates new chunk
```

### No Reallocation
```cpp
Deque<int> dq;
dq.push_back(1);
int* ptr = &dq[0];  // Get pointer to element

dq.push_back(2);
dq.push_back(3);
// ptr still valid! Elements never move
```

## Exception Safety

### Strong Guarantee
Most operations provide strong exception guarantee:
- If exception thrown, deque unchanged
- Examples: push_back, push_front, insert

### Basic Guarantee
Some operations provide basic guarantee:
- Deque valid but state may change
- Example: some complex operations

### No-throw Guarantee
Some operations never throw:
- swap, move constructor, move assignment (if T's move doesn't throw)

## Best Practices

### ✅ Do
- Use deque for double-ended operations
- Prefer emplace_back/emplace_front for efficiency
- Use move semantics for large objects
- Pass by const reference when not transferring ownership

### ❌ Don't
- Don't use deque if you only need back operations (use vector)
- Don't assume contiguous memory
- Don't manually manage memory (RAII handles it)
- Don't hold pointers to elements across insertions at opposite end

## Thread Safety

### ❌ Not Thread-Safe
- Concurrent modifications are unsafe
- Concurrent reads are safe
- Use external synchronization

```cpp
// UNSAFE
thread1: dq.push_back(1);
thread2: dq.push_front(2);

// SAFE
thread1: int x = dq[0];
thread2: int y = dq[1];
```

## Use Cases

### 1. Task Queue
```cpp
Deque<Task> tasks;
tasks.push_back(normal_task);      // Regular priority
tasks.push_front(urgent_task);     // High priority
```

### 2. Undo/Redo Buffer
```cpp
Deque<Action> history;
history.push_back(action);         // Add action
Action last = history.back();      // Undo
history.pop_back();
```

### 3. Sliding Window Algorithm
```cpp
Deque<int> window;
for (int x : stream) {
    window.push_back(x);
    if (window.size() > K) {
        window.pop_front();
    }
    // Process window
}
```

### 4. BFS Queue
```cpp
Deque<Node*> queue;
queue.push_back(root);

while (!queue.empty()) {
    Node* node = queue.front();
    queue.pop_front();
    
    // Add children to back
    for (auto child : node->children) {
        queue.push_back(child);
    }
}
```

## Files

- **deque.hpp**: Implementation (header-only)
- **deque_example.cpp**: 15 comprehensive examples
- **deque_test.cpp**: Unit tests

## See Also

- **Vector**: For single-ended dynamic array
- **unique_ptr**: For single object ownership
- **shared_ptr**: For shared ownership
- **String**: For string handling

## Build and Run

```bash
# Build examples
make run-deque

# Run tests
make test-deque

# Build everything
make all
```

---

**Implementation Status**: ✅ Complete with chunk-based architecture
**Performance**: O(1) operations at both ends, O(1) random access
**Memory**: Efficient chunk allocation, no element reallocation

