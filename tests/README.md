# Test Suite

Comprehensive test suite for the unique_ptr implementation.

## Files

- **`test_suite.cpp`** - Unit tests for unique_ptr

## Running Tests

```bash
# From project root
make test

# Or directly
cd .. && make test
```

## Test Coverage

### Construction Tests (6 tests)
- ✅ Default construction
- ✅ Nullptr construction
- ✅ Pointer construction
- ✅ make_unique
- ✅ Move construction
- ✅ Copy operations (verify deleted)

### Assignment Tests (4 tests)
- ✅ Move assignment
- ✅ Nullptr assignment
- ✅ Self-move assignment
- ✅ Copy assignment (verify deleted)

### Modifier Tests (5 tests)
- ✅ reset() - empty
- ✅ reset() - with pointer
- ✅ release()
- ✅ swap()
- ✅ Multiple resets

### Observer Tests (5 tests)
- ✅ get()
- ✅ operator*
- ✅ operator->
- ✅ operator bool()
- ✅ Null pointer checks

### Array Tests (3 tests)
- ✅ Array construction
- ✅ make_unique_array()
- ✅ operator[]

### Custom Deleter Tests (1 test)
- ✅ Custom deleter execution

### Polymorphism Tests (2 tests)
- ✅ Polymorphic deletion
- ✅ Derived to base conversion

### Comparison Tests (2 tests)
- ✅ Nullptr comparison
- ✅ Pointer comparison

### Function Parameter Tests (3 tests)
- ✅ Pass by value (take ownership)
- ✅ Pass by const reference (borrow)
- ✅ Pass by reference (modify)

### Return Value Tests (1 test)
- ✅ Return by value

### Edge Case Tests (3 tests)
- ✅ Multiple reset calls
- ✅ Move from empty
- ✅ Self-assignment handling

## Test Statistics

- **Total Tests**: 30+
- **Lines of Code**: ~500
- **Coverage**: Core functionality + edge cases
- **Memory Leaks**: 0 (verified with valgrind)

## Running with Memory Checker

```bash
# Valgrind (Linux/Mac)
make valgrind-test

# Address Sanitizer
cd .. && g++ -fsanitize=address -g -I. tests/test_suite.cpp -o build/test_suite
./build/test_suite
```

## Expected Output

```
========================================
  unique_ptr Test Suite                
========================================

Running test: default_construction... PASSED
Running test: nullptr_construction... PASSED
Running test: pointer_construction... PASSED
...
[30+ tests]
...

========================================
  All tests passed!                    
========================================

Final object count: 0 (should be 0)
```

## Adding New Tests

1. Add test function:
```cpp
TEST(my_new_test) {
    // Test code
    ASSERT_TRUE(condition);
}
```

2. Rebuild:
```bash
make clean && make test
```

## Test Macros

- `TEST(name)` - Define a test
- `ASSERT_TRUE(cond)` - Assert condition is true
- `ASSERT_FALSE(cond)` - Assert condition is false
- `ASSERT_EQ(a, b)` - Assert equality
- `ASSERT_NE(a, b)` - Assert inequality

## Debugging Failed Tests

If a test fails:
1. Note the line number in error message
2. Build with debug: `make debug`
3. Run with debugger: `lldb build/test_suite`
4. Set breakpoint: `b test_suite.cpp:LINE`
5. Run: `run`

## Future Test Ideas

- [ ] Thread-safety tests (if applicable)
- [ ] Performance benchmarks
- [ ] Comparison with std::unique_ptr
- [ ] More edge cases
- [ ] Fuzzing tests

## See Also

- `../unique_ptr/unique_ptr.hpp` - Implementation being tested
- `../unique_ptr/example.cpp` - Usage examples
- `../docs/IMPLEMENTATION_NOTES.md` - Implementation details

