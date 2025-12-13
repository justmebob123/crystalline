# Crystalline - Prime-Based Lattice Language Model

A revolutionary language model architecture based on prime number lattices and crystalline structures, featuring autonomous web crawling and continuous learning capabilities.

## Project Structure

This project consists of three independent libraries and two consumer applications:

### Libraries

1. **libprimemath** - Core mathematical operations
   - Arbitrary precision arithmetic (BigInt, BigFixed)
   - Transcendental functions
   - Lattice algorithms and prime coordinates
   - CLLM (Crystalline Lattice Language Model) implementation

2. **libcrawler** - Web crawling and continuous training
   - Autonomous web crawling with rate limiting
   - HTML preprocessing and text extraction
   - Tokenization pipeline
   - Continuous model training

### Applications

1. **cllm_crawler** - Command-line crawler tool
   - Pure consumer of libcrawler.so
   - Autonomous web crawling
   - Real-time status updates

2. **hyper_prime_spiral** - Interactive GUI application
   - Pure consumer of libcrawler.so and libprimemath
   - Prime spiral visualization
   - CLLM training interface
   - Research tools

## Architecture

The project follows proper library architecture with complete independence between components:

```
┌─────────────────────────────────────────────────────────────┐
│                    libcrawler.so                            │
│  (Shared library - provides all crawler functionality)      │
└─────────────────────────────────────────────────────────────┘
                    ▲                        ▲
                    │                        │
        ┌───────────┴──────────┐   ┌────────┴──────────┐
        │   cllm_crawler       │   │  hyper_prime_spiral│
        │   (CLI tool)         │   │  (GUI app)         │
        │  100% independent    │   │  100% independent  │
        └──────────────────────┘   └────────────────────┘
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed architecture documentation.

## Building

### Prerequisites

- GCC with C11 support
- libcurl (for web crawling)
- libpthread (for threading)
- SDL2 and SDL2_ttf (for GUI only)

### Build Commands

```bash
# Build all libraries
make all

# Build crawler library
make libcrawler.so

# Build CLI tool
make crawler

# Build GUI application
cd app && make
```

### Clean Build

```bash
make clean
make all
make crawler
cd app && make
```

## Running

### CLI Crawler

```bash
# Basic usage
LD_LIBRARY_PATH=. ./tools/cllm_crawler --start-url https://example.com

# With page limit
LD_LIBRARY_PATH=. ./tools/cllm_crawler --start-url https://example.com --max-pages 100

# Custom data directory
LD_LIBRARY_PATH=. ./tools/cllm_crawler --start-url https://example.com --data-dir ./my_data
```

### GUI Application

```bash
cd app
LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

## Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Complete project architecture
- **[ARCHITECTURAL_REDESIGN.md](ARCHITECTURAL_REDESIGN.md)** - Design rationale and migration
- **[LIBRARY_API_DESIGN.md](LIBRARY_API_DESIGN.md)** - Detailed API specification
- **[docs/mathematical_framework/](docs/mathematical_framework/)** - Mathematical foundations
- **[docs/session_history/](docs/session_history/)** - Development session notes

## Key Features

### Proper Library Architecture
- ✅ Independent libraries with clean APIs
- ✅ CLI and GUI are pure consumers
- ✅ No coupling between components
- ✅ Thread-safe operations
- ✅ Event-driven updates

### Web Crawler
- ✅ Autonomous crawling with rate limiting
- ✅ robots.txt compliance
- ✅ Domain filtering
- ✅ Continuous preprocessing and tokenization
- ✅ Real-time training pipeline

### CLLM (Crystalline Lattice Language Model)
- ✅ Prime-based lattice embeddings
- ✅ Crystalline attention mechanisms
- ✅ Continuous learning from web data
- ✅ Arbitrary precision arithmetic

### GUI Features
- ✅ Prime spiral visualization
- ✅ Interactive training interface
- ✅ Real-time crawler monitoring
- ✅ Research and analysis tools

## Development

### Adding New Features

The modular architecture makes it easy to extend:

1. **New Library Consumers**: Create new applications that use libcrawler.so or libprimemath
2. **Python Bindings**: Wrap the C libraries for Python use
3. **Web Service**: Create HTTP API using the libraries
4. **Distributed Crawling**: Coordinate multiple crawler instances

### Testing

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

## License

See [LICENSE](LICENSE) file for details.

## Contributing

This project follows strict architectural principles:

1. **Library Independence**: Each library must be self-contained
2. **Consumer Independence**: CLI and GUI must have no shared code
3. **Clean APIs**: All public APIs must be well-documented
4. **Thread Safety**: All library functions must be thread-safe
5. **No Coupling**: Components must not depend on implementation details

## Contact

For questions or contributions, please open an issue on GitHub.