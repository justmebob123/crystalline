# OBJECTIVE 28: Geometric Recovery Algorithm - Independent Makefile
# This Makefile builds the complete geometric recovery system independently

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I./include
LDFLAGS = -lm -lssl -lcrypto -lpthread

# Directories
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
BUILD_DIR = build
LIB_DIR = lib

# Output
LIB_NAME = libgeometric_recovery.a
LIB_SO = libgeometric_recovery.so

# Source files
SOURCES = \
	$(SRC_DIR)/geometric_recovery_complete.c \
	$(SRC_DIR)/geometric_utils.c \
	$(SRC_DIR)/oscillation_vector.c \
	$(SRC_DIR)/q_validation.c \
	$(SRC_DIR)/quadrant_polarity.c \
	$(SRC_DIR)/shared_geometry.c \
	$(SRC_DIR)/platonic_model_core.c \
	$(SRC_DIR)/platonic_model_oscillations.c \
	$(SRC_DIR)/platonic_model_recovery.c \
	$(SRC_DIR)/platonic_model_scaling.c \
	$(SRC_DIR)/platonic_model_persistence.c \
	$(SRC_DIR)/tetration_real.c \
	$(SRC_DIR)/prime_float_math.c \
	$(SRC_DIR)/iterative_recovery.c \
	$(SRC_DIR)/iterative_recovery_v2.c \
        $(SRC_DIR)/recursive_recovery.c \
        $(SRC_DIR)/clock_recovery.c \
        $(SRC_DIR)/platonic_solids.c \
        $(SRC_DIR)/k_recovery_enhanced.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test programs
TEST_SOURCES = \
	$(TEST_DIR)/test_iterative_recovery_v2.c \
	$(TEST_DIR)/test_geometric_recovery.c \
	$(TEST_DIR)/test_torus_recovery_v2.c

TEST_BINS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)

# Default target
all: directories lib tests

# Create directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)

# Build static library
lib: $(LIB_DIR)/$(LIB_NAME)

$(LIB_DIR)/$(LIB_NAME): $(OBJECTS)
	@echo "Creating static library: $@"
	ar rcs $@ $^
	@echo "Static library created successfully"

# Build shared library
shared: $(LIB_DIR)/$(LIB_SO)

$(LIB_DIR)/$(LIB_SO): $(OBJECTS)
	@echo "Creating shared library: $@"
	$(CC) -shared -o $@ $^ $(LDFLAGS)
	@echo "Shared library created successfully"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Build test programs
tests: $(TEST_BINS)

$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(LIB_DIR)/$(LIB_NAME)
	@echo "Building test: $@"
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS) -o $@

# Run tests
test: tests
	@echo "Running test_iterative_recovery_v2..."
	@$(BUILD_DIR)/test_iterative_recovery_v2 || true
	@echo ""
	@echo "Running test_geometric_recovery..."
	@$(BUILD_DIR)/test_geometric_recovery || true
	@echo ""
	@echo "Running test_torus_recovery_v2..."
	@$(BUILD_DIR)/test_torus_recovery_v2 || true

# Quick test (just iterative recovery)
quicktest: $(BUILD_DIR)/test_iterative_recovery_v2
	@echo "Running quick test..."
	@$(BUILD_DIR)/test_iterative_recovery_v2

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
	@echo "Clean complete"

# Install library system-wide (requires sudo)
install: lib
	@echo "Installing library to /usr/local/lib..."
	sudo cp $(LIB_DIR)/$(LIB_NAME) /usr/local/lib/
	@echo "Installing headers to /usr/local/include..."
	sudo mkdir -p /usr/local/include/geometric_recovery
	sudo cp $(INC_DIR)/*.h /usr/local/include/geometric_recovery/
	sudo ldconfig
	@echo "Installation complete"

# Uninstall library
uninstall:
	@echo "Uninstalling library..."
	sudo rm -f /usr/local/lib/$(LIB_NAME)
	sudo rm -rf /usr/local/include/geometric_recovery
	sudo ldconfig
	@echo "Uninstall complete"

# Show file sizes
sizes: lib
	@echo "Library sizes:"
	@ls -lh $(LIB_DIR)/$(LIB_NAME)
	@echo ""
	@echo "Object file sizes:"
	@ls -lh $(BUILD_DIR)/*.o | awk '{print $$9, $$5}'

# Count lines of code
loc:
	@echo "Lines of code:"
	@echo "Source files:"
	@wc -l $(SOURCES) | tail -1
	@echo ""
	@echo "Header files:"
	@wc -l $(INC_DIR)/*.h | tail -1
	@echo ""
	@echo "Test files:"
	@wc -l $(TEST_SOURCES) | tail -1
	@echo ""
	@echo "Total:"
	@cat $(SOURCES) $(INC_DIR)/*.h $(TEST_SOURCES) | wc -l

# Generate documentation
docs:
	@echo "Generating documentation..."
	@echo "See README.md, ARCHITECTURE.md, and ALGORITHM_EXPLAINED.md"
	@echo "Documentation is already complete in docs/ directory"

# Check for common issues
check:
	@echo "Checking for common issues..."
	@echo "Checking for math.h usage (should use prime_float_math.h):"
	@grep -n "include <math.h>" $(SOURCES) || echo "  ✓ No math.h found"
	@echo ""
	@echo "Checking for memory leaks (requires valgrind):"
	@which valgrind > /dev/null && echo "  Run: make valgrind" || echo "  Install valgrind to check for leaks"
	@echo ""
	@echo "Checking for undefined behavior (requires ubsan):"
	@echo "  Rebuild with: CFLAGS += -fsanitize=undefined"

# Memory leak check (requires valgrind)
valgrind: $(BUILD_DIR)/test_iterative_recovery_v2
	@echo "Running valgrind memory leak check..."
	valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --verbose \
		$(BUILD_DIR)/test_iterative_recovery_v2

# Performance profiling (requires gprof)
profile: CFLAGS += -pg
profile: clean all
	@echo "Running with profiling enabled..."
	@$(BUILD_DIR)/test_iterative_recovery_v2
	@gprof $(BUILD_DIR)/test_iterative_recovery_v2 gmon.out > profile.txt
	@echo "Profile saved to profile.txt"

# Benchmark all tests
benchmark: tests
	@echo "Benchmarking all tests..."
	@echo "Test 1: Iterative Recovery V2"
	@time $(BUILD_DIR)/test_iterative_recovery_v2
	@echo ""
	@echo "Test 2: Geometric Recovery"
	@time $(BUILD_DIR)/test_geometric_recovery
	@echo ""
	@echo "Test 3: Torus Recovery V2"
	@time $(BUILD_DIR)/test_torus_recovery_v2

# Help
help:
	@echo "OBJECTIVE 28: Geometric Recovery Algorithm - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build library and tests (default)"
	@echo "  lib          - Build static library only"
	@echo "  shared       - Build shared library"
	@echo "  tests        - Build test programs"
	@echo "  test         - Run all tests"
	@echo "  quicktest    - Run quick test only"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install library system-wide (requires sudo)"
	@echo "  uninstall    - Uninstall library"
	@echo "  sizes        - Show file sizes"
	@echo "  loc          - Count lines of code"
	@echo "  docs         - Generate documentation"
	@echo "  check        - Check for common issues"
	@echo "  valgrind     - Run memory leak check"
	@echo "  profile      - Run performance profiling"
	@echo "  benchmark    - Benchmark all tests"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build everything"
	@echo "  make test         # Build and run tests"
	@echo "  make quicktest    # Quick test"
	@echo "  make clean all    # Clean rebuild"
	@echo "  make install      # Install system-wide"

.PHONY: all directories lib shared tests test quicktest clean install uninstall \
        sizes loc docs check valgrind profile benchmark help

# Clock recovery test
$(BUILD_DIR)/test_clock_mapping: $(TEST_DIR)/test_clock_mapping.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-clock
test-clock: $(BUILD_DIR)/test_clock_mapping
	@echo "Running clock mapping tests..."
	@$(BUILD_DIR)/test_clock_mapping

# Platonic solids test
$(BUILD_DIR)/test_platonic_solids: $(TEST_DIR)/test_platonic_solids.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-platonic
test-platonic: $(BUILD_DIR)/test_platonic_solids
	@echo "Running Platonic solids tests..."
	@$(BUILD_DIR)/test_platonic_solids

# Integrated recovery test
$(BUILD_DIR)/test_integrated_recovery: $(TEST_DIR)/test_integrated_recovery.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-integrated
test-integrated: $(BUILD_DIR)/test_integrated_recovery
	@echo "Running integrated recovery tests..."
	@$(BUILD_DIR)/test_integrated_recovery

.PHONY: test-all
test-all: test-clock test-platonic test-integrated
	@echo ""
	@echo "All tests completed!"

# Enhanced k recovery test
$(BUILD_DIR)/test_k_recovery_enhanced: $(TEST_DIR)/test_k_recovery_enhanced.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-k-recovery
test-k-recovery: $(BUILD_DIR)/test_k_recovery_enhanced
	@echo "Running enhanced k recovery tests..."
	@$(BUILD_DIR)/test_k_recovery_enhanced

# Deep recovery analysis test
$(BUILD_DIR)/test_deep_recovery_analysis: $(TEST_DIR)/test_deep_recovery_analysis.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-deep-analysis
test-deep-analysis: $(BUILD_DIR)/test_deep_recovery_analysis
	@echo "Running deep recovery analysis..."
	@$(BUILD_DIR)/test_deep_recovery_analysis

# Quadrant analysis test
$(BUILD_DIR)/test_quadrant_analysis: $(TEST_DIR)/test_quadrant_analysis.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-quadrant
test-quadrant: $(BUILD_DIR)/test_quadrant_analysis
	@echo "Running quadrant analysis..."
	@$(BUILD_DIR)/test_quadrant_analysis
