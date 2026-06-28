# ============================================================================
#  how-to-stl -- build system
# ----------------------------------------------------------------------------
#  Every component lives in <name>/ with:
#     <name>/<name>.hpp           header-only implementation
#     <name>/<name>_example.cpp   runnable example tour
#     tests/<name>_test.cpp       unit tests
#
#  This Makefile is fully generic: add a folder name to MODULES below and it
#  automatically gains  make run-<name>  and  make test-<name>  targets.
#
#  Common targets:
#     make all            build every example + test into build/
#     make examples       build every example
#     make test           build AND run every test suite
#     make run-vector     build + run one example   (any module name)
#     make test-set       build + run one test suite (any module name)
#     make clean          remove build artifacts
#     make help           list everything
# ============================================================================

CXX      := g++
CXXFLAGS := -std=c++14 -Wall -Wextra -Wpedantic -O2 -I.
LDFLAGS  := -pthread            # harmless for non-threaded modules
BUILD    := build

# The single source of truth -- one line per component.
MODULES := \
	unique_ptr shared_ptr \
	array vector string deque list \
	set map multiset multimap \
	unordered_set unordered_map \
	stack queue priority_queue \
	pair tuple optional bitset \
	allocator arena_allocator locks thread_pool

EXAMPLE_BINS := $(addprefix $(BUILD)/,$(addsuffix _example,$(MODULES)))
TEST_BINS    := $(addprefix $(BUILD)/,$(addsuffix _test,$(MODULES)))

# --------------------------------------------------------------------------
# Aggregate targets
# --------------------------------------------------------------------------
.PHONY: all examples build-tests test run clean format tree sizes help
.DEFAULT_GOAL := all

all: examples build-tests

examples: $(EXAMPLE_BINS)

build-tests: $(TEST_BINS)

# Build, then run, every test suite. Stops at the first failure.
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		echo "\n=== $$t ==="; \
		$$t || exit 1; \
	done
	@echo "\n✓ All test suites passed"

# Build, then run, every example.
run: $(EXAMPLE_BINS)
	@for e in $(EXAMPLE_BINS); do \
		echo "\n=== $$e ==="; \
		$$e; \
	done

# --------------------------------------------------------------------------
# Pattern rules -- compile any module on demand
# --------------------------------------------------------------------------
$(BUILD):
	@mkdir -p $(BUILD)

# build/<name>_example  <-  <name>/<name>_example.cpp
# The source path repeats the module name in both the folder and the file, which
# a single pattern rule can't express, so we generate one explicit rule per
# module with $(eval).
define EXAMPLE_RULE
$(BUILD)/$(1)_example: $(1)/$(1)_example.cpp | $(BUILD)
	@echo "Building $(1) example..."
	@$(CXX) $(CXXFLAGS) $$< -o $$@ $(LDFLAGS)
endef
$(foreach m,$(MODULES),$(eval $(call EXAMPLE_RULE,$(m))))

# build/<name>_test  <-  tests/<name>_test.cpp  (single stem, plain pattern rule)
$(BUILD)/%_test: tests/%_test.cpp | $(BUILD)
	@echo "Building $* test..."
	@$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# make run-<name>  /  make test-<name>
run-%: $(BUILD)/%_example
	@echo "\n=== Running $* example ==="
	@$<

test-%: $(BUILD)/%_test
	@echo "\n=== Running $* tests ==="
	@$<

# make asan-<name>  -- compile + run one test under AddressSanitizer + UBSan.
# Built fresh each time (separate flags), not cached in build/.
asan-%: tests/%_test.cpp | $(BUILD)
	@echo "\n=== $* under ASan + UBSan ==="
	@$(CXX) -std=c++14 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
		-pthread -I. $< -o $(BUILD)/$*_asan
	@$(BUILD)/$*_asan

# --------------------------------------------------------------------------
# Utilities
# --------------------------------------------------------------------------
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD) *.dSYM */*.dSYM
	@echo "✓ Clean complete"

format:
	@echo "Formatting (requires clang-format)..."
	@find . -name '*.hpp' -o -name '*.cpp' | xargs clang-format -i
	@echo "✓ Format complete"

tree:
	@tree -L 2 --dirsfirst -I 'build|*.dSYM' . 2>/dev/null || \
		find . -maxdepth 2 -not -path '*/.*' -not -path '*/build/*' | sort

sizes:
	@find . -type f \( -name '*.hpp' -o -name '*.cpp' -o -name '*.md' \) \
		-exec ls -lh {} \; | awk '{printf "%-55s %8s\n", $$9, $$5}' | sort

help:
	@echo "how-to-stl build system"
	@echo "======================="
	@echo ""
	@echo "Aggregate:"
	@echo "  make all          build every example + test"
	@echo "  make examples     build every example"
	@echo "  make test         build AND run every test suite"
	@echo "  make run          build AND run every example"
	@echo "  make clean        remove build artifacts"
	@echo ""
	@echo "Per module (any of: $(MODULES)):"
	@echo "  make run-<name>   build + run that example   (e.g. make run-vector)"
	@echo "  make test-<name>  build + run that test      (e.g. make test-set)"
	@echo "  make asan-<name>  run that test under ASan+UBSan (e.g. make asan-deque)"
	@echo ""
	@echo "Utilities:"
	@echo "  make format       clang-format all sources"
	@echo "  make tree         show directory structure"
	@echo "  make sizes        list file sizes"

# Keep intermediate per-module binaries instead of auto-deleting them.
.PRECIOUS: $(BUILD)/%_example $(BUILD)/%_test
