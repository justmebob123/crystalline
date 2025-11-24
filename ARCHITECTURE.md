# Crystalline Project Architecture

## Overview

The Crystalline project consists of three independent libraries and two consumer applications, all built with proper separation of concerns.

## Library Structure

### 1. libprimemath.a / libprimemath.so
**Purpose**: Core mathematical operations and AI functionality

**Components**:
- **Core**: Arbitrary precision arithmetic (BigInt, BigFixed)
- **Transcendental**: Advanced mathematical functions
- **Geometry**: Lattice algorithms, prime coordinates, hyperdimensional math
- **AI**: CLLM (Crystalline Lattice Language Model) implementation

**Dependencies**: None (fully self-contained)

### 2. libcrawler.so
**Purpose**: Web crawling, preprocessing, tokenization, and continuous training

**Components**:
- **crawler_core.c**: Web page downloading with rate limiting
- **preprocessor.c**: HTML cleaning and text extraction
- **tokenizer.c**: Text tokenization for training
- **continuous_training.c**: Continuous model training pipeline
- **crawler_api.c**: High-level API for consumers

**Dependencies**: libprimemath.a, libcurl, libpthread

**API Design**: See LIBRARY_API_DESIGN.md and ARCHITECTURAL_REDESIGN.md

### 3. libcllm.so (Future)
**Purpose**: Standalone CLLM library (currently part of libprimemath)

## Consumer Applications

### 1. cllm_crawler (CLI Tool)
**Location**: `tools/cllm_crawler`

**Purpose**: Command-line web crawler

**Implementation**:
- Pure consumer of libcrawler.so
- No crawler implementation code
- Parses command-line arguments
- Registers callbacks for events
- Prints status to stdout

**Usage**:
```bash
./tools/cllm_crawler --start-url https://example.com --max-pages 100
```

### 2. hyper_prime_spiral (GUI Application)
**Location**: `app/hyper_prime_spiral`

**Purpose**: Interactive visualization and training interface

**Implementation**:
- Pure consumer of libcrawler.so and libprimemath.a
- No crawler implementation code
- SDL2-based UI
- Direct library integration
- Real-time status updates via callbacks

**Features**:
- Prime spiral visualization
- CLLM training interface
- Web crawler control
- Research tools

## Key Architectural Principles

### 1. Library Independence
- Each library is self-contained
- Clear dependency hierarchy: libcrawler → libprimemath → (no deps)
- No circular dependencies

### 2. Consumer Independence
- CLI and GUI are completely independent
- No shared code between consumers
- Both use the same library API
- Can run on different platforms

### 3. Clean API Surface
- Well-documented public APIs
- Thread-safe operations
- Event-driven updates
- No implementation details exposed

### 4. Build System
```
Makefile (root)
├── libprimemath.a (static library)
├── libprimemath.so (shared library)
├── libcrawler.so (shared library)
├── tools/cllm_crawler (CLI tool)
└── app/Makefile
    └── hyper_prime_spiral (GUI app)
```

## Directory Structure

```
crystalline/
├── include/           # Public headers
│   ├── cllm.h        # CLLM API
│   ├── crawler.h     # Crawler API
│   └── prime_*.h     # Math APIs
├── src/
│   ├── core/         # Core math
│   ├── transcendental/
│   ├── geometry/
│   ├── ai/           # CLLM implementation
│   └── crawler/      # Crawler implementation
├── app/              # GUI application
│   ├── *.c          # App source files
│   └── ui/tabs/     # UI components
├── tools/            # CLI tools
│   └── cllm_crawler # Crawler CLI
├── demos/            # Example programs
└── docs/             # Documentation
```

## Build Instructions

### Build Everything
```bash
make all          # Build libraries
make crawler      # Build CLI tool
cd app && make    # Build GUI app
```

### Build Individual Components
```bash
make libprimemath.a   # Static math library
make libprimemath.so  # Shared math library
make libcrawler.so    # Crawler library
make crawler          # CLI tool only
```

### Run Applications
```bash
# CLI tool
LD_LIBRARY_PATH=. ./tools/cllm_crawler --start-url https://example.com

# GUI app
cd app
LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

## Design Documents

- **LIBRARY_API_DESIGN.md**: Detailed crawler API specification
- **ARCHITECTURAL_REDESIGN.md**: Migration from old to new architecture
- **LIBRARY_ANALYSIS.md**: Library structure analysis

## Future Enhancements

1. **Separate libcllm.so**: Extract CLLM from libprimemath
2. **Python Bindings**: Create Python wrappers for libraries
3. **Web Service**: HTTP API using the libraries
4. **Distributed Crawling**: Multi-instance coordination

## Testing

Each component can be tested independently:

```bash
# Test math library
cd demos && make && ./prime_demo

# Test CLLM
cd demos && ./cllm_demo

# Test crawler (CLI)
LD_LIBRARY_PATH=. ./tools/cllm_crawler --start-url https://example.com --max-pages 5

# Test GUI
cd app && LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

## Dependencies

### System Requirements
- GCC with C11 support
- libcurl (for web crawling)
- libpthread (for threading)
- SDL2 and SDL2_ttf (for GUI only)

### Library Dependencies
```
libprimemath.a: (no dependencies)
libcrawler.so:  libprimemath.a, libcurl, libpthread
cllm_crawler:   libcrawler.so
hyper_prime_spiral: libprimemath.a, libcrawler.so, SDL2, SDL2_ttf
```

## Conclusion

This architecture provides:
- **Modularity**: Independent, reusable libraries
- **Maintainability**: Single source of truth for each component
- **Flexibility**: Easy to create new consumers
- **Testability**: Each component can be tested independently
- **Portability**: Libraries can be used on any platform