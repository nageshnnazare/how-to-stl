# Stack Implementation

LIFO (Last In, First Out) container adapter providing stack operations.

## Features
- **LIFO Access**: Last element added is first to be removed
- **O(1) Operations**: Constant time push, pop, and top
- **Container Adapter**: Built on deque (or vector/list)
- **Type Safe**: Templated for any type
- **STL Compatible**: Same interface as std::stack

## Quick Example

```cpp
Stack<int> s;
s.push(10);
s.push(20);
s.push(30);

std::cout << s.top();  // 30 (last in)
s.pop();
std::cout << s.top();  // 20
```

## Operations

- `push(value)` - Add element to top (O(1))
- `pop()` - Remove top element (O(1))
- `top()` - Access top element (O(1))
- `emplace(args...)` - Construct element in-place (O(1))
- `size()`, `empty()` - Capacity queries (O(1))
- `swap(other)` - Swap contents (O(1))

## Use Cases

### 1. Balanced Parentheses
```cpp
bool is_balanced(const std::string& expr) {
    Stack<char> s;
    for (char c : expr) {
        if (c == '(') s.push(c);
        else if (c == ')') {
            if (s.empty()) return false;
            s.pop();
        }
    }
    return s.empty();
}
```

### 2. Reverse String
```cpp
std::string reverse(const std::string& str) {
    Stack<char> s;
    for (char c : str) s.push(c);
    
    std::string reversed;
    while (!s.empty()) {
        reversed += s.top();
        s.pop();
    }
    return reversed;
}
```

### 3. Undo/Redo System
```cpp
Stack<State> undo_stack;
Stack<State> redo_stack;

void execute_action(State new_state) {
    undo_stack.push(current_state);
    current_state = new_state;
    // Clear redo on new action
    redo_stack = Stack<State>();
}

void undo() {
    if (!undo_stack.empty()) {
        redo_stack.push(current_state);
        current_state = undo_stack.top();
        undo_stack.pop();
    }
}
```

### 4. Postfix Expression Evaluation
```cpp
int evaluate_postfix(const std::vector<std::string>& tokens) {
    Stack<int> s;
    for (const auto& token : tokens) {
        if (is_operator(token)) {
            int b = s.top(); s.pop();
            int a = s.top(); s.pop();
            s.push(apply(a, b, token));
        } else {
            s.push(std::stoi(token));
        }
    }
    return s.top();
}
```

### 5. Depth-First Search (DFS)
```cpp
void dfs(int start, const Graph& graph) {
    Stack<int> s;
    std::vector<bool> visited(graph.size(), false);
    
    s.push(start);
    while (!s.empty()) {
        int node = s.top();
        s.pop();
        
        if (!visited[node]) {
            visited[node] = true;
            process(node);
            
            for (int neighbor : graph[node]) {
                if (!visited[neighbor]) {
                    s.push(neighbor);
                }
            }
        }
    }
}
```

### 6. Min Stack (Track Minimum)
```cpp
struct MinStack {
    Stack<int> data;
    Stack<int> mins;
    
    void push(int x) {
        data.push(x);
        if (mins.empty() || x <= mins.top()) {
            mins.push(x);
        }
    }
    
    void pop() {
        if (data.top() == mins.top()) {
            mins.pop();
        }
        data.pop();
    }
    
    int top() { return data.top(); }
    int get_min() { return mins.top(); }
};
```

## Underlying Container

Stack can be built on different containers:

```cpp
// Default: deque (good balance)
Stack<int> s1;

// With vector (cache-friendly)
Stack<int, std::vector<int>> s2;

// With list (stable pointers)
Stack<int, std::list<int>> s3;
```

## Performance

| Operation | Time | Space |
|-----------|------|-------|
| push | O(1) | O(1) |
| pop | O(1) | O(1) |
| top | O(1) | O(1) |
| empty/size | O(1) | O(1) |

## When to Use

**Use Stack when:**
- Need LIFO (Last In, First Out) access
- Implementing DFS, backtracking
- Expression evaluation (postfix/prefix)
- Undo/redo functionality
- Function call simulation
- Parentheses matching

**Don't use when:**
- Need FIFO (use Queue)
- Need random access (use Vector)
- Need both ends (use Deque)

## Algorithm Applications

1. **Backtracking**: Maze solving, N-Queens
2. **Parsing**: Compiler design, expression evaluation
3. **DFS**: Graph traversal, topological sort
4. **Undo Systems**: Text editors, games
5. **Function Calls**: Call stack simulation
6. **Browser History**: Back button

## Examples

See `stack_example.cpp` for 10 comprehensive examples:
1. Basic operations
2. String stack (undo system)
3. Balanced parentheses checker
4. Reverse string
5. Postfix expression evaluation
6. Undo/redo system
7. DFS traversal
8. Min stack tracking
9. Copy and move semantics
10. Custom container

Run:
```bash
./build/stack_example
```

## Testing

Run test suite:
```bash
./build/stack_test
```

All 15 tests cover:
- Basic push/pop operations
- LIFO ordering
- Copy/move semantics
- Comparison operators
- Edge cases

---

**Lines of Code**: 200+ (implementation) + 300+ (examples) + 180+ (tests)  
**Test Coverage**: 15/15 tests passing ✅  
**Adapter Pattern**: Built on underlying container  
**Complexity**: O(1) for all operations
