# ============================================================================
# Prime Math Library - Master Makefile
# ============================================================================
# Crystalline Lattice Arbitrary Precision Math Library
# Organized modular build system with NO external math dependencies
# ============================================================================

# Compiler and flags
CC = gcc
AR = ar

# Detect CPU capabilities for SIMD optimization
# Check if AVX2 is available (optional, gracefully degrades)
SIMD_FLAGS := $(shell grep -q avx2 /proc/cpuinfo 2>/dev/null && echo "-mavx2 -mfma" || echo "")

CFLAGS = -Wall -Wextra -g -O2 -fPIC -I./include -I./algorithms/include $(SIMD_FLAGS)
LDFLAGS = 
ARFLAGS = rcs

# Library names - Shared (.so)
CRYSTALLINE_LIB = libcrystalline.so
ALGORITHMS_LIB = libalgorithms.so
CLLM_LIB = libcllm.so
CRAWLER_LIB = libcrawler.so

# Library names - Static (.a)
CRYSTALLINE_STATIC = libcrystalline.a
ALGORITHMS_STATIC = libalgorithms.a
CLLM_STATIC = libcllm.a
CRAWLER_STATIC = libcrawler.a

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
INFRASTRUCTURE_SOURCES = $(wildcard src/ai/infrastructure/*.c)
AI_SOURCES += $(INFRASTRUCTURE_SOURCES)
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

all: $(CRYSTALLINE_LIB) $(CRYSTALLINE_STATIC) $(ALGORITHMS_LIB) $(ALGORITHMS_STATIC) $(CLLM_LIB) $(CLLM_STATIC) $(CRAWLER_LIB) $(CRAWLER_STATIC) $(DOCPROC_LIB) tools
	@echo "✓ Build complete!"
	@echo "  Shared Libraries:"
	@echo "    - $(CRYSTALLINE_LIB)"
	@echo "    - $(ALGORITHMS_LIB)"
	@echo "    - $(CLLM_LIB)"
	@echo "    - $(CRAWLER_LIB)"
	@echo "  Static Libraries:"
	@echo "    - $(CRYSTALLINE_STATIC)"
	@echo "    - $(ALGORITHMS_STATIC)"
	@echo "    - $(CLLM_STATIC)"
	@echo "    - $(CRAWLER_STATIC)"

# ============================================================================
# Three Independent Libraries
# ============================================================================

# 1. Crystalline Lattice Library (core math + geometry)
CRYSTALLINE_OBJECTS = $(CORE_OBJECTS) $(TRANS_OBJECTS) $(GEOM_OBJECTS)

$(CRYSTALLINE_LIB): $(CRYSTALLINE_OBJECTS)
	@echo "Creating crystalline shared library: $@"
	$(CC) -shared -o $@ $^
	@echo "✓ Crystalline shared library created"

$(CRYSTALLINE_STATIC): $(CRYSTALLINE_OBJECTS)
	@echo "Creating crystalline static library: $@"
	$(AR) $(ARFLAGS) $@ $^
	@echo "✓ Crystalline static library created"

# 3. CLLM Library (AI/language model - depends on crystalline and algorithms)
# 2. Algorithms Library (mathematical algorithms - depends on crystalline)
$(ALGORITHMS_LIB): $(CRYSTALLINE_LIB)
	@echo "Building algorithms shared library..."
	@$(MAKE) -C algorithms
	@cp algorithms/$(ALGORITHMS_LIB) .
	@echo "✓ Algorithms shared library created"

$(ALGORITHMS_STATIC): $(CRYSTALLINE_STATIC)
	@echo "Building algorithms static library..."
	@$(MAKE) -C algorithms static
	@cp algorithms/$(ALGORITHMS_STATIC) .
	@echo "✓ Algorithms static library created"

$(CLLM_LIB): $(AI_OBJECTS) $(CRYSTALLINE_LIB) $(ALGORITHMS_LIB)
	@echo "Creating CLLM shared library: $@"
	$(CC) -shared -o $@ $(AI_OBJECTS) -L. -lcrystalline -lalgorithms -lm
	@echo "✓ CLLM shared library created"

$(CLLM_STATIC): $(AI_OBJECTS) $(CRYSTALLINE_STATIC) $(ALGORITHMS_STATIC)
	@echo "Creating CLLM static library: $@"
	$(AR) $(ARFLAGS) $@ $(AI_OBJECTS)
	@echo "✓ CLLM static library created"

# Removed legacy monolithic libraries

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

install: all
	@echo "Installing Crystalline CLLM Libraries..."
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 libcrystalline.so $(DESTDIR)$(LIBDIR)
	install -m 755 libalgorithms.so $(DESTDIR)$(LIBDIR)
	install -m 755 libcllm.so $(DESTDIR)$(LIBDIR)
	install -m 755 libcrawler.so $(DESTDIR)$(LIBDIR)
	install -m 644 libcrystalline.a $(DESTDIR)$(LIBDIR)
	install -m 644 libalgorithms.a $(DESTDIR)$(LIBDIR)
	install -m 644 libcllm.a $(DESTDIR)$(LIBDIR)
	install -m 644 libcrawler.a $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)/crystalline
	cp -r include/* $(DESTDIR)$(INCLUDEDIR)/crystalline/
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
	@echo "Checking for flag:"
	@if echo "$(LDFLAGS)" | grep -q "\-lm"; then \
		echo "✗ Found flag in LDFLAGS!"; \
		exit 1; \
	else \
		echo "✓ No flag present"; \
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
	@echo "║ Mathematical Independence: YES (no, no math.h)            ║"
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
	rm -f $(CRYSTALLINE_LIB) $(ALGORITHMS_LIB) $(CLLM_LIB) $(CRAWLER_LIB) $(DOCPROC_LIB) $(STATIC_LIB) $(SHARED_LIB)
	rm -f tools/cllm_pdf_extract tools/cllm_ocr tools/cllm_pdf_ocr tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build \
                tools/init_lattice_embeddings tools/benchmark_ntt_attention tools/validate_kissing_spheres \
                tools/analyze_cymatic_resonance tools/visualize_angular_positions
	@if [ -d tests ]; then $(MAKE) -C tests clean 2>/dev/null || true; fi
	@if [ -d algorithms ]; then $(MAKE) -C algorithms clean 2>/dev/null || true; fi
	@if [ -d demos ]; then $(MAKE) -C demos clean 2>/dev/null || true; fi
	@if [ -d app ]; then $(MAKE) -C app clean 2>/dev/null || true; fi
	@echo "✓ Clean complete"

# ============================================================================
# Debug Build
# ============================================================================

debug: CFLAGS += -g -O2 -DDEBUG -fsanitize=address -fsanitize=undefined
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: clean all
	@echo "✓ Debug build complete"

# ============================================================================
# ============================================================================
# Crawler Library (libcrawler.so)
# ============================================================================

CRAWLER_SOURCES = src/crawler/crawler_core.c src/crawler/preprocessor.c \
                  src/crawler/tokenizer.c src/crawler/continuous_training.c \
                  src/crawler/crawler_api.c src/crawler/file_processor.c \
                  src/crawler/prime_randomization.c src/crawler/link_management.c \
                  src/crawler/url_patterns.c src/crawler/file_processor_pdf.c \
                  src/crawler/file_processor_image.c src/crawler/file_processor_office.c \
                  src/crawler/url_database.c src/crawler/url_filter.c \
                  src/crawler/url_priority.c src/crawler/url_blocker.c \
                  src/crawler/crawler_url_manager.c src/crawler/content_filter.c \
                  src/crawler/site_handlers.c src/crawler/handlers/handlers.c \
                  src/crawler/handlers/twitter_handler.c src/crawler/handlers/britannica_handler.c \
                  src/crawler/handlers/etymonline_handler.c src/crawler/handlers/wikipedia_handler.c \
                  src/crawler/handlers/reddit_handler.c src/crawler/handlers/stackoverflow_handler.c \
                  src/crawler/handlers/news_handler.c
CRAWLER_OBJECTS = $(CRAWLER_SOURCES:.c=.o)
CRAWLER_LIB = libcrawler.so

$(CRAWLER_LIB): $(CRAWLER_OBJECTS) $(CLLM_LIB)
	@echo "Creating crawler shared library: $@"
	$(CC) -shared -o $@ $(CRAWLER_OBJECTS) -L. -lcrystalline -lcllm -lcurl -lpthread -lsqlite3
	@echo "✓ Crawler shared library created"

$(CRAWLER_STATIC): $(CRAWLER_OBJECTS) $(CLLM_STATIC)
	@echo "Creating crawler static library: $@"
	$(AR) $(ARFLAGS) $@ $(CRAWLER_OBJECTS)
	@echo "✓ Crawler static library created"

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
	$(CC) -shared -o $@ $(DOCPROC_OBJECTS)
	@echo "✓ Document processing library created"

# ============================================================================
# Document Processing CLI Tools
# ============================================================================

tools: tools/cllm_pdf_extract tools/cllm_ocr tools/cllm_pdf_ocr tools/cllm \
       tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build \
       tools/cllm_model_manager \
       tools/init_lattice_embeddings tools/benchmark_ntt_attention \
       tools/validate_kissing_spheres tools/analyze_cymatic_resonance \
       tools/visualize_angular_positions

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
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Inference tool built: tools/cllm_inference"

tools/cllm_tokenize: $(CLLM_LIB)
	@echo "Building tokenizer tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_tokenize tools/cllm_tokenize.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Tokenizer tool built: tools/cllm_tokenize"

tools/cllm_vocab_build: $(CLLM_LIB)
	@echo "Building vocabulary builder tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_vocab_build tools/cllm_vocab_build.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'

tools/init_lattice_embeddings: $(CLLM_LIB)
	@echo "Building lattice embeddings initializer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/init_lattice_embeddings tools/init_lattice_embeddings.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Lattice embeddings tool built: tools/init_lattice_embeddings"

tools/benchmark_ntt_attention: $(CLLM_LIB)
	@echo "Building NTT attention benchmark..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/benchmark_ntt_attention tools/benchmark_ntt_attention.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ NTT benchmark tool built: tools/benchmark_ntt_attention"

tools/validate_kissing_spheres: $(CLLM_LIB)
	@echo "Building kissing spheres validator..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/validate_kissing_spheres tools/validate_kissing_spheres.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Kissing spheres validator built: tools/validate_kissing_spheres"

tools/analyze_cymatic_resonance: $(CLLM_LIB)
	@echo "Building cymatic resonance analyzer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/analyze_cymatic_resonance tools/analyze_cymatic_resonance.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Cymatic analyzer tool built: tools/analyze_cymatic_resonance"

tools/visualize_angular_positions: $(CLLM_LIB)
	@echo "Building angular position visualizer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/visualize_angular_positions tools/visualize_angular_positions.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Angular visualizer tool built: tools/visualize_angular_positions"

all_tools: tools tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build \
        tools/init_lattice_embeddings tools/benchmark_ntt_attention \
        tools/validate_kissing_spheres tools/analyze_cymatic_resonance \
        tools/visualize_angular_positions


# ============================================================================
# Production Build Targets
# ============================================================================

PROD_CFLAGS = -Wall -Wextra -O3 -std=c11 -DPRODUCTION_BUILD=1 -march=native -flto -I./include
PROD_TEST_DIR = tests

# Production test executables
benchmark_large_scale: $(PROD_TEST_DIR)/benchmark_large_scale.c $(CLLM_LIB)
	@echo "Building large-scale benchmark..."
	@mkdir -p tools
	$(CC) $(PROD_CFLAGS) -o tools/benchmark_large_scale $(PROD_TEST_DIR)/benchmark_large_scale.c \
		-L. -lcllm -lcrystalline -lalgorithms -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Large-scale benchmark built: tools/benchmark_large_scale"

validate_training_quality: $(PROD_TEST_DIR)/validate_training_quality.c $(CLLM_LIB)
	@echo "Building training quality validator..."
	@mkdir -p tools
	$(CC) $(PROD_CFLAGS) -o tools/validate_training_quality $(PROD_TEST_DIR)/validate_training_quality.c \
		-L. -lcllm -lcrystalline -lalgorithms -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Training validator built: tools/validate_training_quality"

smoke_test_production: $(PROD_TEST_DIR)/smoke_test_production.c $(CLLM_LIB)
	@echo "Building production smoke tests..."
	@mkdir -p tools
	$(CC) $(PROD_CFLAGS) -o tools/smoke_test_production $(PROD_TEST_DIR)/smoke_test_production.c \
		-L. -lcllm -lcrystalline -lalgorithms -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Smoke tests built: tools/smoke_test_production"

# Production build target
production: benchmark_large_scale validate_training_quality smoke_test_production
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║         PRODUCTION BUILD COMPLETED SUCCESSFULLY                ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Production tools built:"
	@echo "  ✓ tools/benchmark_large_scale"
	@echo "  ✓ tools/validate_training_quality"
	@echo "  ✓ tools/smoke_test_production"
	@echo ""

# Run production tests
prod-tests: production
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║              RUNNING PRODUCTION TEST SUITE                     ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Running smoke tests..."
	@./tools/smoke_test_production || exit 1
	@echo ""
	@echo "Running training quality validation..."
	@./tools/validate_training_quality || exit 1
	@echo ""
	@echo "Running large-scale benchmarks..."
	@timeout 300 ./tools/benchmark_large_scale || echo "Benchmark timed out or failed"
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║           PRODUCTION TESTS COMPLETED                           ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"


# Unified CLI tool (replaces individual tools)
tools/cllm: $(CLLM_LIB) $(CRYSTALLINE_LIB) $(ALGORITHMS_LIB)
	@echo "Building unified CLLM CLI tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm tools/cllm_unified.c \
		-L. -lcllm -lcrystalline -lalgorithms -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Unified CLI tool built: tools/cllm"

tools/cllm_model_manager: $(CLLM_LIB)
	@echo "Building model manager tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_model_manager tools/cllm_model_manager.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Model manager tool built: tools/cllm_model_manager"

tools/diagnose_inference: $(CLLM_LIB)
	@echo "Building inference diagnostic tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/diagnose_inference tools/diagnose_inference.c \
		-L. -L./algorithms -lcllm -lalgorithms -lcrystalline -lm -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Inference diagnostic tool built: tools/diagnose_inference"
