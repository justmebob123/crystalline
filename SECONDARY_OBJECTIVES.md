# SECONDARY OBJECTIVES - Crystalline Enhancement Plan

## Overview
This document outlines comprehensive enhancements to the Crystalline system, focusing on:
1. **CRITICAL: Fix Rainbow Table to Use Clock Lattice Mapping** (NEW - HIGH PRIORITY)
2. Complete UI enhancements (all pending features)
3. Advanced crawler controls with prime-based randomization
4. Pure C implementation of all file processors (no Python dependencies)
5. Proper library organization and categorization
6. Advanced URL pattern detection
7. Dependency management for multiple Linux distributions

---

## 🔴 CRITICAL PRIORITY: FIX RAINBOW TABLE COORDINATE SYSTEM

### Problem Identified
The `prime_rainbow.c` file is using **SPIRAL COORDINATE FORMULAS** instead of the **CLOCK LATTICE MAPPING** implemented in `clock_lattice.c`. This causes the X-pattern visualization instead of the correct concentric ring structure.

### Current INCORRECT Implementation (prime_rainbow.c)

```c
// ❌ WRONG - Uses spiral formula
double fast_prime_angle(int prime) {
    return (2.0 * PRIME_PI * prime) / (prime + 1);  // Spiral angle
}

// ❌ WRONG - Radius grows outward with sqrt(prime)
double fast_prime_radius(int prime) {
    return prime_sqrt((double)prime);  // Spiral radius
}

// ❌ WRONG - No relationship to Babylonian structure
int fast_prime_layer(int prime) {
    return (prime % RAINBOW_LAYERS) + 1;  // Arbitrary layering
}

// ❌ WRONG - Uses spiral coordinates
void fast_prime_fold_coords(int prime, double* x, double* y) {
    double angle = fast_prime_angle(prime);
    double radius = fast_prime_radius(prime);
    *x = radius * prime_cos(angle);
    *y = radius * prime_sin(angle);
}
```

### Correct Implementation (Should Use clock_lattice.c)

```c
// ✅ CORRECT - Use clock lattice mapping
double fast_prime_angle(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.angle;
}

// ✅ CORRECT - Use clock lattice radius (INWARD counting)
double fast_prime_radius(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.radius;  // 0.25 (outer) to 1.0 (inner)
}

// ✅ CORRECT - Use clock lattice ring
int fast_prime_layer(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.ring;  // 0-3 for first 232 primes
}

// ✅ CORRECT - Use clock lattice then fold to sphere
void fast_prime_fold_coords(int prime_index, double* x, double* y, double* z) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    SphereCoord sphere = fold_clock_to_sphere(pos);
    *x = sphere.x;
    *y = sphere.y;
    *z = sphere.z;
}
```

### Mathematical Foundation

**Babylonian Clock Structure (from MASTER_PLAN OBJECTIVE 19):**
```
Ring 0 (OUTSIDE): ∞/0 boundary
  ↓ (counting INWARD toward unity)
Ring 1: Primes 1-12 (HOURS) at 25% radius - OUTER (smaller primes)
  ↓
Ring 2: Primes 13-72 (MINUTES) at 50% radius
  ↓
Ring 3: Primes 73-132 (SECONDS) at 75% radius
  ↓
Ring 4: Primes 133-232 (MILLISECONDS) at 100% radius - INNER (larger primes)
  ↓
Center: 1 (Unity)
```

**Key Principles:**
1. **Smaller primes on OUTER rings** (counting inward from ∞/0)
2. **Larger primes on INNER rings** (approaching unity at center)
3. **Clock positions, NOT spiral** (12, 60, 60, 100 positions)
4. **Modular arithmetic** (mod 12, mod 60, mod 100)
5. **12-fold symmetry** (primes > 3 in {1, 5, 7, 11} mod 12)
6. **Stereographic projection** to fold clock → sphere

### Implementation Steps

#### Step 1: Update Function Signatures
Change from `int prime` to `int prime_index` to clarify we're using prime INDEX not prime VALUE:

```c
// OLD signatures (confusing):
double fast_prime_angle(int prime);
double fast_prime_radius(int prime);

// NEW signatures (clear):
double fast_prime_angle(int prime_index);
double fast_prime_radius(int prime_index);
```

#### Step 2: Include clock_lattice.h
```c
#include "clock_lattice.h"
```

#### Step 3: Rewrite Each Function

**fast_prime_angle():**
```c
double fast_prime_angle(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.angle;
}
```

**fast_prime_radius():**
```c
double fast_prime_radius(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.radius;
}
```

**fast_prime_layer():**
```c
int fast_prime_layer(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.ring;
}
```

**fast_prime_fold_coords():**
```c
void fast_prime_fold_coords(int prime_index, double* x, double* y, double* z) {
    if (!x || !y || !z) return;
    
    // Get clock position
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    
    // Fold to 3D sphere
    SphereCoord sphere = fold_clock_to_sphere(pos);
    
    *x = sphere.x;
    *y = sphere.y;
    *z = sphere.z;
}
```

**fast_prime_position():**
```c
int fast_prime_position(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return pos.position;
}
```

#### Step 4: Update BigInt Versions

```c
double big_fast_prime_angle(BigInt *prime) {
    if (!prime || prime->len == 0) return 0.0;
    
    // Convert BigInt to prime index (this needs proper implementation)
    // For now, use first digit as approximation
    uint64_t prime_index = prime->d[0];
    return fast_prime_angle((int)prime_index);
}

// Similar for other big_fast_prime_* functions
```

#### Step 5: Add Modular Arithmetic Functions

```c
// Get modular relationships for a prime
PrimeModular fast_prime_modular(uint64_t prime) {
    return get_prime_modular(prime);  // Use clock_lattice function
}

// Check if position is sacred (π, 12 o'clock, etc.)
bool fast_prime_is_sacred(int prime_index) {
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    return is_sacred_position(pos);  // Use clock_lattice function
}
```

#### Step 6: Update Frequency Function

```c
double fast_prime_frequency(int prime_index) {
    // Base frequency on clock position, not prime value
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    
    // Frequency based on ring (deeper rings = higher frequency)
    double base_freq = 432.0;  // A4 tuning
    double ring_multiplier = 1.0 + (pos.ring * 0.5);  // Ring 0=1.0, Ring 3=2.5
    
    // Modulate by position within ring
    double position_factor = (double)pos.position / 12.0;  // Normalized
    
    return base_freq * ring_multiplier * (1.0 + position_factor * PHI);
}
```

### Testing Requirements

After implementation, verify:

1. **Visual Test**: Visualization shows concentric rings, NOT X pattern
2. **Ring Test**: Primes 1-12 on outer ring (25% radius)
3. **Ring Test**: Primes 133-232 on inner ring (100% radius)
4. **Angle Test**: 3 o'clock position = 0° (π position)
5. **Angle Test**: 12 o'clock position = -90° (top)
6. **Symmetry Test**: All primes > 3 in {1, 5, 7, 11} mod 12
7. **Modular Test**: mod_12, mod_60, mod_100 relationships correct
8. **Sphere Test**: Stereographic projection produces unit sphere

### Files to Modify

1. **src/geometry/prime_rainbow.c** - Rewrite coordinate functions
2. **include/prime_rainbow.h** - Update function signatures
3. **app/visualization.c** (if exists) - Update visualization to use new coords
4. **tests/** - Add tests for clock lattice integration

### Dependencies

- `clock_lattice.h` - Already implemented correctly
- `clock_lattice.c` - Already implemented correctly
- No new dependencies needed

### Expected Results

After fix:
- ✅ Concentric ring visualization (not X pattern)
- ✅ Smaller primes on outer rings
- ✅ Larger primes on inner rings
- ✅ Clock positions (12, 60, 60, 100)
- ✅ Proper stereographic projection
- ✅ 12-fold symmetry maintained
- ✅ Modular arithmetic relationships preserved

### Priority: HIGH
This must be fixed before the rainbow table can be used for proper prime visualization and semantic embedding.

---

---

## RULES & STRUCTURE

### Core Principles
1. **Pure C Implementation**: All code must be written in C, no Python scripts
2. **Library Organization**: Proper categorization and modular design
3. **External Dependencies**: Document all dependencies with installation scripts
4. **Custom Implementations**: Provide C implementations where libraries don't exist
5. **UI Integration**: All features must have proper UI controls
6. **Testing**: Each component must be tested before integration
7. **Documentation**: Comprehensive docs for all new features

### Implementation Order
1. Library reorganization (foundation)
2. File processor C implementations (core functionality)
3. Advanced crawler features (prime randomization, URL patterns)
4. UI enhancements (all pending features)
5. Testing and integration
6. Documentation and dependency management

---

## PHASE 1: LIBRARY REORGANIZATION (4 hours)

### 1.1 Create Unified Document Processing Library (2 hours)
**Goal**: Consolidate all file processing into `libdocproc`

#### Structure
```
src/docproc/
├── core/
│   ├── format_detection.c      # File type detection
│   ├── text_extraction.c       # Base extraction functions
│   └── text_cleaning.c         # Text normalization
├── formats/
│   ├── pdf_processor.c         # PDF extraction (poppler)
│   ├── office_processor.c      # DOC/DOCX/RTF (custom + antiword)
│   ├── spreadsheet_processor.c # XLS/XLSX (custom implementation)
│   ├── presentation_processor.c # PPT/PPTX (custom implementation)
│   ├── odf_processor.c         # ODT/ODS/ODP (XML parsing)
│   ├── ebook_processor.c       # EPUB (ZIP + HTML)
│   ├── markup_processor.c      # MD/HTML/XML
│   ├── data_processor.c        # JSON/CSV/YAML/TOML
│   ├── code_processor.c        # Source code files
│   ├── email_processor.c       # EML/MSG
│   ├── image_processor.c       # OCR (tesseract)
│   └── archive_processor.c     # ZIP/TAR/etc
├── utils/
│   ├── xml_parser.c            # Custom XML parser
│   ├── zip_handler.c           # ZIP file handling
│   └── encoding_converter.c    # Character encoding
└── docproc.h                   # Public API
```

#### Tasks
- [ ] Create directory structure
- [ ] Design unified API
- [ ] Implement format detection in pure C
- [ ] Create base extraction framework
- [ ] Add error handling and logging

### 1.2 Implement Custom File Format Processors (2 hours)
**Goal**: Pure C implementations for all formats

#### Priority Formats (Custom C Implementation Required)
- [ ] **DOCX/XLSX/PPTX**: ZIP + XML parsing (no python-docx)
- [ ] **ODF (ODT/ODS/ODP)**: ZIP + XML parsing (no odt2txt)
- [ ] **EPUB**: ZIP + HTML parsing (no ebooklib)
- [ ] **YAML**: Custom YAML parser in C
- [ ] **TOML**: Custom TOML parser in C

#### Formats Using Existing C Libraries
- [ ] **PDF**: pdftotext (poppler-utils)
- [ ] **DOC**: antiword
- [ ] **RTF**: unrtf
- [ ] **Images**: tesseract
- [ ] **JSON**: jq or custom parser
- [ ] **CSV**: Custom parser (simple)

---

## PHASE 2: PURE C FILE PROCESSORS (8 hours)

### 2.1 Office Open XML (DOCX/XLSX/PPTX) Processor (3 hours)
**Implementation**: ZIP extraction + XML parsing

#### Components
- [ ] ZIP file reader (libzip or custom)
- [ ] XML parser (libxml2 or custom)
- [ ] DOCX text extraction
  - Parse document.xml
  - Extract paragraphs, tables, headers
  - Handle styles and formatting
- [ ] XLSX data extraction
  - Parse worksheets
  - Extract cell values
  - Handle formulas
- [ ] PPTX slide extraction
  - Parse slides
  - Extract text from shapes
  - Handle notes and comments

#### Files to Create
```c
src/docproc/formats/ooxml_processor.c
src/docproc/formats/ooxml_parser.c
src/docproc/utils/zip_handler.c
```

### 2.2 OpenDocument Format (ODT/ODS/ODP) Processor (2 hours)
**Implementation**: ZIP extraction + XML parsing

#### Components
- [ ] ODF structure parser
- [ ] ODT text extraction (content.xml)
- [ ] ODS spreadsheet extraction
- [ ] ODP presentation extraction

#### Files to Create
```c
src/docproc/formats/odf_processor.c
```

### 2.3 EPUB Processor (1 hour)
**Implementation**: ZIP extraction + HTML parsing

#### Components
- [ ] EPUB structure parser (container.xml, content.opf)
- [ ] Chapter extraction
- [ ] HTML content parsing
- [ ] Metadata extraction

#### Files to Create
```c
src/docproc/formats/epub_processor.c
```

### 2.4 YAML/TOML Parsers (2 hours)
**Implementation**: Custom parsers in C

#### YAML Parser
- [ ] Tokenizer
- [ ] Parser for basic YAML structures
- [ ] Value extraction
- [ ] Text conversion

#### TOML Parser
- [ ] INI-style parser
- [ ] Section handling
- [ ] Value extraction

#### Files to Create
```c
src/docproc/formats/yaml_parser.c
src/docproc/formats/toml_parser.c
```

---

## PHASE 3: ADVANCED CRAWLER FEATURES (6 hours)

### 3.1 Prime-Based Randomization System (2 hours)

#### Features
- [ ] Prime number selection for crawl frequency
- [ ] Prime-based delay calculation
- [ ] Prime-based link selection from queue
- [ ] Configurable prime ranges

#### Implementation
```c
typedef struct {
    uint64_t frequency_prime;     // Prime for crawl frequency
    uint64_t link_selection_prime; // Prime for link randomization
    uint64_t delay_min_prime;      // Min delay prime
    uint64_t delay_max_prime;      // Max delay prime
    bool use_prime_randomization;
} CrawlerPrimeConfig;

// Functions
void crawler_set_prime_config(CrawlerState* state, CrawlerPrimeConfig* config);
uint64_t calculate_prime_delay(uint64_t min_prime, uint64_t max_prime);
int select_prime_link(CrawlerState* state, char* url, size_t url_size);
```

#### Files to Create
```c
src/crawler/prime_randomization.c
src/crawler/prime_randomization.h
```

### 3.2 Advanced URL Pattern Detection (2 hours)

#### Patterns to Detect
- [ ] JavaScript onclick handlers
- [ ] Data attributes (data-href, data-url)
- [ ] Meta refresh redirects
- [ ] JavaScript window.location
- [ ] AJAX endpoints
- [ ] API endpoints in scripts
- [ ] Sitemap.xml references
- [ ] RSS/Atom feed links
- [ ] Canonical URLs
- [ ] Open Graph URLs

#### Implementation
```c
typedef enum {
    URL_PATTERN_HREF,
    URL_PATTERN_ONCLICK,
    URL_PATTERN_DATA_ATTR,
    URL_PATTERN_META_REFRESH,
    URL_PATTERN_JS_LOCATION,
    URL_PATTERN_AJAX,
    URL_PATTERN_API,
    URL_PATTERN_SITEMAP,
    URL_PATTERN_FEED,
    URL_PATTERN_CANONICAL,
    URL_PATTERN_OPENGRAPH
} URLPatternType;

int extract_urls_advanced(const char* html, const char* base_url, 
                         URLPatternType* patterns, int num_patterns,
                         const char* output_file);
```

#### Files to Create
```c
src/crawler/url_patterns.c
src/crawler/url_patterns.h
```

### 3.3 Dynamic Link Management (2 hours)

#### Features
- [ ] Add links dynamically during crawl
- [ ] Link queue management
- [ ] Duplicate detection
- [ ] Priority queue support
- [ ] Link filtering rules

#### Implementation
```c
typedef struct {
    char url[2048];
    int priority;
    time_t added_time;
    char source_url[2048];
} CrawlerLink;

int crawler_add_link(CrawlerState* state, const char* url, int priority);
int crawler_add_links_batch(CrawlerState* state, CrawlerLink* links, int count);
int crawler_get_queue_size(CrawlerState* state);
int crawler_clear_queue(CrawlerState* state);
```

---

## PHASE 4: COMPREHENSIVE UI ARCHITECTURE REDESIGN (17 hours)

**CRITICAL**: The current UI architecture has fundamental design flaws that must be fixed before continuing with individual tab enhancements.

### 4.0 UI Architecture Overhaul (9 hours) ⚠️ PRIORITY

#### Problem Statement
The current UI assumes all tabs use a split layout (visualization + control panel), which doesn't work for tabs like Crawler, LLM, Training, and Research. This causes:
- Narrow control panels (320px) inadequate for complex interfaces
- Inconsistent layouts across tabs
- Poor use of screen space
- Difficult to maintain and extend

#### Solution: Flexible Layout System

**4.0.1 Create Layout Manager (1 hour)**
- [ ] Create `app/ui/layout_manager.h` and `app/ui/layout_manager.c`
- [ ] Define layout types: LAYOUT_SPLIT, LAYOUT_FULL_WIDTH, LAYOUT_CUSTOM
- [ ] Implement `get_tab_layout()` function
- [ ] Support multi-column layouts (1, 2, 3+ columns)
- [ ] Calculate column widths dynamically

**4.0.2 Update Main Render Loop (1 hour)**
- [ ] Modify `app/main.c` render function
- [ ] Get layout for current tab before drawing
- [ ] Pass layout to tab draw functions
- [ ] Remove hardcoded control panel logic

**4.0.3 Redesign Crawler Tab with 3-Column Layout (2 hours)**
- [ ] Column 1: Prime Configuration + URL Patterns
- [ ] Column 2: Link Management + Activity Log
- [ ] Column 3: Status Display + Controls
- [ ] Use full window width (WINDOW_WIDTH - 40)
- [ ] Proper spacing and visual hierarchy

**4.0.4 Update LLM Tab with 2-Column Layout (2 hours)**
- [ ] Column 1: Conversation area (70% width)
- [ ] Column 2: Model controls and parameters (30% width)
- [ ] Better use of screen space
- [ ] Improved readability

**4.0.5 Update Training Tab with 2-Column Layout (2 hours)**
- [ ] Column 1: Sphere visualization (60% width)
- [ ] Column 2: Metrics and controls (40% width)
- [ ] Larger visualization area
- [ ] Better metrics display

**4.0.6 Update Research Tab with 2-Column Layout (1 hour)**
- [ ] Column 1: File list (40% width)
- [ ] Column 2: File content viewer (60% width)
- [ ] Better content viewing experience

### 4.1 Crawler Tab Feature Completion (After 4.0) (1 hour)

**Note**: Basic 3-column layout completed in 4.0.3, this adds remaining features

#### Additional Features
- [ ] **Prime Configuration Panel**
  - Frequency prime input (uint64_t)
  - Link selection prime input
  - Delay min/max prime inputs
  - Enable/disable prime randomization toggle
  - Prime validation indicator

- [ ] **Link Management Panel**
  - Add link input field
  - Add button
  - Link list display (scrollable)
  - Remove link button
  - Clear all button
  - Queue size indicator

- [ ] **URL Pattern Selection**
  - Checkboxes for each pattern type
  - Enable/disable all button
  - Pattern statistics

- [ ] **Status Display**
  - Current crawl frequency
  - Next crawl time (prime-based)
  - Links in queue
  - Links crawled
  - Current URL being processed

#### Implementation
```c
// In app/ui/tabs/tab_crawler.c (new file)
void draw_crawler_tab(AppState* state);
void handle_crawler_tab_click(AppState* state, int x, int y);
void update_crawler_status(AppState* state);
```

### 4.2 Training Tab Advanced Features (After 4.0) (2 hours)

#### Features from Original Plan
- [ ] Recursive sub-spheres visualization
- [ ] Zoom and pan controls
- [ ] Mouse-over tooltips
- [ ] 3D visualization mode
- [ ] 2D/3D toggle

#### Implementation
```c
// Enhanced sphere visualization
typedef struct {
    int zoom_level;
    float pan_x, pan_y;
    bool show_3d;
    bool show_subspheres;
    int max_depth;
} SphereVisualizationConfig;
```

### 4.3 Collapsible Panels (After 4.0) (1 hour)

#### Complete Implementation
- [ ] Integrate collapsible headers into all tabs
- [ ] Add click detection
- [ ] Implement smooth animations
- [ ] Save panel states
- [ ] Keyboard shortcuts (Ctrl+number)

### 4.4 LLM Tab Remaining Features (After 4.0) (1 hour)

#### Optional Enhancements
- [ ] Repetition penalty slider
- [ ] Frequency penalty slider
- [ ] Presence penalty slider
- [ ] Stop sequences input
- [ ] Random seed input
- [ ] Cancel generation button
- [ ] Keyboard shortcuts (Ctrl+Enter to send)
- [ ] Context menu (right-click options)
- [ ] Drag and drop file upload

---

## PHASE 5: DEPENDENCY MANAGEMENT (2 hours)

### 5.1 Create Installation Scripts (1 hour)

#### Ubuntu/Debian Script
```bash
#!/bin/bash
# install_deps_ubuntu.sh

# System packages
apt-get update
apt-get install -y \
    build-essential \
    libcurl4-openssl-dev \
    libxml2-dev \
    libzip-dev \
    zlib1g-dev \
    poppler-utils \
    antiword \
    unrtf \
    tesseract-ocr \
    jq

# Optional: Build from source if needed
# ...
```

#### CentOS/RHEL Script
```bash
#!/bin/bash
# install_deps_centos.sh

# System packages
yum groupinstall -y "Development Tools"
yum install -y \
    libcurl-devel \
    libxml2-devel \
    libzip-devel \
    zlib-devel \
    poppler-utils \
    antiword \
    tesseract \
    jq

# Additional packages from EPEL
yum install -y epel-release
yum install -y unrtf
```

### 5.2 Dependency Documentation (1 hour)

#### Create DEPENDENCIES.md
- [ ] List all required libraries
- [ ] Version requirements
- [ ] Installation instructions per distro
- [ ] Build from source instructions
- [ ] Troubleshooting guide

---

## PHASE 6: TESTING & INTEGRATION (4 hours)

### 6.1 Unit Tests (2 hours)
- [ ] Test each file processor
- [ ] Test prime randomization
- [ ] Test URL pattern detection
- [ ] Test link management
- [ ] Test UI components

### 6.2 Integration Tests (2 hours)
- [ ] Test full crawler pipeline
- [ ] Test UI interactions
- [ ] Test with real-world data
- [ ] Performance benchmarking
- [ ] Memory leak detection

---

## PHASE 7: DOCUMENTATION (2 hours)

### 7.1 User Documentation
- [ ] Crawler configuration guide
- [ ] Prime randomization explanation
- [ ] URL pattern guide
- [ ] File format support matrix
- [ ] UI usage guide

### 7.2 Developer Documentation
- [ ] Library API reference
- [ ] Architecture diagrams
- [ ] Extension guide
- [ ] Contributing guidelines

---

## IMPLEMENTATION TIMELINE

### Total Estimated Time: ~34 hours

**Week 1** (16 hours)
- Phase 1: Library Reorganization (4h)
- Phase 2: Pure C File Processors (8h)
- Phase 3: Advanced Crawler Features (4h)

**Week 2** (18 hours)
- Phase 3: Advanced Crawler Features (2h remaining)
- Phase 4: Complete UI Enhancements (8h)
- Phase 5: Dependency Management (2h)
- Phase 6: Testing & Integration (4h)
- Phase 7: Documentation (2h)

---

## PRIORITY ORDER

### Critical Path (Must Complete First)
1. Library reorganization
2. Pure C file processors (DOCX/XLSX/PPTX)
3. Crawler prime randomization
4. Crawler UI enhancements

### Secondary Priority
5. Advanced URL patterns
6. Training tab enhancements
7. LLM tab remaining features
8. Collapsible panels

### Final Polish
9. Testing and integration
10. Documentation
11. Dependency scripts

---

## SUCCESS CRITERIA

### Phase Completion Checklist
- [ ] All Python code removed and replaced with C
- [ ] libdocproc library created and functional
- [ ] All 25+ file formats supported in pure C
- [ ] Prime-based randomization working
- [ ] Advanced URL pattern detection implemented
- [ ] Dynamic link management functional
- [ ] All UI enhancements complete
- [ ] Installation scripts for Ubuntu and CentOS
- [ ] Comprehensive documentation
- [ ] All tests passing
- [ ] No memory leaks
- [ ] Build successful on both Ubuntu and CentOS

---

## CURRENT STATUS

**Phase**: Planning Complete
**Next Step**: Begin Phase 1 - Library Reorganization
**Estimated Start**: Awaiting user confirmation

---

## NOTES

### Design Decisions
1. **Pure C**: No Python dependencies for production use
2. **Modular**: Each processor is independent
3. **Extensible**: Easy to add new formats
4. **Performant**: C implementations are faster than Python
5. **Portable**: Works on Ubuntu and CentOS

### External Dependencies (Minimal)
- libxml2 (XML parsing)
- libzip (ZIP handling)
- libcurl (HTTP - already used)
- poppler-utils (PDF - CLI tool)
- tesseract (OCR - CLI tool)

### Custom Implementations (No External Deps)
- YAML parser
- TOML parser
- CSV parser
- JSON parser (optional, can use jq)
- Office XML parsers
- ODF parsers
- EPUB parser

---

## QUESTIONS FOR USER

Before proceeding, please confirm:
1. ✅ Remove all Python code and implement in pure C?
2. ✅ Create unified libdocproc library?
3. ✅ Implement prime-based randomization?
4. ✅ Add all UI enhancements?
5. ✅ Support both Ubuntu and CentOS?
6. ✅ Custom C implementations where needed?

**Ready to begin implementation upon your approval!**