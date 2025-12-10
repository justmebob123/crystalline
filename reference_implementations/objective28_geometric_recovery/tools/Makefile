# Tools Makefile for OBJECTIVE 28

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I../include
LDFLAGS = -L../lib -lgeometric_recovery -lm -lssl -lcrypto

all: generate_ecdsa_samples generate_comprehensive_test_data run_comprehensive_tests test_real_recovery_comprehensive geometric_recovery_cli

generate_ecdsa_samples: generate_ecdsa_samples.c
	@echo "Building ECDSA sample generator..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

generate_comprehensive_test_data: generate_comprehensive_test_data.c
	@echo "Building comprehensive test data generator..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

run_comprehensive_tests: run_comprehensive_tests.c
	@echo "Building comprehensive test runner..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

test_real_recovery_comprehensive: test_real_recovery_comprehensive.c
	@echo "Building real recovery comprehensive test..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

geometric_recovery_cli: geometric_recovery_cli.c
	@echo "Building unified CLI tool..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

clean:
	rm -f generate_ecdsa_samples generate_comprehensive_test_data run_comprehensive_tests test_real_recovery_comprehensive geometric_recovery_cli
	@echo "✓ Cleaned"

.PHONY: all clean