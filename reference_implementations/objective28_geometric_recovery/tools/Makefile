# Tools Makefile for OBJECTIVE 28

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lssl -lcrypto

all: generate_ecdsa_samples

generate_ecdsa_samples: generate_ecdsa_samples.c
	@echo "Building ECDSA sample generator..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

clean:
	rm -f generate_ecdsa_samples
	@echo "✓ Cleaned"

.PHONY: all clean