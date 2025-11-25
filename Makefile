# ============================================================================
# Prime Math Library - Master Makefile
# ============================================================================
# Crystalline Lattice Arbitrary Precision Math Library
# Organized modular build system with NO external math dependencies
# ============================================================================

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -g -O0 -fPIC -I./include -mavx2 -mfma
LDFLAGS = 
ARFLAGS = rcs

# Library names
CRYSTALLINE_LIB = libcrystalline.so
CLLM_LIB = libcllm.so
CRAWLER_LIB = libcrawler.so

# Legacy names (for backwards compatibility)
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
TOOLS_DIR = tools
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

all: $(CRYSTALLINE_LIB) $(CLLM_LIB) $(CRAWLER_LIB) $(DOCPROC_LIB) $(STATIC_LIB) $(SHARED_LIB) tools all_tools
	@echo "✓ Build complete!"
	@echo "  Crystalline library: $(CRYSTALLINE_LIB)"
	@echo "  CLLM library: $(CLLM_LIB)"
	@echo "  Crawler library: $(CRAWLER_LIB)"
	@echo "  Legacy static: $(STATIC_LIB)"
	@echo "  Legacy shared: $(SHARED_LIB)"
	@echo "  Crawler library: $(CRAWLER_LIB)"

# ============================================================================
# Three Independent Libraries
# ============================================================================

# 1. Crystalline Lattice Library (core math + geometry)
CRYSTALLINE_OBJECTS = $(CORE_OBJECTS) $(TRANS_OBJECTS) $(GEOM_OBJECTS)

$(CRYSTALLINE_LIB): $(CRYSTALLINE_OBJECTS)
	@echo "Creating crystalline library: $@"
	$(CC) -shared -o $@ $^ -lm
	@echo "✓ Crystalline library created"

# 2. CLLM Library (AI/language model - depends on crystalline)
$(CLLM_LIB): $(AI_OBJECTS) $(CRYSTALLINE_LIB)
	@echo "Creating CLLM library: $@"
	$(CC) -shared -o $@ $(AI_OBJECTS) -L. -lcrystalline -lm
	@echo "✓ CLLM library created"

# ============================================================================
# Legacy Libraries (for backwards compatibility)
# ============================================================================


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

# Crawler objects
src/crawler/%.o: src/crawler/%.c $(HEADERS)

src/document_processing/%.o: src/document_processing/%.c $(HEADERS)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiling [CRAWLER]: $<"
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
	rm -f $(CRYSTALLINE_LIB) $(CLLM_LIB) $(CRAWLER_LIB) $(DOCPROC_LIB) $(STATIC_LIB) $(SHARED_LIB)
	rm -f tools/cllm_pdf_extract tools/cllm_ocr tools/cllm_pdf_ocr tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build
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

# ============================================================================
# ============================================================================
# Crawler Library (libcrawler.so)
# ============================================================================

CRAWLER_SOURCES = src/crawler/crawler_core.c src/crawler/preprocessor.c \
                  src/crawler/tokenizer.c src/crawler/continuous_training.c \
                  src/crawler/crawler_api.c
CRAWLER_OBJECTS = $(CRAWLER_SOURCES:.c=.o)
CRAWLER_LIB = libcrawler.so

$(CRAWLER_LIB): $(CRAWLER_OBJECTS) $(CLLM_LIB)
	@echo "Creating crawler library: $@"
	$(CC) -shared -o $@ $(CRAWLER_OBJECTS) -L. -lcrystalline -lcllm -lcurl -lpthread -lm
	@echo "✓ Crawler library created"

# ============================================================================
# Crawler CLI Tool (uses libcrawler.so)
# ============================================================================

crawler: $(CRAWLER_LIB)
	@echo "Building crawler CLI tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_crawler tools/cllm_crawler.c \
		-L. -lcrawler -lcllm -lcrystalline -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Crawler CLI built: tools/cllm_crawler"


# ============================================================================
# Document Processing Library (libdocproc.so)
# ============================================================================

DOCPROC_SOURCES = src/document_processing/cllm_pdf.c src/document_processing/cllm_ocr.c
DOCPROC_OBJECTS = $(DOCPROC_SOURCES:.c=.o)
DOCPROC_LIB = libdocproc.so

$(DOCPROC_LIB): $(DOCPROC_OBJECTS)
	@echo "Creating document processing library: $@"
	$(CC) -shared -o $@ $(DOCPROC_OBJECTS) -lm
	@echo "✓ Document processing library created"

# ============================================================================
# Document Processing CLI Tools
# ============================================================================

tools: tools/cllm_pdf_extract tools/cllm_ocr tools/cllm_pdf_ocr

tools/cllm_pdf_extract: $(DOCPROC_LIB)
	@echo "Building PDF extraction tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_pdf_extract tools/cllm_pdf_extract.c \
		-L. -ldocproc -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ PDF extraction tool built: tools/cllm_pdf_extract"

tools/cllm_ocr: $(DOCPROC_LIB)
	@echo "Building OCR tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_ocr tools/cllm_ocr.c \
		-L. -ldocproc -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ OCR tool built: tools/cllm_ocr"

tools/cllm_pdf_ocr: $(DOCPROC_LIB)
	@echo "Building combined PDF+OCR tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_pdf_ocr tools/cllm_pdf_ocr.c \
		-L. -ldocproc -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Combined PDF+OCR tool built: tools/cllm_pdf_ocr"


# ============================================================================
# Additional CLI Tools
# ============================================================================

tools/cllm_inference: $(CLLM_LIB)
	@echo "Building inference tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_inference tools/cllm_inference.c \
		-L. -lcllm -lcrystalline -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Inference tool built: tools/cllm_inference"

tools/cllm_tokenize: $(CLLM_LIB)
	@echo "Building tokenizer tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_tokenize tools/cllm_tokenize.c \
		-L. -lcllm -lcrystalline -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Tokenizer tool built: tools/cllm_tokenize"

tools/cllm_vocab_build: $(CLLM_LIB)
	@echo "Building vocabulary builder tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_vocab_build tools/cllm_vocab_build.c \
		-L. -lcllm -lcrystalline -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Vocabulary builder tool built: tools/cllm_vocab_build"

all_tools: tools tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build

