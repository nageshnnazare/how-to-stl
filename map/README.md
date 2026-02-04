# Map Implementation

Red-Black Tree based ordered key-value container.

## Features
- **Ordered**: Keys sorted automatically
- **Unique Keys**: No duplicate keys
- **O(log n)**: Insert, find, erase operations
- **Key-Value**: Associate values with keys
- **operator[]**: Direct element access

## Quick Example
```cpp
Map<std::string, int> ages;
ages["Alice"] = 30;
ages["Bob"] = 25;

int age = ages["Alice"];  // Access value
ages.erase("Bob");        // Remove key
bool has = ages.contains("Alice");

for (const auto& p : ages) {  // Iterate in key order
    std::cout << p.first << ": " << p.second << "\n";
}
```

## Operations
- `operator[key]` - Access/insert element (O(log n))
- `at(key)` - Access with bounds check (O(log n))
- `insert({key, value})` - Add pair (O(log n))
- `erase(key)` - Remove by key (O(log n))
- `find(key)`, `contains(key)` - Lookup (O(log n))

See `map_example.cpp` for comprehensive examples.
