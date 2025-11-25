# Makefile for libalgorithms.so - Mathematical Algorithms Library

CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11 -D_GNU_SOURCE
LDFLAGS = -shared
INCLUDES = -I./include -I../include

# Source files
SOURCES = src/numerical.c \
          src/loss_functions.c \
          src/optimizers.c \
          src/backprop.c \
          src/statistics.c \
          src/threading.c \
          src/shared_memory.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Output library
LIBRARY = libalgorithms.so
CRYSTALLINE_LIB = ../libcrystalline.so
# Crystalline library dependency

.PHONY: all clean install

all: $(LIBRARY)
$(LIBRARY): $(OBJECTS) $(CRYSTALLINE_LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) -L.. -lcrystalline

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(LIBRARY)

install: $(LIBRARY)
	cp $(LIBRARY) /usr/local/lib/
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
