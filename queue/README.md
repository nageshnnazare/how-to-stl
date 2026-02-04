# Queue Implementation

FIFO (First In, First Out) container adapter providing queue operations.

## Features
- **FIFO Access**: First element added is first to be removed
- **O(1) Operations**: Constant time push, pop, front, and back
- **Container Adapter**: Built on deque (or list)
- **Type Safe**: Templated for any type
- **STL Compatible**: Same interface as std::queue

## Quick Example

```cpp
Queue<int> q;
q.push(10);
q.push(20);
q.push(30);

std::cout << q.front();  // 10 (first in)
q.pop();
std::cout << q.front();  // 20
```

## Operations

- `push(value)` - Add element to back (O(1))
- `pop()` - Remove front element (O(1))
- `front()` - Access front element (O(1))
- `back()` - Access back element (O(1))
- `emplace(args...)` - Construct element in-place (O(1))
- `size()`, `empty()` - Capacity queries (O(1))
- `swap(other)` - Swap contents (O(1))

## Use Cases

### 1. Task Scheduling
```cpp
Queue<Task> task_queue;

void schedule_task(Task t) {
    task_queue.push(t);
}

void process_tasks() {
    while (!task_queue.empty()) {
        Task t = task_queue.front();
        task_queue.pop();
        execute(t);
    }
}
```

### 2. Breadth-First Search (BFS)
```cpp
void bfs(int start, const Graph& graph) {
    Queue<int> q;
    std::vector<bool> visited(graph.size(), false);
    
    q.push(start);
    visited[start] = true;
    
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        process(node);
        
        for (int neighbor : graph[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }
}
```

### 3. Printer Queue
```cpp
Queue<PrintJob> print_queue;

void add_print_job(const PrintJob& job) {
    print_queue.push(job);
    std::cout << "Job queued: " << job.name << "\n";
}

void process_print_queue() {
    while (!print_queue.empty()) {
        PrintJob job = print_queue.front();
        print_queue.pop();
        print(job);
    }
}
```

### 4. Level Order Tree Traversal
```cpp
void level_order(TreeNode* root) {
    if (!root) return;
    
    Queue<TreeNode*> q;
    q.push(root);
    
    while (!q.empty()) {
        TreeNode* node = q.front();
        q.pop();
        
        std::cout << node->val << " ";
        
        if (node->left) q.push(node->left);
        if (node->right) q.push(node->right);
    }
}
```

### 5. Recent Calls Counter
```cpp
struct RecentCounter {
    Queue<int> calls;
    
    int ping(int t) {
        calls.push(t);
        // Remove calls older than 3000ms
        while (!calls.empty() && calls.front() < t - 3000) {
            calls.pop();
        }
        return calls.size();
    }
};
```

### 6. Josephus Problem
```cpp
int josephus(int n, int k) {
    Queue<int> q;
    for (int i = 1; i <= n; ++i) {
        q.push(i);
    }
    
    while (q.size() > 1) {
        // Move k-1 people to back
        for (int i = 0; i < k - 1; ++i) {
            q.push(q.front());
            q.pop();
        }
        // Eliminate kth person
        q.pop();
    }
    
    return q.front();  // Survivor
}
```

## Underlying Container

Queue can be built on different containers:

```cpp
// Default: deque (efficient at both ends)
Queue<int> q1;

// With list (stable pointers, no reallocation)
Queue<int, std::list<int>> q2;

// Note: Vector not recommended (O(n) pop_front)
```

## Performance

| Operation | Time | Space |
|-----------|------|-------|
| push | O(1) | O(1) |
| pop | O(1) | O(1) |
| front | O(1) | O(1) |
| back | O(1) | O(1) |
| empty/size | O(1) | O(1) |

## When to Use

**Use Queue when:**
- Need FIFO (First In, First Out) access
- Implementing BFS algorithms
- Task scheduling systems
- Producer-consumer patterns
- Level-order tree traversal
- Buffering data streams

**Don't use when:**
- Need LIFO (use Stack)
- Need random access (use Vector)
- Need priority-based access (use PriorityQueue)

## Stack vs Queue

```cpp
// Stack: LIFO
Stack<int> s;
s.push(1); s.push(2); s.push(3);
while (!s.empty()) {
    std::cout << s.top() << " ";  // 3 2 1
    s.pop();
}

// Queue: FIFO
Queue<int> q;
q.push(1); q.push(2); q.push(3);
while (!q.empty()) {
    std::cout << q.front() << " ";  // 1 2 3
    q.pop();
}
```

## Algorithm Applications

1. **BFS**: Graph traversal, shortest path
2. **Level Order**: Tree traversal by levels
3. **Scheduling**: Task queues, CPU scheduling
4. **Buffering**: I/O buffers, message queues
5. **Simulation**: Event queues, waiting lines
6. **Streaming**: Data processing pipelines

## Examples

See `queue_example.cpp` for 10 comprehensive examples:
1. Basic operations
2. Task scheduler
3. BFS traversal
4. Printer queue simulation
5. Binary tree level order
6. Sliding window (simplified)
7. Recent API calls counter
8. Josephus problem
9. Copy and move semantics
10. Stack vs Queue comparison

Run:
```bash
./build/queue_example
```

## Testing

Run test suite:
```bash
./build/queue_test
```

All 15 tests cover:
- Basic push/pop operations
- FIFO ordering
- Front and back access
- Copy/move semantics
- Comparison operators
- Edge cases

## Real-World Usage

**Operating Systems:**
- Process scheduling
- I/O request buffering

**Networking:**
- Packet routing
- Message queues

**Game Development:**
- Event handling
- Command buffering

**Web Servers:**
- Request queuing
- Rate limiting

---

**Lines of Code**: 230+ (implementation) + 310+ (examples) + 180+ (tests)  
**Test Coverage**: 15/15 tests passing ✅  
**Adapter Pattern**: Built on underlying container  
**Complexity**: O(1) for all operations
