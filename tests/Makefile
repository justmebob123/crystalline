# Test Suite Makefile for Crystalline CLLM

CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -I../include -mavx2 -mfma
LDFLAGS = -L.. -lcrystalline -lcllm -lm -Wl,-rpath,'$$ORIGIN/..'

# Test directories
UNIT_DIR = unit
INTEGRATION_DIR = integration
PERFORMANCE_DIR = performance
VALIDATION_DIR = validation

# Unit tests
UNIT_TESTS = \
	$(UNIT_DIR)/test_softmax_backward \
	$(UNIT_DIR)/test_attention_cache

# Integration tests
INTEGRATION_TESTS = \
	$(INTEGRATION_DIR)/test_forward_backward \
	$(INTEGRATION_DIR)/test_lr_scheduling

# Performance tests
PERFORMANCE_TESTS = \
	$(PERFORMANCE_DIR)/benchmark_training_speed

# Validation tests
VALIDATION_TESTS = \
	$(VALIDATION_DIR)/test_numerical_gradients

# Mathematical integration tests
MATH_TESTS = test_mathematical_integration

# All tests
ALL_TESTS = $(UNIT_TESTS) $(INTEGRATION_TESTS) $(PERFORMANCE_TESTS) $(VALIDATION_TESTS) $(MATH_TESTS)

.PHONY: all unit integration performance validation clean run-unit run-integration run-performance run-validation run-all

all: unit integration

unit: $(UNIT_TESTS)

integration: $(INTEGRATION_TESTS)

performance: $(PERFORMANCE_TESTS)

validation: $(VALIDATION_TESTS)

# Unit test compilation
$(UNIT_DIR)/test_softmax_backward: $(UNIT_DIR)/test_softmax_backward.c
	@echo "Building unit test: test_softmax_backward..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_softmax_backward built"

$(UNIT_DIR)/test_attention_cache: $(UNIT_DIR)/test_attention_cache.c
	@echo "Building unit test: test_attention_cache..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_attention_cache built"

# Integration test compilation
$(INTEGRATION_DIR)/test_forward_backward: $(INTEGRATION_DIR)/test_forward_backward.c
	@echo "Building integration test: test_forward_backward..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_forward_backward built"

$(INTEGRATION_DIR)/test_lr_scheduling: $(INTEGRATION_DIR)/test_lr_scheduling.c
	@echo "Building integration test: test_lr_scheduling..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_lr_scheduling built"

# Performance test compilation
$(PERFORMANCE_DIR)/benchmark_training_speed: $(PERFORMANCE_DIR)/benchmark_training_speed.c
	@echo "Building performance test: benchmark_training_speed..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ benchmark_training_speed built"

# Validation test compilation
$(VALIDATION_DIR)/test_numerical_gradients: $(VALIDATION_DIR)/test_numerical_gradients.c
	@echo "Building validation test: test_numerical_gradients..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_numerical_gradients built"

# Run tests
run-unit: unit
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Unit Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(UNIT_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All unit tests completed"

run-integration: integration
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Integration Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(INTEGRATION_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All integration tests completed"

run-performance: performance
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Performance Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(PERFORMANCE_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All performance tests completed"

run-validation: validation
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Validation Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(VALIDATION_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All validation tests completed"

run-all: all
	@$(MAKE) run-unit
	@$(MAKE) run-integration

clean:
	@echo "Cleaning test executables..."
	@rm -f $(ALL_TESTS)
	@echo "✓ Tests cleaned"

help:
	@echo "Crystalline CLLM Test Suite"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build unit and integration tests (default)"
	@echo "  unit             - Build unit tests only"
	@echo "  integration      - Build integration tests only"
	@echo "  performance      - Build performance tests only"
	@echo "  validation       - Build validation tests only"
	@echo ""
	@echo "  run-unit         - Run unit tests"
	@echo "  run-integration  - Run integration tests"
	@echo "  run-performance  - Run performance tests"
	@echo "  run-validation   - Run validation tests"
	@echo "  run-all          - Run all tests"
	@echo ""
	@echo "  clean            - Remove test executables"
	@echo "  help             - Show this help message"
# Mathematical integration test
test_mathematical_integration: test_mathematical_integration.c
	@echo "Building mathematical integration test..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Mathematical integration test built"

run-math: test_mathematical_integration
	@echo ""
	@echo "Running mathematical integration tests..."
	@echo ""
	@./test_mathematical_integration
	@echo ""
	@echo "✓ Mathematical integration tests completed"
