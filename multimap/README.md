# Multimap Implementation

Ordered key-value container that **allows duplicate keys**.

## Features
- **Ordered**: Keys sorted
- **Allows Duplicate Keys**: Multiple values per key
- **Useful for**: One-to-many relationships

## Quick Example
```cpp
Multimap<std::string, int> scores;
scores.insert({"Alice", 90});
scores.insert({"Alice", 95});  // Same key, different value
scores.insert({"Alice", 88});

int count = scores.count("Alice");  // Returns 3
```

Perfect for storing multiple values associated with the same key.
