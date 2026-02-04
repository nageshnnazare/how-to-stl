# Documentation

General documentation for the smart pointer implementations.

## Files

### Core Documentation

- **`ARCHITECTURE.md`** (16 KB) - Visual diagrams and architecture
  - Class hierarchy diagrams
  - Memory layout diagrams
  - Lifetime flow charts
  - Move semantics flow
  - Operation state transitions
  - Performance characteristics

- **`IMPLEMENTATION_NOTES.md`** (11 KB) - Deep technical dive
  - Core concepts (RAII, ownership)
  - Template design decisions
  - Key operations explained
  - Deleter design
  - Type conversions
  - Exception safety
  - Common pitfalls and solutions

- **`QUICK_REFERENCE.md`** (8.8 KB) - Cheat sheet
  - Basic operations
  - Code snippets
  - Function parameters
  - Return values
  - Common patterns
  - Anti-patterns
  - Debugging tips

## Reading Guide

### For Beginners
Start with:
1. `../unique_ptr/README.md` - Basic concepts
2. `QUICK_REFERENCE.md` - Practical usage
3. `../unique_ptr/example.cpp` - Working examples

### For Intermediate Developers
Continue with:
4. `IMPLEMENTATION_NOTES.md` - Design details
5. `../unique_ptr/unique_ptr.hpp` - Source code
6. `../shared_ptr/README.md` - Shared ownership

### For Advanced Developers
Deep dive:
7. `ARCHITECTURE.md` - Visual understanding
8. `../shared_ptr/shared_ptr.hpp` - Complex implementation
9. Memory management analysis
10. Performance optimization

## Topics Covered

### ARCHITECTURE.md

**Visual Diagrams:**
- Class hierarchy
- Memory layout (stack vs heap)
- Lifetime flow (construction → usage → destruction)
- Move semantics flow
- Operation state transitions
- Deleter dispatch mechanism
- Type conversion diagrams
- Function parameter patterns

**Performance Analysis:**
- Memory overhead comparison
- Runtime overhead benchmarks
- Zero-cost abstractions
- Optimization opportunities

**Integration Patterns:**
- STL containers
- Pimpl idiom
- Factory patterns

### IMPLEMENTATION_NOTES.md

**Core Concepts:**
- RAII principles
- Exclusive ownership model
- Move-only semantics
- Exception safety guarantees

**Design Decisions:**
- Template parameter order
- Why deleted copy operations
- Array specialization rationale
- Deleter design patterns

**Technical Details:**
- Constructor implementation
- Move constructor mechanics
- Destructor behavior
- Reset operation ordering
- Release semantics

**Advanced Topics:**
- Type conversions (Derived → Base)
- Empty base optimization
- Perfect forwarding
- SFINAE and type traits

**Common Pitfalls:**
- Double ownership
- Dangling references
- Incorrect array deletion
- Forgetting to move

### QUICK_REFERENCE.md

**Basic Operations:**
- Create unique_ptr
- Access objects
- Check validity
- Transfer ownership
- Reset/release

**Arrays:**
- Create arrays
- Access elements
- Proper deletion

**Custom Deleters:**
- Define deleters
- Use with unique_ptr
- Lambda deleters

**Function Parameters:**
- Take ownership (by value)
- Borrow (by const ref)
- Modify pointer (by ref)

**Common Patterns:**
- Factory functions
- Pimpl idiom
- RAII wrappers
- Optional ownership

**Cheat Sheet:**
- Quick reference table
- Common operations
- Compilation commands
- Debugging tools

## Usage

### Quick Lookup
```bash
# Search for a specific topic
grep -r "move semantics" docs/

# Find all examples
grep -r "Example:" docs/
```

### Generate PDF (optional)
```bash
# Using pandoc
pandoc ARCHITECTURE.md -o architecture.pdf
pandoc IMPLEMENTATION_NOTES.md -o implementation.pdf
pandoc QUICK_REFERENCE.md -o reference.pdf
```

### Browse Online
These markdown files are best viewed in:
- GitHub/GitLab (with rendering)
- VS Code (with markdown preview)
- Any markdown viewer

## Quick Access

### Common Questions

**Q: How do I create a unique_ptr?**  
→ See `QUICK_REFERENCE.md` - "Basic Operations"

**Q: Why use unique_ptr instead of raw pointers?**  
→ See `IMPLEMENTATION_NOTES.md` - "Core Concepts"

**Q: How does move semantics work?**  
→ See `ARCHITECTURE.md` - "Move Semantics Flow"

**Q: What's the memory layout?**  
→ See `ARCHITECTURE.md` - "Memory Layout"

**Q: How do I pass unique_ptr to functions?**  
→ See `QUICK_REFERENCE.md` - "Function Parameters"

**Q: What are common mistakes?**  
→ See `IMPLEMENTATION_NOTES.md` - "Common Pitfalls"

**Q: How does shared_ptr differ?**  
→ See `../shared_ptr/README.md`

## Key Takeaways

### unique_ptr
- **Zero overhead**: Same size and speed as raw pointers
- **Move-only**: Cannot be copied (exclusive ownership)
- **RAII**: Automatic cleanup, exception-safe
- **Type-safe**: Compile-time error prevention

### When to Use Each Document

| Goal | Document |
|------|----------|
| Quick syntax lookup | QUICK_REFERENCE.md |
| Understand design | IMPLEMENTATION_NOTES.md |
| Visual understanding | ARCHITECTURE.md |
| Usage examples | ../unique_ptr/example.cpp |
| Implementation details | ../unique_ptr/unique_ptr.hpp |

## Contributing

To improve documentation:
1. Keep diagrams simple and clear
2. Add concrete code examples
3. Explain the "why" not just "what"
4. Include anti-patterns
5. Update cross-references

## See Also

- `../README.md` - Project overview
- `../INDEX.md` - Complete file listing
- `../unique_ptr/` - unique_ptr implementation
- `../shared_ptr/` - shared_ptr implementation
- `../tests/` - Test suites

## Feedback

Documentation improvements welcome! Focus areas:
- More visual diagrams
- Additional examples
- Performance benchmarks
- Comparison with std::unique_ptr
- Advanced use cases

---

**Total Documentation**: ~35 KB (11K + 16K + 8.8K)  
**Diagrams**: 15+ visual representations  
**Examples**: 50+ code snippets  
**Topics**: 30+ covered in depth

