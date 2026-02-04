# PriorityQueue Implementation

Binary heap-based priority queue providing efficient access to the highest (or lowest) priority element.

## Features
- **Binary Heap**: Efficient O(log n) operations
- **Max Heap by Default**: Largest element at top
- **Customizable**: Use any comparator (min heap, custom priority)
- **O(1) Access**: Constant time access to top element
- **O(log n) Insert/Remove**: Logarithmic time modifications
- **Container Adapter**: Built on top of vector by default

## Quick Example

```cpp
// Max heap (default)
PriorityQueue<int> pq;
pq.push(30);
pq.push(10);
pq.push(50);

std::cout << pq.top();  // 50 (largest)
pq.pop();
std::cout << pq.top();  // 30

// Min heap
PriorityQueue<int, std::vector<int>, std::greater<int>> min_pq;
min_pq.push(30);
min_pq.push(10);
min_pq.push(50);

std::cout << min_pq.top();  // 10 (smallest)
```

## Operations

- `push(value)` - Insert element (O(log n))
- `pop()` - Remove top element (O(log n))
- `top()` - Access top element (O(1))
- `emplace(args...)` - Construct element in-place (O(log n))
- `size()`, `empty()` - Capacity queries (O(1))
- `swap(other)` - Swap contents (O(1))

## Algorithm Details

### Binary Heap Structure
- Complete binary tree stored in a vector
- Parent at index `i`, children at `2i+1` and `2i+2`
- Heap property: Parent ≥ children (max heap)

### Operations

**Insert (Heapify Up):**
1. Add element at end
2. Bubble up while violating heap property
3. Time: O(log n)

**Remove (Heapify Down):**
1. Replace root with last element
2. Bubble down while violating heap property
3. Time: O(log n)

**Build Heap (Floyd's Algorithm):**
1. Start from last non-leaf node
2. Heapify down all nodes
3. Time: O(n) - optimal!

## Use Cases

### 1. Task Scheduling
```cpp
struct Task {
    std::string name;
    int priority;
};

auto cmp = [](const Task& a, const Task& b) {
    return a.priority < b.priority;
};

PriorityQueue<Task, std::vector<Task>, decltype(cmp)> tasks(cmp);
tasks.push({"urgent", 10});
tasks.push({"normal", 5});

// Process highest priority first
while (!tasks.empty()) {
    process(tasks.top());
    tasks.pop();
}
```

### 2. Finding Median (Two Heaps)
```cpp
PriorityQueue<int> lower_half;  // max heap
PriorityQueue<int, std::vector<int>, std::greater<int>> upper_half;  // min heap

// Median is either top of lower_half or average of both tops
```

### 3. Top K Elements
```cpp
// Keep min heap of size k
PriorityQueue<int, std::vector<int>, std::greater<int>> pq;
for (int num : large_array) {
    pq.push(num);
    if (pq.size() > k) pq.pop();
}
// pq now contains k largest elements
```

### 4. Merge K Sorted Lists
```cpp
struct Element {
    int value;
    int list_id;
};

auto cmp = [](const Element& a, const Element& b) {
    return a.value > b.value;  // min heap
};

PriorityQueue<Element, std::vector<Element>, decltype(cmp)> pq(cmp);
// Add first element from each list, then keep popping and adding
```

### 5. Dijkstra's Algorithm
```cpp
struct Node {
    int id;
    int distance;
};

auto cmp = [](const Node& a, const Node& b) {
    return a.distance > b.distance;
};

PriorityQueue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
// Process nodes in order of shortest distance
```

### 6. Heap Sort
```cpp
PriorityQueue<int> pq;
for (int x : unsorted) pq.push(x);

std::vector<int> sorted;
while (!pq.empty()) {
    sorted.push_back(pq.top());
    pq.pop();
}
// sorted contains elements in descending order
```

## Performance

| Operation | Time Complexity | Space |
|-----------|----------------|-------|
| push | O(log n) | O(1) |
| pop | O(log n) | O(1) |
| top | O(1) | O(1) |
| emplace | O(log n) | O(1) |
| size/empty | O(1) | O(1) |
| build from range | O(n) | O(n) |

## Implementation Highlights

### Heap Property Maintenance
- **Heapify Up**: After insertion, bubble element up to correct position
- **Heapify Down**: After removal, bubble element down to correct position
- **Complete Binary Tree**: Always maintains complete tree structure

### Floyd's Build Heap Algorithm
- Constructs heap from unsorted data in O(n) time
- More efficient than inserting elements one-by-one
- Used in range constructor

### Container Adapter Pattern
- Uses underlying container (default: vector) for storage
- Can use any random-access container
- Provides heap semantics on top

## Comparator Examples

```cpp
// Min heap
PriorityQueue<int, std::vector<int>, std::greater<int>> min_pq;

// Custom comparator
auto cmp = [](const Point& a, const Point& b) {
    return a.distance() < b.distance();
};
PriorityQueue<Point, std::vector<Point>, decltype(cmp)> pq(cmp);

// Function object
struct CompareDistance {
    bool operator()(const Node& a, const Node& b) const {
        return a.dist > b.dist;  // min heap by distance
    }
};
PriorityQueue<Node, std::vector<Node>, CompareDistance> pq;
```

## Advanced Usage

### Emplace for In-Place Construction
```cpp
struct Task {
    std::string name;
    int priority;
    Task(std::string n, int p) : name(n), priority(p) {}
};

PriorityQueue<Task> pq;
pq.emplace("task1", 5);  // Constructs Task directly in queue
```

### Building from Existing Container
```cpp
std::vector<int> data = {5, 2, 8, 1, 9};
PriorityQueue<int> pq(std::less<int>(), data);  // O(n) build
```

### Copy and Move Semantics
```cpp
PriorityQueue<int> pq1;
pq1.push(10);

PriorityQueue<int> pq2 = pq1;          // Copy
PriorityQueue<int> pq3 = std::move(pq1);  // Move
```

## Complexity Analysis

**Why O(log n) for insert/remove?**
- Binary heap has height log₂(n)
- Heapify operations traverse at most one path
- Each level takes O(1) comparisons

**Why O(n) for build heap?**
- Floyd's algorithm processes each level optimally
- Most nodes are near bottom (don't move far)
- Mathematical analysis: Σ (n/2^(h+1)) * h = O(n)

## Common Patterns

### Priority-Based Processing
```cpp
while (!pq.empty()) {
    auto item = pq.top();
    pq.pop();
    process(item);
}
```

### Maintaining K Best Elements
```cpp
if (pq.size() < k) {
    pq.push(element);
} else if (element > pq.top()) {  // for max-k with min-heap
    pq.pop();
    pq.push(element);
}
```

### Streaming Data
```cpp
for (const auto& item : stream) {
    pq.push(item);
    if (condition) {
        process(pq.top());
        pq.pop();
    }
}
```

## Testing

Run comprehensive test suite:
```bash
make test-priority-queue
# or
./build/priority_queue_test
```

All 20 tests cover:
- Basic operations
- Heap property verification
- Custom comparators
- Edge cases
- Move/copy semantics
- Exception handling

## Examples

See `priority_queue_example.cpp` for 10 comprehensive examples:
1. Basic max heap operations
2. Min heap with custom comparator
3. Range constructor
4. Custom types (task scheduling)
5. Emplace for in-place construction
6. Finding median with two heaps
7. Top K elements algorithm
8. Heap sort
9. Merging K sorted lists
10. Copy and move semantics

Run examples:
```bash
./build/priority_queue_example
```

## STL Compatibility

This implementation follows `std::priority_queue` interface:
- Same function names and semantics
- Compatible with STL containers and algorithms
- Drop-in replacement for most use cases

## When to Use

**Use PriorityQueue when:**
- Need efficient access to max/min element
- Processing elements by priority
- Implementing Dijkstra, Prim, or A* algorithms
- Finding top K elements
- Merging sorted sequences
- Managing event queues

**Don't use when:**
- Need to access arbitrary elements (use Set/Map)
- Need to iterate in order (use Set)
- Only need FIFO (use Deque)

## Related Containers

- **Set**: Ordered elements, allows iteration
- **Deque**: Both ends, but no priority
- **Vector**: Random access, but no heap structure

---

**Lines of Code**: 250+ (implementation) + 300+ (examples) + 180+ (tests)  
**Test Coverage**: 20/20 tests passing ✅  
**Algorithm**: Binary heap (Floyd's build-heap)  
**Complexity**: O(log n) push/pop, O(1) top, O(n) build

