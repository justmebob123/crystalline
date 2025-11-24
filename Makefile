# ============================================================================
# Crystalline Project - Master Makefile
# ============================================================================
# Three independent libraries:
# 1. libprimemath - Core crystalline lattice math library
# 2. libcllm - Crystalline Lattice Language Model library
# 3. libcrawler - Web crawler and continuous learning library
# ============================================================================

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -Wno-format-truncation -g -O0 -fPIC -I./include -mavx2 -mfma
LDFLAGS = 
ARFLAGS = rcs

# Library names
PRIMEMATH_STATIC = libprimemath.a
PRIMEMATH_SHARED = libprimemath.so
CLLM_STATIC = libcllm.a
CLLM_SHARED = libcllm.so
CRAWLER_STATIC = libcrawler.a
CRAWLER_SHARED = libcrawler.so

# Installation directories
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include

# Source directories
SRC_CORE = src/core
SRC_TRANS = src/transcendental
SRC_GEOM = src/geometry
SRC_AI = src/ai
SRC_CRAWLER = src/crawler

# Source files organized by library
PRIMEMATH_SOURCES = $(wildcard $(SRC_CORE)/*.c) $(wildcard $(SRC_TRANS)/*.c) $(wildcard $(SRC_GEOM)/*.c)
CLLM_SOURCES = $(wildcard $(SRC_AI)/*.c)
CRAWLER_SOURCES = $(wildcard $(SRC_CRAWLER)/*.c)

# Object files
PRIMEMATH_OBJECTS = $(PRIMEMATH_SOURCES:.c=.o)
CLLM_OBJECTS = $(CLLM_SOURCES:.c=.o)
CRAWLER_OBJECTS = $(CRAWLER_SOURCES:.c=.o)

# Header files
HEADERS = $(wildcard include/*.h)

# Tools directory
TOOLS_DIR = tools

# ============================================================================
# Main Targets
# ============================================================================

.PHONY: all clean install uninstall test demos app info verify help crawler libs

all: libs
	@echo "✓ All libraries built successfully!"

libs: primemath cllm crawler
	@echo "✓ Library build complete!"
	@echo "  libprimemath: $(PRIMEMATH_STATIC), $(PRIMEMATH_SHARED)"
	@echo "  libcllm:      $(CLLM_STATIC), $(CLLM_SHARED)"
	@echo "  libcrawler:   $(CRAWLER_STATIC), $(CRAWLER_SHARED)"

# ============================================================================
# Library 1: Prime Math (Core Crystalline Lattice)
# ============================================================================

primemath: $(PRIMEMATH_STATIC) $(PRIMEMATH_SHARED)

$(PRIMEMATH_STATIC): $(PRIMEMATH_OBJECTS)
	@echo "Creating libprimemath.a (Core Crystalline Lattice Math)"
	$(AR) $(ARFLAGS) $@ $^
	@echo "✓ libprimemath.a created"

$(PRIMEMATH_SHARED): $(PRIMEMATH_OBJECTS)
	@echo "Creating libprimemath.so (Core Crystalline Lattice Math)"
	$(CC) -shared -o $@ $^ $(LDFLAGS)
	@echo "✓ libprimemath.so created"

# ============================================================================
# Library 2: CLLM (Crystalline Lattice Language Model)
# ============================================================================

cllm: primemath $(CLLM_STATIC) $(CLLM_SHARED)

$(CLLM_STATIC): $(CLLM_OBJECTS)
	@echo "Creating libcllm.a (CLLM Library)"
	$(AR) $(ARFLAGS) $@ $^
	@echo "✓ libcllm.a created"

$(CLLM_SHARED): $(CLLM_OBJECTS) $(PRIMEMATH_SHARED)
	@echo "Creating libcllm.so (CLLM Library)"
	$(CC) -shared -o $@ $(CLLM_OBJECTS) -L. -lprimemath $(LDFLAGS)
	@echo "✓ libcllm.so created"

# ============================================================================
# Library 3: Crawler (Web Crawler & Continuous Learning)
# ============================================================================

crawler-lib: cllm $(CRAWLER_STATIC) $(CRAWLER_SHARED)

$(CRAWLER_STATIC): $(CRAWLER_OBJECTS)
	@echo "Creating libcrawler.a (Crawler Library)"
	$(AR) $(ARFLAGS) $@ $^
	@echo "✓ libcrawler.a created"

$(CRAWLER_SHARED): $(CRAWLER_OBJECTS) $(CLLM_SHARED) $(PRIMEMATH_SHARED)
	@echo "Creating libcrawler.so (Crawler Library)"
	$(CC) -shared -o $@ $(CRAWLER_OBJECTS) -L. -lcllm -lprimemath -lcurl -lpthread $(LDFLAGS)
	@echo "✓ libcrawler.so created"

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

# AI/CLLM objects
$(SRC_AI)/%.o: $(SRC_AI)/%.c $(HEADERS)
	@echo "Compiling [CLLM]: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Crawler objects
$(SRC_CRAWLER)/%.o: $(SRC_CRAWLER)/%.c $(HEADERS)
	@echo "Compiling [CRAWLER]: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# Subdirectory Builds
# ============================================================================

test: libs
	@echo "Building test suite..."
	@if [ -d tests ] && [ -f tests/Makefile ]; then \
		$(MAKE) -C tests; \
	else \
		echo "No test suite found"; \
	fi

demos: libs
	@echo "Building demos..."
	@if [ -d demos ] && [ -f demos/Makefile ]; then \
		$(MAKE) -C demos; \
	else \
		echo "No demos found"; \
	fi

app: libs
	@echo "Building application..."
	@if [ -d app ] && [ -f app/Makefile ]; then \
		$(MAKE) -C app; \
	else \
		echo "No application found"; \
	fi

# ============================================================================
# Crawler Tool
# ============================================================================

crawler: crawler-lib
	@echo "Building crawler tool..."
	@mkdir -p $(TOOLS_DIR)
	$(CC) $(CFLAGS) -o $(TOOLS_DIR)/cllm_crawler tools/cllm_crawler.c \
		-L. -lcrawler -lcllm -lprimemath -lcurl -lpthread -lm
	@echo "✓ Crawler built: $(TOOLS_DIR)/cllm_crawler"

# ============================================================================
# Installation
# ============================================================================

install: libs
	@echo "Installing Crystalline libraries..."
	install -d $(DESTDIR)$(LIBDIR)
	install -m 644 $(PRIMEMATH_STATIC) $(DESTDIR)$(LIBDIR)
	install -m 755 $(PRIMEMATH_SHARED) $(DESTDIR)$(LIBDIR)
	install -m 644 $(CLLM_STATIC) $(DESTDIR)$(LIBDIR)
	install -m 755 $(CLLM_SHARED) $(DESTDIR)$(LIBDIR)
	install -m 644 $(CRAWLER_STATIC) $(DESTDIR)$(LIBDIR)
	install -m 755 $(CRAWLER_SHARED) $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)/crystalline
	install -m 644 $(HEADERS) $(DESTDIR)$(INCLUDEDIR)/crystalline
	ldconfig
	@echo "✓ Installation complete"
	@echo "  Libraries: $(LIBDIR)/"
	@echo "  Headers:   $(INCLUDEDIR)/crystalline/"

uninstall:
	@echo "Uninstalling Crystalline libraries..."
	rm -f $(DESTDIR)$(LIBDIR)/$(PRIMEMATH_STATIC)
	rm -f $(DESTDIR)$(LIBDIR)/$(PRIMEMATH_SHARED)
	rm -f $(DESTDIR)$(LIBDIR)/$(CLLM_STATIC)
	rm -f $(DESTDIR)$(LIBDIR)/$(CLLM_SHARED)
	rm -f $(DESTDIR)$(LIBDIR)/$(CRAWLER_STATIC)
	rm -f $(DESTDIR)$(LIBDIR)/$(CRAWLER_SHARED)
	rm -rf $(DESTDIR)$(INCLUDEDIR)/crystalline
	ldconfig
	@echo "✓ Uninstallation complete"

# ============================================================================
# Verification and Information
# ============================================================================

verify:
	@echo "Verifying mathematical independence..."
	@echo "Checking for math.h dependencies in core library:"
	@if grep -r "math\.h" $(SRC_CORE) $(SRC_TRANS) $(SRC_GEOM) 2>/dev/null; then \
		echo "✗ Found math.h dependencies in core library!"; \
		exit 1; \
	else \
		echo "✓ Core library is mathematically independent"; \
	fi
	@echo ""
	@echo "✓ Mathematical independence verified!"

info:
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║      Crystalline Project - Build Information                  ║"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Compiler:        $(CC)                                         "
	@echo "║ CFLAGS:          $(CFLAGS)"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Library 1: libprimemath (Core Crystalline Lattice)            ║"
	@echo "║   Core:          $(words $(wildcard $(SRC_CORE)/*.c)) files"
	@echo "║   Transcendental: $(words $(wildcard $(SRC_TRANS)/*.c)) files"
	@echo "║   Geometry:      $(words $(wildcard $(SRC_GEOM)/*.c)) files"
	@echo "║   Total:         $(words $(PRIMEMATH_SOURCES)) files"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Library 2: libcllm (CLLM)                                     ║"
	@echo "║   AI/CLLM:       $(words $(CLLM_SOURCES)) files"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Library 3: libcrawler (Crawler)                               ║"
	@echo "║   Crawler:       $(words $(CRAWLER_SOURCES)) files"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Mathematical Independence: YES (no -lm in core)               ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"

help:
	@echo "Crystalline Project - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build all three libraries (default)"
	@echo "  libs       - Build all three libraries"
	@echo "  primemath  - Build libprimemath only"
	@echo "  cllm       - Build libcllm only (requires libprimemath)"
	@echo "  crawler-lib- Build libcrawler only (requires libcllm)"
	@echo "  clean      - Remove all build artifacts"
	@echo "  test       - Build and run test suite"
	@echo "  demos      - Build demo programs"
	@echo "  app        - Build SDL application"
	@echo "  crawler    - Build crawler tool"
	@echo "  install    - Install libraries and headers (requires sudo)"
	@echo "  uninstall  - Remove installed files (requires sudo)"
	@echo "  verify     - Verify mathematical independence"
	@echo "  info       - Display build information"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Library Structure:"
	@echo "  libprimemath - Core crystalline lattice math (independent)"
	@echo "  libcllm      - CLLM (depends on libprimemath)"
	@echo "  libcrawler   - Crawler (depends on libcllm + libprimemath)"

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(PRIMEMATH_OBJECTS) $(CLLM_OBJECTS) $(CRAWLER_OBJECTS)
	rm -f $(PRIMEMATH_STATIC) $(PRIMEMATH_SHARED)
	rm -f $(CLLM_STATIC) $(CLLM_SHARED)
	rm -f $(CRAWLER_STATIC) $(CRAWLER_SHARED)
	rm -f $(TOOLS_DIR)/cllm_crawler
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
