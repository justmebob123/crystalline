# Crystalline UI V2 Library Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIC -I../../../include
LDFLAGS = -lSDL2 -lSDL2_ttf -lm

# Source files
SRCS = element.c container.c button.c list.c slider.c dropdown.c panel.c label.c textinput.c textarea.c
OBJS = $(SRCS:.c=.o)

# Library name
LIB = libcrystalline_v2.a

# Build library
$(LIB): $(OBJS)
	ar rcs $@ $^
	@echo "✓ Crystalline UI V2 library built"

# Compile object files
%.o: %.c %.h core.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(LIB)
	@echo "✓ Cleaned Crystalline UI V2"

.PHONY: clean

# Test demo
test_demo: test_demo.c $(LIB)
	$(CC) $(CFLAGS) test_demo.c -o test_demo -L. -lcrystalline_v2 $(LDFLAGS)
	@echo "✓ Test demo built"

.PHONY: test_demo
