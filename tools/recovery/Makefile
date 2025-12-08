# Makefile for recovery tools

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../algorithms/include -I../../src
LDFLAGS = -L../../algorithms/lib/recovery_common -L../..
LIBS = -lrecovery_common -lalgorithms -lcrystalline -lm

# Tools
TOOLS = geometric-recovery

# Default target
all: $(TOOLS)

# Build geometric-recovery
geometric-recovery: geometric-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Clean
clean:
	rm -f $(TOOLS)

# Install
install: $(TOOLS)
	mkdir -p ../../bin
	cp $(TOOLS) ../../bin/

.PHONY: all clean install