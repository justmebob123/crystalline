# Makefile for libalgorithms - Mathematical Algorithms Library

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11 -D_GNU_SOURCE
LDFLAGS = -shared
ARFLAGS = rcs
INCLUDES = -I./include -I./include/blind_recovery -I../include

# Source files
SOURCES = src/numerical.c \
          src/loss_functions.c \
          src/optimizers.c \
          src/backprop.c \
          src/statistics.c \
          src/threading.c \
          src/shared_memory.c \
          src/lock_free_queue.c \
          src/sphere_packing.c \
          src/sphere_threading.c \
          src/memory_management.c \
          src/visualization.c \
          src/visualization_2d.c \
          src/visualization_3d.c \
          src/visualization_crystalline.c \
          src/hierarchical_primes.c \
          src/hierarchical_structures.c \
          src/batch_processing.c \
          src/hierarchical_prime_partitions.c \
          src/lattice_sphere_positions.c \
          src/angular_attention.c \
          src/cymatic_modulation.c \
          src/ntt_attention.c \
          src/lattice_embeddings.c \
          src/lattice_embeddings_bigfixed.c \
          src/loss_functions_bigfixed.c \
          src/tensor_ops.c \
          src/qk_mapping.c \
          src/iterative_search.c \
          src/validation.c \
          src/mathematical_formulas.c \
          src/symbolic_field_theory.c \
          src/nonce_generation.c \
          src/bigfixed_math_wrappers.c \
          src/blind_recovery/oscillation_detection.c \
          src/blind_recovery/structural_mapping.c \
          src/blind_recovery/coprime_analysis.c \
          src/blind_recovery/corruption_detection.c \
          src/blind_recovery/anchor_selection.c \
          src/blind_recovery/triangulation.c \
          src/blind_recovery/anchor_adjustment.c \
          src/blind_recovery/confidence_scoring.c \
          src/blind_recovery/candidate_generation.c \
          src/blind_recovery/fitness_scoring.c \
          src/blind_recovery/iterative_refinement.c \
          src/blind_recovery/convergence_detection.c \
          src/blind_recovery/multi_scale_analysis.c \
          src/blind_recovery/recursive_stabilization.c \
          src/blind_recovery/stabilization_metrics.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Output libraries
SHARED_LIB = libalgorithms.so
STATIC_LIB = libalgorithms.a
CRYSTALLINE_LIB = ../libcrystalline.so

.PHONY: all static clean install test test_phase1 test_phase2

all: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS) $(CRYSTALLINE_LIB)
	@echo "Creating shared algorithms library: $@"
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) -L.. -lcrystalline -lm
	@echo "✓ Shared algorithms library created"

static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	@echo "Creating static algorithms library: $@"
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	@echo "✓ Static algorithms library created"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB)

install: $(SHARED_LIB) $(STATIC_LIB)
	cp $(SHARED_LIB) $(STATIC_LIB) /usr/local/lib/
	cp include/*.h /usr/local/include/
	ldconfig

# Dependencies
src/numerical.o: src/numerical.c include/numerical.h
src/loss_functions.o: src/loss_functions.c include/loss_functions.h include/numerical.h
src/optimizers.o: src/optimizers.c include/optimizers.h
src/backprop.o: src/backprop.c include/backprop.h include/numerical.h
src/statistics.o: src/statistics.c include/statistics.h
src/threading.o: src/threading.c include/threading.h
src/shared_memory.o: src/shared_memory.c include/shared_memory.h
src/lock_free_queue.o: src/lock_free_queue.c include/lock_free_queue.h
src/sphere_packing.o: src/sphere_packing.c include/sphere_packing.h
src/hierarchical_primes.o: src/hierarchical_primes.c include/hierarchical_primes.h
src/hierarchical_structures.o: src/hierarchical_structures.c include/hierarchical_structures.h
src/batch_processing.o: src/batch_processing.c include/batch_processing.h

# Test targets
test: test_phase1 test_phase2

test_phase1: tests/test_phase1.c $(SHARED_LIB)
	@echo "Building Phase 1 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase1 tests/test_phase1.c -L. -lalgorithms -L.. -lcrystalline -lm
	@echo "Running Phase 1 tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase1

test_phase2: tests/test_phase2.c $(SHARED_LIB)
	@echo "Building Phase 2 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase2 tests/test_phase2.c -L. -lalgorithms -L.. -lcrystalline -lm
	@echo "Running Phase 2 tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase2

test_phase3: tests/test_phase3.c $(SHARED_LIB)
	@echo "Building Phase 3 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase3 tests/test_phase3.c -L. -lalgorithms -L.. -lcrystalline -lm
	@echo "Running Phase 3 tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase3

test_phase4: tests/test_phase4_comprehensive.c $(SHARED_LIB)
	@echo "Building Phase 4 comprehensive tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase4_comprehensive tests/test_phase4_comprehensive.c -L. -lalgorithms -L.. -lcrystalline -lm
	@echo "Running Phase 4 comprehensive tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase4_comprehensive
