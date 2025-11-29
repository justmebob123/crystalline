# Makefile for libalgorithms - Mathematical Algorithms Library

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11 -D_GNU_SOURCE
LDFLAGS = -shared
ARFLAGS = rcs
INCLUDES = -I./include -I../include

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
          src/hierarchical_primes.c \
          src/hierarchical_structures.c \
          src/batch_processing.c \
          src/hierarchical_prime_partitions.c \
          src/lattice_sphere_positions.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Output libraries
SHARED_LIB = libalgorithms.so
STATIC_LIB = libalgorithms.a
CRYSTALLINE_LIB = ../libcrystalline.so

.PHONY: all static clean install

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
