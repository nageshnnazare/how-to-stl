# Makefile for Smart Pointer Implementations
# Organized directory structure

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Wpedantic -O2 -I.

# Directories
UNIQUE_PTR_DIR = unique_ptr
SHARED_PTR_DIR = shared_ptr
STRING_DIR = string
VECTOR_DIR = vector
DEQUE_DIR = deque
TESTS_DIR = tests
BUILD_DIR = build

# Source files
UNIQUE_PTR_HEADER = $(UNIQUE_PTR_DIR)/unique_ptr.hpp
UNIQUE_PTR_EXAMPLE = $(UNIQUE_PTR_DIR)/unique_ptr_example.cpp
SHARED_PTR_HEADER = $(SHARED_PTR_DIR)/shared_ptr.hpp
SHARED_PTR_EXAMPLE = $(SHARED_PTR_DIR)/shared_ptr_example.cpp
STRING_HEADER = $(STRING_DIR)/string.hpp
STRING_EXAMPLE = $(STRING_DIR)/string_example.cpp
VECTOR_HEADER = $(VECTOR_DIR)/vector.hpp
VECTOR_EXAMPLE = $(VECTOR_DIR)/vector_example.cpp
DEQUE_HEADER = $(DEQUE_DIR)/deque.hpp
DEQUE_EXAMPLE = $(DEQUE_DIR)/deque_example.cpp
UNIQUE_PTR_TEST = $(TESTS_DIR)/unique_ptr_test.cpp
SHARED_PTR_TEST = $(TESTS_DIR)/shared_ptr_test.cpp
STRING_TEST = $(TESTS_DIR)/string_test.cpp
VECTOR_TEST = $(TESTS_DIR)/vector_test.cpp
DEQUE_TEST = $(TESTS_DIR)/deque_test.cpp

# Output binaries
UNIQUE_EXAMPLE_BIN = $(BUILD_DIR)/unique_ptr_example
SHARED_EXAMPLE_BIN = $(BUILD_DIR)/shared_ptr_example
STRING_EXAMPLE_BIN = $(BUILD_DIR)/string_example
VECTOR_EXAMPLE_BIN = $(BUILD_DIR)/vector_example
DEQUE_EXAMPLE_BIN = $(BUILD_DIR)/deque_example
UNIQUE_PTR_TEST_BIN = $(BUILD_DIR)/unique_ptr_test
SHARED_PTR_TEST_BIN = $(BUILD_DIR)/shared_ptr_test
STRING_TEST_BIN = $(BUILD_DIR)/string_test
VECTOR_TEST_BIN = $(BUILD_DIR)/vector_test
DEQUE_TEST_BIN = $(BUILD_DIR)/deque_test

# Default target - build everything
all: $(BUILD_DIR) $(UNIQUE_EXAMPLE_BIN) $(SHARED_EXAMPLE_BIN) $(STRING_EXAMPLE_BIN) $(VECTOR_EXAMPLE_BIN) $(DEQUE_EXAMPLE_BIN) \
     $(UNIQUE_PTR_TEST_BIN) $(SHARED_PTR_TEST_BIN) $(STRING_TEST_BIN) $(VECTOR_TEST_BIN) $(DEQUE_TEST_BIN)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build unique_ptr example
$(UNIQUE_EXAMPLE_BIN): $(UNIQUE_PTR_EXAMPLE) $(UNIQUE_PTR_HEADER)
	@echo "Building unique_ptr example..."
	$(CXX) $(CXXFLAGS) $(UNIQUE_PTR_EXAMPLE) -o $(UNIQUE_EXAMPLE_BIN)
	@echo "✓ Built: $(UNIQUE_EXAMPLE_BIN)"

# Build shared_ptr example
$(SHARED_EXAMPLE_BIN): $(SHARED_PTR_EXAMPLE) $(SHARED_PTR_HEADER)
	@echo "Building shared_ptr example..."
	$(CXX) $(CXXFLAGS) $(SHARED_PTR_EXAMPLE) -o $(SHARED_EXAMPLE_BIN)
	@echo "✓ Built: $(SHARED_EXAMPLE_BIN)"

# Build string example
$(STRING_EXAMPLE_BIN): $(STRING_EXAMPLE) $(STRING_HEADER)
	@echo "Building string example..."
	$(CXX) $(CXXFLAGS) $(STRING_EXAMPLE) -o $(STRING_EXAMPLE_BIN)
	@echo "✓ Built: $(STRING_EXAMPLE_BIN)"

# Build vector example
$(VECTOR_EXAMPLE_BIN): $(VECTOR_EXAMPLE) $(VECTOR_HEADER)
	@echo "Building vector example..."
	$(CXX) $(CXXFLAGS) $(VECTOR_EXAMPLE) -o $(VECTOR_EXAMPLE_BIN)
	@echo "✓ Built: $(VECTOR_EXAMPLE_BIN)"

# Build deque example
$(DEQUE_EXAMPLE_BIN): $(DEQUE_EXAMPLE) $(DEQUE_HEADER)
	@echo "Building deque example..."
	$(CXX) $(CXXFLAGS) $(DEQUE_EXAMPLE) -o $(DEQUE_EXAMPLE_BIN)
	@echo "✓ Built: $(DEQUE_EXAMPLE_BIN)"

# Build unique_ptr test suite
$(UNIQUE_PTR_TEST_BIN): $(UNIQUE_PTR_TEST) $(UNIQUE_PTR_HEADER)
	@echo "Building unique_ptr test suite..."
	$(CXX) $(CXXFLAGS) $(UNIQUE_PTR_TEST) -o $(UNIQUE_PTR_TEST_BIN)
	@echo "✓ Built: $(UNIQUE_PTR_TEST_BIN)"

# Build shared_ptr test suite
$(SHARED_PTR_TEST_BIN): $(SHARED_PTR_TEST) $(SHARED_PTR_HEADER)
	@echo "Building shared_ptr test suite..."
	$(CXX) $(CXXFLAGS) $(SHARED_PTR_TEST) -o $(SHARED_PTR_TEST_BIN)
	@echo "✓ Built: $(SHARED_PTR_TEST_BIN)"

# Build string test suite
$(STRING_TEST_BIN): $(STRING_TEST) $(STRING_HEADER)
	@echo "Building string test suite..."
	$(CXX) $(CXXFLAGS) $(STRING_TEST) -o $(STRING_TEST_BIN)
	@echo "✓ Built: $(STRING_TEST_BIN)"

# Build vector test suite
$(VECTOR_TEST_BIN): $(VECTOR_TEST) $(VECTOR_HEADER)
	@echo "Building vector test suite..."
	$(CXX) $(CXXFLAGS) $(VECTOR_TEST) -o $(VECTOR_TEST_BIN)
	@echo "✓ Built: $(VECTOR_TEST_BIN)"

# Build deque test suite
$(DEQUE_TEST_BIN): $(DEQUE_TEST) $(DEQUE_HEADER)
	@echo "Building deque test suite..."
	$(CXX) $(CXXFLAGS) $(DEQUE_TEST) -o $(DEQUE_TEST_BIN)
	@echo "✓ Built: $(DEQUE_TEST_BIN)"

# Run unique_ptr example
run-unique: $(UNIQUE_EXAMPLE_BIN)
	@echo "\n=== Running unique_ptr example ==="
	@$(UNIQUE_EXAMPLE_BIN)

# Run shared_ptr example
run-shared: $(SHARED_EXAMPLE_BIN)
	@echo "\n=== Running shared_ptr example ==="
	@$(SHARED_EXAMPLE_BIN)

# Run string example
run-string: $(STRING_EXAMPLE_BIN)
	@echo "\n=== Running string example ==="
	@$(STRING_EXAMPLE_BIN)

# Run vector example
run-vector: $(VECTOR_EXAMPLE_BIN)
	@echo "\n=== Running vector example ==="
	@$(VECTOR_EXAMPLE_BIN)

# Run deque example
run-deque: $(DEQUE_EXAMPLE_BIN)
	@echo "\n=== Running deque example ==="
	@$(DEQUE_EXAMPLE_BIN)

# Run unique_ptr test suite
test-unique: $(UNIQUE_PTR_TEST_BIN)
	@echo "\n=== Running unique_ptr test suite ==="
	@$(UNIQUE_PTR_TEST_BIN)

# Run shared_ptr test suite
test-shared: $(SHARED_PTR_TEST_BIN)
	@echo "\n=== Running shared_ptr test suite ==="
	@$(SHARED_PTR_TEST_BIN)

# Run string test suite
test-string: $(STRING_TEST_BIN)
	@echo "\n=== Running string test suite ==="
	@$(STRING_TEST_BIN)

# Run vector test suite
test-vector: $(VECTOR_TEST_BIN)
	@echo "\n=== Running vector test suite ==="
	@$(VECTOR_TEST_BIN)

# Run deque test suite
test-deque: $(DEQUE_TEST_BIN)
	@echo "\n=== Running deque test suite ==="
	@$(DEQUE_TEST_BIN)

# Run all test suites
test: test-unique test-shared test-string test-vector test-deque

# Run all examples and tests
run-all: run-unique run-shared run-string run-vector run-deque test

# Build with debug symbols
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: clean all

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(UNIQUE_PTR_DIR)/*.o $(SHARED_PTR_DIR)/*.o $(TESTS_DIR)/*.o
	@rm -rf *.dSYM $(UNIQUE_PTR_DIR)/*.dSYM $(SHARED_PTR_DIR)/*.dSYM $(TESTS_DIR)/*.dSYM
	@echo "✓ Clean complete"

# Run valgrind on unique_ptr example
valgrind-unique: $(UNIQUE_EXAMPLE_BIN)
	@echo "\n=== Valgrind: unique_ptr example ==="
	valgrind --leak-check=full --show-leak-kinds=all $(UNIQUE_EXAMPLE_BIN)

# Run valgrind on shared_ptr example
valgrind-shared: $(SHARED_EXAMPLE_BIN)
	@echo "\n=== Valgrind: shared_ptr example ==="
	valgrind --leak-check=full --show-leak-kinds=all $(SHARED_EXAMPLE_BIN)

# Run valgrind on unique_ptr tests
valgrind-test-unique: $(UNIQUE_PTR_TEST_BIN)
	@echo "\n=== Valgrind: unique_ptr test suite ==="
	valgrind --leak-check=full --show-leak-kinds=all $(UNIQUE_PTR_TEST_BIN)

# Run valgrind on shared_ptr tests
valgrind-test-shared: $(SHARED_PTR_TEST_BIN)
	@echo "\n=== Valgrind: shared_ptr test suite ==="
	valgrind --leak-check=full --show-leak-kinds=all $(SHARED_PTR_TEST_BIN)

# Run valgrind on all tests
valgrind-test: valgrind-test-unique valgrind-test-shared

# Format code (requires clang-format)
format:
	@echo "Formatting code..."
	@find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
	@echo "✓ Format complete"

# Show directory structure
tree:
	@echo "\n=== Project Structure ==="
	@tree -L 2 --dirsfirst -I 'build|*.dSYM' .

# Show file sizes
sizes:
	@echo "\n=== File Sizes ==="
	@find . -type f \( -name "*.hpp" -o -name "*.cpp" -o -name "*.md" \) -exec ls -lh {} \; | awk '{printf "%-50s %8s\n", $$9, $$5}' | sort

# Help target
help:
	@echo "Smart Pointer Implementation - Build System"
	@echo "==========================================="
	@echo ""
	@echo "Building:"
	@echo "  all              - Build all examples and tests (default)"
	@echo "  debug            - Build with debug symbols and -O0"
	@echo "  clean            - Remove all build artifacts"
	@echo ""
	@echo "Running:"
	@echo "  run-unique       - Run unique_ptr examples"
	@echo "  run-shared       - Run shared_ptr examples"
	@echo "  run-string       - Run string examples"
	@echo "  run-vector       - Run vector examples"
	@echo "  run-deque        - Run deque examples"
	@echo "  test-unique      - Run unique_ptr tests"
	@echo "  test-shared      - Run shared_ptr tests"
	@echo "  test-string      - Run string tests"
	@echo "  test-vector      - Run vector tests"
	@echo "  test-deque       - Run deque tests"
	@echo "  test             - Run all tests"
	@echo "  run-all          - Run everything"
	@echo ""
	@echo "Memory Checking:"
	@echo "  valgrind-unique      - Check unique_ptr for memory leaks"
	@echo "  valgrind-shared      - Check shared_ptr for memory leaks"
	@echo "  valgrind-test-unique - Check unique_ptr tests for memory leaks"
	@echo "  valgrind-test-shared - Check shared_ptr tests for memory leaks"
	@echo "  valgrind-test        - Check all tests for memory leaks"
	@echo ""
	@echo "Utilities:"
	@echo "  format           - Format code with clang-format"
	@echo "  tree             - Show directory structure"
	@echo "  sizes            - Show file sizes"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Directory Structure:"
	@echo "  unique_ptr/      - unique_ptr implementation and examples"
	@echo "  shared_ptr/      - shared_ptr implementation and examples"
	@echo "  string/          - String class implementation and examples"
	@echo "  vector/          - Vector class implementation and examples"
	@echo "  deque/           - Deque class implementation and examples"
	@echo "  tests/           - Test suites"
	@echo "  docs/            - General documentation"
	@echo "  build/           - Build output (generated)"

.PHONY: all run-unique run-shared run-string run-vector run-deque test-unique test-shared test-string test-vector test-deque test run-all debug clean \
        valgrind-unique valgrind-shared valgrind-test-unique valgrind-test-shared valgrind-test \
        format tree sizes help

# Associative containers
SET_DIR = set
MAP_DIR = map
MULTISET_DIR = multiset
MULTIMAP_DIR = multimap
UNORDERED_SET_DIR = unordered_set
UNORDERED_MAP_DIR = unordered_map

SET_EXAMPLE = $(SET_DIR)/set_example.cpp
MAP_EXAMPLE = $(MAP_DIR)/map_example.cpp
MULTISET_EXAMPLE = $(MULTISET_DIR)/multiset_example.cpp
MULTIMAP_EXAMPLE = $(MULTIMAP_DIR)/multimap_example.cpp
UNORDERED_SET_EXAMPLE = $(UNORDERED_SET_DIR)/unordered_set_example.cpp
UNORDERED_MAP_EXAMPLE = $(UNORDERED_MAP_DIR)/unordered_map_example.cpp

SET_TEST = $(TESTS_DIR)/set_test.cpp
MAP_TEST = $(TESTS_DIR)/map_test.cpp
MULTISET_TEST = $(TESTS_DIR)/multiset_test.cpp
MULTIMAP_TEST = $(TESTS_DIR)/multimap_test.cpp
UNORDERED_SET_TEST = $(TESTS_DIR)/unordered_set_test.cpp
UNORDERED_MAP_TEST = $(TESTS_DIR)/unordered_map_test.cpp

SET_EXAMPLE_BIN = $(BUILD_DIR)/set_example
MAP_EXAMPLE_BIN = $(BUILD_DIR)/map_example
MULTISET_EXAMPLE_BIN = $(BUILD_DIR)/multiset_example
MULTIMAP_EXAMPLE_BIN = $(BUILD_DIR)/multimap_example
UNORDERED_SET_EXAMPLE_BIN = $(BUILD_DIR)/unordered_set_example
UNORDERED_MAP_EXAMPLE_BIN = $(BUILD_DIR)/unordered_map_example

SET_TEST_BIN = $(BUILD_DIR)/set_test
MAP_TEST_BIN = $(BUILD_DIR)/map_test
MULTISET_TEST_BIN = $(BUILD_DIR)/multiset_test
MULTIMAP_TEST_BIN = $(BUILD_DIR)/multimap_test
UNORDERED_SET_TEST_BIN = $(BUILD_DIR)/unordered_set_test
UNORDERED_MAP_TEST_BIN = $(BUILD_DIR)/unordered_map_test

# Build associative container examples
$(SET_EXAMPLE_BIN): $(SET_EXAMPLE)
	@$(CXX) $(CXXFLAGS) $(SET_EXAMPLE) -o $(SET_EXAMPLE_BIN)

$(MAP_EXAMPLE_BIN): $(MAP_EXAMPLE)
	@$(CXX) $(CXXFLAGS) $(MAP_EXAMPLE) -o $(MAP_EXAMPLE_BIN)

$(MULTISET_EXAMPLE_BIN): $(MULTISET_EXAMPLE)
	@$(CXX) $(CXXFLAGS) $(MULTISET_EXAMPLE) -o $(MULTISET_EXAMPLE_BIN)

$(MULTIMAP_EXAMPLE_BIN): $(MULTIMAP_EXAMPLE)
	@$(CXX) $(CXXFLAGS) $(MULTIMAP_EXAMPLE) -o $(MULTIMAP_EXAMPLE_BIN)

$(UNORDERED_SET_EXAMPLE_BIN): $(UNORDERED_SET_EXAMPLE)
	@$(CXX) $(CXXFLAGS) $(UNORDERED_SET_EXAMPLE) -o $(UNORDERED_SET_EXAMPLE_BIN)

$(UNORDERED_MAP_EXAMPLE_BIN): $(UNORDERED_MAP_EXAMPLE)
	@$(CXX) $(CXXFLAGS) $(UNORDERED_MAP_EXAMPLE) -o $(UNORDERED_MAP_EXAMPLE_BIN)

# Build associative container tests
$(SET_TEST_BIN): $(SET_TEST)
	@$(CXX) $(CXXFLAGS) $(SET_TEST) -o $(SET_TEST_BIN)

$(MAP_TEST_BIN): $(MAP_TEST)
	@$(CXX) $(CXXFLAGS) $(MAP_TEST) -o $(MAP_TEST_BIN)

$(MULTISET_TEST_BIN): $(MULTISET_TEST)
	@$(CXX) $(CXXFLAGS) $(MULTISET_TEST) -o $(MULTISET_TEST_BIN)

$(MULTIMAP_TEST_BIN): $(MULTIMAP_TEST)
	@$(CXX) $(CXXFLAGS) $(MULTIMAP_TEST) -o $(MULTIMAP_TEST_BIN)

$(UNORDERED_SET_TEST_BIN): $(UNORDERED_SET_TEST)
	@$(CXX) $(CXXFLAGS) $(UNORDERED_SET_TEST) -o $(UNORDERED_SET_TEST_BIN)

$(UNORDERED_MAP_TEST_BIN): $(UNORDERED_MAP_TEST)
	@$(CXX) $(CXXFLAGS) $(UNORDERED_MAP_TEST) -o $(UNORDERED_MAP_TEST_BIN)

# Run associative examples
run-set: $(SET_EXAMPLE_BIN)
	@echo "\n=== Set Example ===" && @$(SET_EXAMPLE_BIN)

run-map: $(MAP_EXAMPLE_BIN)
	@echo "\n=== Map Example ===" && @$(MAP_EXAMPLE_BIN)

run-associative: run-set run-map $(MULTISET_EXAMPLE_BIN) $(MULTIMAP_EXAMPLE_BIN) $(UNORDERED_SET_EXAMPLE_BIN) $(UNORDERED_MAP_EXAMPLE_BIN)
	@echo "\n=== Multiset Example ===" && @$(MULTISET_EXAMPLE_BIN)
	@echo "\n=== Multimap Example ===" && @$(MULTIMAP_EXAMPLE_BIN)
	@echo "\n=== UnorderedSet Example ===" && @$(UNORDERED_SET_EXAMPLE_BIN)
	@echo "\n=== UnorderedMap Example ===" && @$(UNORDERED_MAP_EXAMPLE_BIN)

# Run associative tests
test-associative: $(SET_TEST_BIN) $(MAP_TEST_BIN) $(MULTISET_TEST_BIN) $(MULTIMAP_TEST_BIN) $(UNORDERED_SET_TEST_BIN) $(UNORDERED_MAP_TEST_BIN)
	@echo "\n=== Set Test ===" && @$(SET_TEST_BIN)
	@echo "\n=== Map Test ===" && @$(MAP_TEST_BIN)
	@echo "\n=== Multiset Test ===" && @$(MULTISET_TEST_BIN)
	@echo "\n=== Multimap Test ===" && @$(MULTIMAP_TEST_BIN)
	@echo "\n=== UnorderedSet Test ===" && @$(UNORDERED_SET_TEST_BIN)
	@echo "\n=== UnorderedMap Test ===" && @$(UNORDERED_MAP_TEST_BIN)

.PHONY: run-set run-map run-associative test-associative

# Advanced Systems Programming
ALLOCATOR_DIR = allocator
LOCKS_DIR = locks
ARENA_ALLOCATOR_DIR = arena_allocator
THREAD_POOL_DIR = thread_pool

ALLOCATOR_EXAMPLE = $(ALLOCATOR_DIR)/allocator_example.cpp
LOCKS_EXAMPLE = $(LOCKS_DIR)/locks_example.cpp
ARENA_ALLOCATOR_EXAMPLE = $(ARENA_ALLOCATOR_DIR)/arena_allocator_example.cpp
THREAD_POOL_EXAMPLE = $(THREAD_POOL_DIR)/thread_pool_example.cpp

ALLOCATOR_TEST = $(TESTS_DIR)/allocator_test.cpp
LOCKS_TEST = $(TESTS_DIR)/locks_test.cpp
ARENA_ALLOCATOR_TEST = $(TESTS_DIR)/arena_allocator_test.cpp
THREAD_POOL_TEST = $(TESTS_DIR)/thread_pool_test.cpp

ALLOCATOR_EXAMPLE_BIN = $(BUILD_DIR)/allocator_example
LOCKS_EXAMPLE_BIN = $(BUILD_DIR)/locks_example
ARENA_ALLOCATOR_EXAMPLE_BIN = $(BUILD_DIR)/arena_allocator_example
THREAD_POOL_EXAMPLE_BIN = $(BUILD_DIR)/thread_pool_example

ALLOCATOR_TEST_BIN = $(BUILD_DIR)/allocator_test
LOCKS_TEST_BIN = $(BUILD_DIR)/locks_test
ARENA_ALLOCATOR_TEST_BIN = $(BUILD_DIR)/arena_allocator_test
THREAD_POOL_TEST_BIN = $(BUILD_DIR)/thread_pool_test

# Build advanced systems examples
$(ALLOCATOR_EXAMPLE_BIN): $(ALLOCATOR_EXAMPLE)
	@echo "Building allocator example..."
	@$(CXX) $(CXXFLAGS) -pthread $(ALLOCATOR_EXAMPLE) -o $(ALLOCATOR_EXAMPLE_BIN)
	@echo "✓ Built: $(ALLOCATOR_EXAMPLE_BIN)"

$(LOCKS_EXAMPLE_BIN): $(LOCKS_EXAMPLE)
	@echo "Building locks example..."
	@$(CXX) $(CXXFLAGS) -pthread $(LOCKS_EXAMPLE) -o $(LOCKS_EXAMPLE_BIN)
	@echo "✓ Built: $(LOCKS_EXAMPLE_BIN)"

$(ARENA_ALLOCATOR_EXAMPLE_BIN): $(ARENA_ALLOCATOR_EXAMPLE)
	@echo "Building arena allocator example..."
	@$(CXX) $(CXXFLAGS) -pthread $(ARENA_ALLOCATOR_EXAMPLE) -o $(ARENA_ALLOCATOR_EXAMPLE_BIN)
	@echo "✓ Built: $(ARENA_ALLOCATOR_EXAMPLE_BIN)"

$(THREAD_POOL_EXAMPLE_BIN): $(THREAD_POOL_EXAMPLE)
	@echo "Building thread pool example..."
	@$(CXX) $(CXXFLAGS) -pthread $(THREAD_POOL_EXAMPLE) -o $(THREAD_POOL_EXAMPLE_BIN)
	@echo "✓ Built: $(THREAD_POOL_EXAMPLE_BIN)"

# Build advanced systems tests
$(ALLOCATOR_TEST_BIN): $(ALLOCATOR_TEST)
	@echo "Building allocator test suite..."
	@$(CXX) $(CXXFLAGS) -pthread $(ALLOCATOR_TEST) -o $(ALLOCATOR_TEST_BIN)
	@echo "✓ Built: $(ALLOCATOR_TEST_BIN)"

$(LOCKS_TEST_BIN): $(LOCKS_TEST)
	@echo "Building locks test suite..."
	@$(CXX) $(CXXFLAGS) -pthread $(LOCKS_TEST) -o $(LOCKS_TEST_BIN)
	@echo "✓ Built: $(LOCKS_TEST_BIN)"

$(ARENA_ALLOCATOR_TEST_BIN): $(ARENA_ALLOCATOR_TEST)
	@echo "Building arena allocator test suite..."
	@$(CXX) $(CXXFLAGS) -pthread $(ARENA_ALLOCATOR_TEST) -o $(ARENA_ALLOCATOR_TEST_BIN)
	@echo "✓ Built: $(ARENA_ALLOCATOR_TEST_BIN)"

$(THREAD_POOL_TEST_BIN): $(THREAD_POOL_TEST)
	@echo "Building thread pool test suite..."
	@$(CXX) $(CXXFLAGS) -pthread $(THREAD_POOL_TEST) -o $(THREAD_POOL_TEST_BIN)
	@echo "✓ Built: $(THREAD_POOL_TEST_BIN)"

# Run advanced systems examples
run-allocator: $(ALLOCATOR_EXAMPLE_BIN)
	@echo "\n=== Allocator Example ==="
	@$(ALLOCATOR_EXAMPLE_BIN)

run-locks: $(LOCKS_EXAMPLE_BIN)
	@echo "\n=== Locks Example ==="
	@$(LOCKS_EXAMPLE_BIN)

run-arena-allocator: $(ARENA_ALLOCATOR_EXAMPLE_BIN)
	@echo "\n=== Arena Allocator Example ==="
	@$(ARENA_ALLOCATOR_EXAMPLE_BIN)

run-thread-pool: $(THREAD_POOL_EXAMPLE_BIN)
	@echo "\n=== Thread Pool Example ==="
	@$(THREAD_POOL_EXAMPLE_BIN)

# Run advanced systems tests
test-allocator: $(ALLOCATOR_TEST_BIN)
	@echo "\n=== Allocator Test Suite ==="
	@$(ALLOCATOR_TEST_BIN)

test-locks: $(LOCKS_TEST_BIN)
	@echo "\n=== Locks Test Suite ==="
	@$(LOCKS_TEST_BIN)

test-arena-allocator: $(ARENA_ALLOCATOR_TEST_BIN)
	@echo "\n=== Arena Allocator Test Suite ==="
	@$(ARENA_ALLOCATOR_TEST_BIN)

test-thread-pool: $(THREAD_POOL_TEST_BIN)
	@echo "\n=== Thread Pool Test Suite ==="
	@$(THREAD_POOL_TEST_BIN)

# Build all advanced systems
advanced: $(ALLOCATOR_EXAMPLE_BIN) $(LOCKS_EXAMPLE_BIN) $(ARENA_ALLOCATOR_EXAMPLE_BIN) $(THREAD_POOL_EXAMPLE_BIN) \
          $(ALLOCATOR_TEST_BIN) $(LOCKS_TEST_BIN) $(ARENA_ALLOCATOR_TEST_BIN) $(THREAD_POOL_TEST_BIN)

# Run all advanced systems
run-advanced: run-allocator run-locks run-arena-allocator run-thread-pool

# Test all advanced systems
test-advanced: test-allocator test-locks test-arena-allocator test-thread-pool

.PHONY: run-allocator run-locks run-arena-allocator run-thread-pool \
        test-allocator test-locks test-arena-allocator test-thread-pool \
        advanced run-advanced test-advanced

