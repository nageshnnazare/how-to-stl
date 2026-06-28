# ============================================================================
#  data_structures -- build system (undergrad-style structures & algorithms)
# ----------------------------------------------------------------------------
#  Every structure lives in <name>/ with:
#     <name>/<name>.hpp     header-only implementation (templated, commented)
#     <name>/<name>.cpp     runnable demo with built-in self-checks
#     <name>/README.md      short write-up (idea, operations, complexity)
#
#  Add a folder name to MODULES below and it automatically gains
#  make run-<name>  (build + run that demo).
#
#  Common targets:
#     make all          build every demo into build/
#     make run          build AND run every demo (stops at first failure)
#     make run-<name>   build + run one demo   (e.g. make run-graph)
#     make clean        remove build artifacts
#     make help         list everything
# ============================================================================

CXX      := g++
CXXFLAGS := -std=c++14 -Wall -Wextra -Wpedantic -O2 -I.
BUILD    := build

# One line per structure (the single source of truth).
MODULES := \
	dynamic_array singly_linked_list doubly_linked_list \
	stack queue deque \
	binary_search_tree avl_tree binary_heap trie \
	hash_table graph union_find lru_cache

DEMO_BINS := $(addprefix $(BUILD)/,$(MODULES))

.PHONY: all run clean help
.DEFAULT_GOAL := all

all: $(DEMO_BINS)

$(BUILD):
	@mkdir -p $(BUILD)

# Generate one explicit compile rule per module: build/<m> <- <m>/<m>.cpp
define DEMO_RULE
$(BUILD)/$(1): $(1)/$(1).cpp $(1)/$(1).hpp | $(BUILD)
	@echo "  CXX  $(1)"
	@$(CXX) $(CXXFLAGS) $$< -o $$@
endef
$(foreach m,$(MODULES),$(eval $(call DEMO_RULE,$(m))))

# Build, then run, every demo. Stops at the first failure.
run: $(DEMO_BINS)
	@for d in $(DEMO_BINS); do \
		echo "\n=== $$d ==="; \
		$$d || exit 1; \
	done
	@echo "\n✓ All demos ran successfully"

# make run-<name>
run-%: $(BUILD)/%
	@echo "\n=== Running $* demo ==="
	@$<

clean:
	@rm -rf $(BUILD)
	@echo "Cleaned $(BUILD)/"

help:
	@echo "data_structures targets:"
	@echo "  make all          build every demo into build/"
	@echo "  make run          build + run every demo"
	@echo "  make run-<name>   build + run one demo (e.g. make run-trie)"
	@echo "  make clean        remove build artifacts"
	@echo ""
	@echo "Modules: $(MODULES)"
