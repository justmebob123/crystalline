# ============================================================================
# Prime Math Library - Master Makefile
# ============================================================================
# Crystalline Lattice Arbitrary Precision Math Library
# Organized modular build system with NO external math dependencies
# ============================================================================

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -I./include
LDFLAGS = 
ARFLAGS = rcs

# Library names
STATIC_LIB = libprimemath.a
SHARED_LIB = libprimemath.so

# Installation directories
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include

# Source directories
SRC_CORE = src/core
SRC_TRANS = src/transcendental
SRC_GEOM = src/geometry
SRC_AI = src/ai
SRC_UTILS = src/utils

# Source files organized by category
CORE_SOURCES = $(wildcard $(SRC_CORE)/*.c)
TRANS_SOURCES = $(wildcard $(SRC_TRANS)/*.c)
GEOM_SOURCES = $(wildcard $(SRC_GEOM)/*.c)
AI_SOURCES = $(wildcard $(SRC_AI)/*.c)
UTILS_SOURCES = $(wildcard $(SRC_UTILS)/*.c)

# All sources
ALL_SOURCES = $(CORE_SOURCES) $(TRANS_SOURCES) $(GEOM_SOURCES) $(AI_SOURCES) $(UTILS_SOURCES)

# Object files
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
TRANS_OBJECTS = $(TRANS_SOURCES:.c=.o)
GEOM_OBJECTS = $(GEOM_SOURCES:.c=.o)
AI_OBJECTS = $(AI_SOURCES:.c=.o)
UTILS_OBJECTS = $(UTILS_SOURCES:.c=.o)
ALL_OBJECTS = $(ALL_SOURCES:.c=.o)

# Header files
HEADERS = $(wildcard include/*.h)

# ============================================================================
# Main Targets
# ============================================================================

.PHONY: all clean install uninstall test demos app info verify help

all: $(STATIC_LIB) $(SHARED_LIB)
	@echo "✓ Build complete!"
	@echo "  Static library: $(STATIC_LIB)"
	@echo "  Shared library: $(SHARED_LIB)"

# Static library
$(STATIC_LIB): $(ALL_OBJECTS)
	@echo "Creating static library: $@"
	$(AR) $(ARFLAGS) $@ $^
	@echo "✓ Static library created"

# Shared library
$(SHARED_LIB): $(ALL_OBJECTS)
	@echo "Creating shared library: $@"
	$(CC) -shared -o $@ $^ $(LDFLAGS)
	@echo "✓ Shared library created"

# ============================================================================
# Compilation Rules
# ============================================================================

# Core objects
$(SRC_CORE)/%.o: $(SRC_CORE)/%.c $(HEADERS)
	@echo "Compiling [CORE]: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Transcendental objects
$(SRC_TRANS)/%.o: $(SRC_TRANS)/%.c $(HEADERS)
	@echo "Compiling [TRANS]: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Geometry objects
$(SRC_GEOM)/%.o: $(SRC_GEOM)/%.c $(HEADERS)
	@echo "Compiling [GEOM]: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# AI objects
$(SRC_AI)/%.o: $(SRC_AI)/%.c $(HEADERS)
	@echo "Compiling [AI]: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# Subdirectory Builds
# ============================================================================

test: $(STATIC_LIB)
	@echo "Building test suite..."
	@if [ -d tests ] && [ -f tests/Makefile ]; then \
		$(MAKE) -C tests; \
	else \
		echo "No test suite found"; \
	fi

demos: $(STATIC_LIB)
	@echo "Building demos..."
	@if [ -d demos ] && [ -f demos/Makefile ]; then \
		$(MAKE) -C demos; \
	else \
		echo "No demos found"; \
	fi

app: $(STATIC_LIB)
	@echo "Building application..."
	@if [ -d app ] && [ -f app/Makefile ]; then \
		$(MAKE) -C app; \
	else \
		echo "No application found"; \
	fi

# ============================================================================
# Installation
# ============================================================================

install: $(STATIC_LIB) $(SHARED_LIB)
	@echo "Installing Prime Math Library..."
	install -d $(DESTDIR)$(LIBDIR)
	install -m 644 $(STATIC_LIB) $(DESTDIR)$(LIBDIR)
	install -m 755 $(SHARED_LIB) $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)/primemath
	install -m 644 $(HEADERS) $(DESTDIR)$(INCLUDEDIR)/primemath
	ldconfig
	@echo "✓ Installation complete"

uninstall:
	@echo "Uninstalling Prime Math Library..."
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB)
	rm -rf $(DESTDIR)$(INCLUDEDIR)/primemath
	ldconfig
	@echo "✓ Uninstallation complete"

# ============================================================================
# Verification and Information
# ============================================================================

verify:
	@echo "Verifying mathematical independence..."
	@echo "Checking for math.h dependencies:"
	@if grep -r "math\.h" $(SRC_CORE) $(SRC_TRANS) $(SRC_GEOM) 2>/dev/null; then \
		echo "✗ Found math.h dependencies in core library!"; \
		exit 1; \
	else \
		echo "✓ Core library is mathematically independent"; \
	fi
	@echo ""
	@echo "Checking for -lm flag:"
	@if echo "$(LDFLAGS)" | grep -q "\-lm"; then \
		echo "✗ Found -lm flag in LDFLAGS!"; \
		exit 1; \
	else \
		echo "✓ No -lm flag present"; \
	fi
	@echo ""
	@echo "✓ Mathematical independence verified!"

info:
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║         Prime Math Library - Build Information                ║"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Compiler:        $(CC)                                         "
	@echo "║ CFLAGS:          $(CFLAGS)"
	@echo "║ LDFLAGS:         $(LDFLAGS)"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Source Files:                                                  ║"
	@echo "║   Core:          $(words $(CORE_SOURCES)) files                "
	@echo "║   Transcendental: $(words $(TRANS_SOURCES)) files              "
	@echo "║   Geometry:      $(words $(GEOM_SOURCES)) files                "
	@echo "║   AI/CLLM:       $(words $(AI_SOURCES)) files                  "
	@echo "║   Total:         $(words $(ALL_SOURCES)) files                 "
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Output:                                                        ║"
	@echo "║   Static:        $(STATIC_LIB)                                 "
	@echo "║   Shared:        $(SHARED_LIB)                                 "
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Mathematical Independence: YES (no -lm, no math.h)            ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"

help:
	@echo "Prime Math Library - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build static and shared libraries (default)"
	@echo "  clean      - Remove all build artifacts"
	@echo "  test       - Build and run test suite"
	@echo "  demos      - Build demo programs"
	@echo "  app        - Build SDL application"
	@echo "  install    - Install libraries and headers (requires sudo)"
	@echo "  uninstall  - Remove installed files (requires sudo)"
	@echo "  verify     - Verify mathematical independence"
	@echo "  info       - Display build information"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build everything"
	@echo "  make clean all    # Clean rebuild"
	@echo "  make test         # Build and run tests"
	@echo "  sudo make install # Install system-wide"

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(ALL_OBJECTS)
	rm -f $(STATIC_LIB) $(SHARED_LIB)
	@if [ -d tests ]; then $(MAKE) -C tests clean 2>/dev/null || true; fi
	@if [ -d demos ]; then $(MAKE) -C demos clean 2>/dev/null || true; fi
	@if [ -d app ]; then $(MAKE) -C app clean 2>/dev/null || true; fi
	@echo "✓ Clean complete"

# ============================================================================
# Debug Build
# ============================================================================

debug: CFLAGS += -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: clean all
	@echo "✓ Debug build complete"