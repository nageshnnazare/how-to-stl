# List Implementation

Doubly linked list providing O(1) insertion/deletion at any position with iterator.

## Features
- **Doubly Linked**: Each node has prev and next pointers
- **O(1) Insert/Erase**: With iterator (no reallocation)
- **Bidirectional Iteration**: Forward and backward
- **Stable Pointers**: Elements don't move in memory
- **No Random Access**: Must traverse from beginning

## Quick Example

```cpp
List<int> l;
l.push_back(1);
l.push_back(2);
l.push_front(0);  // O(1) insert at front

for (const auto& val : l) {
    std::cout << val << " ";  // 0 1 2
}
```

## Operations

- `push_front(value)` - Add to front (O(1))
- `push_back(value)` - Add to back (O(1))
- `pop_front()` - Remove from front (O(1))
- `pop_back()` - Remove from back (O(1))
- `front()`, `back()` - Access ends (O(1))
- `size()`, `empty()` - Capacity queries (O(1))
- `clear()` - Remove all elements (O(n))

## When to Use

**Use List when:**
- Frequent insertions/deletions in middle
- Need stable iterators/pointers
- Don't need random access
- Implementing LRU cache, playlists

**Use Vector when:**
- Need random access
- Mostly append operations
- Want cache-friendly memory
- Smaller memory overhead

## Performance

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| push_front | O(1) | No reallocation |
| push_back | O(1) | No reallocation |
| pop_front | O(1) | - |
| pop_back | O(1) | - |
| insert (with iter) | O(1) | Constant time |
| erase (with iter) | O(1) | Constant time |
| access | O(n) | Must traverse |
| search | O(n) | Linear scan |

## Use Cases

### 1. LRU Cache
```cpp
List<int> cache;
// Move accessed item to front
// Evict from back when full
```

### 2. Music Playlist
```cpp
List<std::string> playlist;
playlist.push_back("Song A");
playlist.push_back("Song B");
// Can insert anywhere in O(1) with iterator
```

### 3. Undo/Redo with Deletion
```cpp
List<Action> history;
// Can remove from middle efficiently
```

## Implementation Details

**Node Structure:**
```cpp
struct Node {
    T data;
    Node* prev;
    Node* next;
};
```

**Advantages:**
- No reallocation on growth
- O(1) insert/erase anywhere (with iterator)
- Stable element addresses

**Disadvantages:**
- No random access (no operator[])
- More memory per element (2 pointers)
- Poor cache locality
- Extra pointer chasing

## Examples

See `list_example.cpp` for 10 examples:
1. Basic operations
2. Initializer list
3. Bidirectional iteration
4. Copy and move
5. LRU cache simulation
6. Music playlist
7. List vs Vector comparison
8. Empty and clear
9. Double-ended operations
10. List of strings

Run:
```bash
./build/list_example
```

## Testing

Run test suite:
```bash
./build/list_test
```

All 15 tests cover:
- Push/pop operations
- Front/back access
- Iteration
- Copy/move semantics
- Edge cases

---

**Lines of Code**: 170+ (implementation) + 250+ (examples) + 180+ (tests)  
**Test Coverage**: 15/15 tests passing ✅  
**Complexity**: O(1) insert/erase with iterator, O(n) access  
**Memory**: 2 pointers + data per element

