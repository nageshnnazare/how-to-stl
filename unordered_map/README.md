# UnorderedMap Implementation

Hash table based key-value container with **O(1) average** operations.

## Features
- **Unordered**: No key sorting
- **Unique Keys**: No duplicate keys
- **O(1) average**: Insert, find, erase
- **Hash Table**: Separate chaining
- **Fast**: Better than Map for pure lookups

## Quick Example
```cpp
UnorderedMap<std::string, int> ages;
ages["Alice"] = 30;     // O(1) average
ages["Bob"] = 25;

int age = ages["Alice"]; // O(1) average
bool has = ages.contains("Bob"); // O(1) average
```

Use when you don't need key ordering and want maximum speed.
