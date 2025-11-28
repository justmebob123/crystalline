# TODO: Secondary Objectives - Comprehensive Enhancement Plan

## Overview
This plan addresses extensive improvements beyond the master plan:
1. Pure C implementation (remove Python dependencies)
2. Unified document processing library (libdocproc)
3. Advanced crawler features (prime randomization, URL patterns)
4. Complete UI enhancements (all pending features)
5. Multi-distro support (Ubuntu + CentOS)

**Reference**: See `SECONDARY_OBJECTIVES.md` for complete details

---

## PHASE 1: LIBRARY REORGANIZATION (4 hours) ✅ COMPLETE

### 1.1 Create Unified Document Processing Library (2 hours) ✅
- [x] Create src/docproc/ directory structure
- [x] Design unified API (docproc.h)
- [x] Implement format detection in pure C
- [x] Create base extraction framework
- [x] Add error handling and logging

**Directory Structure**:
```
src/docproc/
├── core/           # Core functionality
│   ├── format_detection.c
│   └── docproc_core.c
├── formats/        # Format-specific processors
│   ├── simple_formats.c
│   ├── cli_extractors.c
│   ├── office_xml.c (stubs)
│   ├── odf.c (stubs)
│   ├── epub.c (stub)
│   ├── yaml.c (stub)
│   ├── html.c
│   ├── email.c (stub)
│   └── archive.c
├── utils/          # Utilities (to be added)
├── docproc.h       # Public API
├── Makefile
├── libdocproc.so   # Shared library
└── libdocproc.a    # Static library
```

### 1.2 Plan Custom File Format Processors (2 hours) ✅
- [x] Design DOCX/XLSX/PPTX processor (ZIP + XML)
- [x] Design ODF processor (ODT/ODS/ODP)
- [x] Design EPUB processor (ZIP + HTML)
- [x] Design YAML/TOML parsers
- [x] Document external dependencies needed
- [x] Create stub implementations
- [x] Build system working

---

## PHASE 2: PURE C FILE PROCESSORS (8 hours) 🔧

### 2.1 Office Open XML Processor (3 hours)
**Goal**: Replace Python implementations with pure C

- [ ] Implement ZIP file reader (using libzip)
- [ ] Implement XML parser (using libxml2 or custom)
- [ ] DOCX text extraction
  - [ ] Parse document.xml
  - [ ] Extract paragraphs and tables
  - [ ] Handle styles
- [ ] XLSX data extraction
  - [ ] Parse worksheets
  - [ ] Extract cell values
  - [ ] Handle formulas
- [ ] PPTX slide extraction
  - [ ] Parse slides
  - [ ] Extract text from shapes

**Files**: `src/docproc/formats/ooxml_processor.c`

### 2.2 OpenDocument Format Processor (2 hours)
- [ ] ODF structure parser
- [ ] ODT text extraction (content.xml)
- [ ] ODS spreadsheet extraction
- [ ] ODP presentation extraction

**Files**: `src/docproc/formats/odf_processor.c`

### 2.3 EPUB Processor (1 hour)
- [ ] EPUB structure parser
- [ ] Chapter extraction
- [ ] HTML content parsing
- [ ] Metadata extraction

**Files**: `src/docproc/formats/epub_processor.c`

### 2.4 YAML/TOML Parsers (2 hours)
- [ ] YAML tokenizer and parser
- [ ] TOML parser (INI-style)
- [ ] Value extraction
- [ ] Text conversion

**Files**: `src/docproc/formats/yaml_parser.c`, `toml_parser.c`

---

## PHASE 3: ADVANCED CRAWLER FEATURES (6 hours) 🕷️

### 3.1 Prime-Based Randomization System (2 hours)

**Features**:
- [ ] Prime number selection for crawl frequency
- [ ] Prime-based delay calculation
- [ ] Prime-based link selection from queue
- [ ] Configurable prime ranges

**Data Structure**:
```c
typedef struct {
    uint64_t frequency_prime;
    uint64_t link_selection_prime;
    uint64_t delay_min_prime;
    uint64_t delay_max_prime;
    bool use_prime_randomization;
} CrawlerPrimeConfig;
```

**Files**: `src/crawler/prime_randomization.c`

### 3.2 Advanced URL Pattern Detection (2 hours)

**Patterns to Detect**:
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

**Files**: `src/crawler/url_patterns.c`

### 3.3 Dynamic Link Management (2 hours)

**Features**:
- [ ] Add links dynamically during crawl
- [ ] Link queue management
- [ ] Duplicate detection
- [ ] Priority queue support
- [ ] Link filtering rules

**API**:
```c
int crawler_add_link(CrawlerState* state, const char* url, int priority);
int crawler_add_links_batch(CrawlerState* state, CrawlerLink* links, int count);
int crawler_get_queue_size(CrawlerState* state);
```

---

## PHASE 4: COMPLETE UI ENHANCEMENTS (8 hours) 🎨

### 4.1 Crawler Tab Enhancement (3 hours)

**New UI Elements**:
- [ ] Prime Configuration Panel
  - [ ] Frequency prime input
  - [ ] Link selection prime input
  - [ ] Delay min/max prime inputs
  - [ ] Enable/disable toggle
  - [ ] Prime validation indicator

- [ ] Link Management Panel
  - [ ] Add link input field
  - [ ] Add button (clears input after adding)
  - [ ] Pop-up confirmation ("Link added to queue")
  - [ ] Link list display (scrollable)
  - [ ] Remove link button
  - [ ] Clear all button
  - [ ] Queue size indicator

- [ ] URL Pattern Selection
  - [ ] Checkboxes for each pattern type
  - [ ] Enable/disable all button
  - [ ] Pattern statistics

- [ ] Status Display
  - [ ] Current crawl frequency
  - [ ] Next crawl time (prime-based)
  - [ ] Links in queue
  - [ ] Links crawled
  - [ ] Current URL

**Files**: `app/ui/tabs/tab_crawler.c` (new)

### 4.2 Training Tab Advanced Features (2 hours)

- [ ] Recursive sub-spheres visualization
- [ ] Zoom and pan controls
- [ ] Mouse-over tooltips
- [ ] 3D visualization mode
- [ ] 2D/3D toggle

**Data Structure**:
```c
typedef struct {
    int zoom_level;
    float pan_x, pan_y;
    bool show_3d;
    bool show_subspheres;
    int max_depth;
} SphereVisualizationConfig;
```

### 4.3 Collapsible Panels (1 hour)

- [ ] Integrate collapsible headers into all tabs
- [ ] Add click detection
- [ ] Implement smooth animations
- [ ] Save panel states
- [ ] Keyboard shortcuts (Ctrl+number)

### 4.4 LLM Tab Remaining Features (2 hours)

- [ ] Repetition penalty slider
- [ ] Frequency penalty slider
- [ ] Presence penalty slider
- [ ] Stop sequences input
- [ ] Random seed input
- [ ] Cancel generation button
- [ ] Keyboard shortcuts (Ctrl+Enter)
- [ ] Context menu (right-click)
- [ ] Drag and drop file upload

---

## PHASE 5: DEPENDENCY MANAGEMENT (2 hours) 📦

### 5.1 Create Installation Scripts (1 hour)

- [ ] Ubuntu/Debian script (`install_deps_ubuntu.sh`)
  - [ ] System packages (build-essential, libcurl, libxml2, libzip, etc.)
  - [ ] Optional: Build from source instructions

- [ ] CentOS/RHEL script (`install_deps_centos.sh`)
  - [ ] Development tools
  - [ ] System packages
  - [ ] EPEL repository setup

### 5.2 Dependency Documentation (1 hour)

- [ ] Create DEPENDENCIES.md
  - [ ] List all required libraries
  - [ ] Version requirements
  - [ ] Installation per distro
  - [ ] Build from source guide
  - [ ] Troubleshooting

---

## PHASE 6: TESTING & INTEGRATION (4 hours) ✅

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

## PHASE 7: DOCUMENTATION (2 hours) 📚

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

**Total Estimated Time**: ~34 hours

### Week 1 (16 hours)
- Phase 1: Library Reorganization (4h)
- Phase 2: Pure C File Processors (8h)
- Phase 3: Advanced Crawler Features (4h)

### Week 2 (18 hours)
- Phase 3: Advanced Crawler Features (2h remaining)
- Phase 4: Complete UI Enhancements (8h)
- Phase 5: Dependency Management (2h)
- Phase 6: Testing & Integration (4h)
- Phase 7: Documentation (2h)

---

## PRIORITY ORDER

### Critical Path (Must Complete First)
1. ✅ Library reorganization
2. ✅ Pure C file processors (DOCX/XLSX/PPTX)
3. ✅ Crawler prime randomization
4. ✅ Crawler UI enhancements

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

## CURRENT STATUS

**Phase**: Planning Complete ✅
**Next Step**: Begin Phase 1 - Library Reorganization
**Awaiting**: User confirmation to proceed

---

## NOTES

### Key Decisions
- ✅ Pure C implementation (no Python)
- ✅ Unified libdocproc library
- ✅ Prime-based randomization
- ✅ Advanced URL patterns
- ✅ Complete UI enhancements
- ✅ Multi-distro support (Ubuntu + CentOS)

### External Dependencies (Minimal)
- libxml2 (XML parsing)
- libzip (ZIP handling)
- libcurl (HTTP - already used)
- poppler-utils (PDF - CLI tool)
- tesseract (OCR - CLI tool)

### Custom C Implementations
- YAML parser
- TOML parser
- CSV parser
- Office XML parsers
- ODF parsers
- EPUB parser

---

## READY TO BEGIN

**All planning complete. Awaiting user approval to start Phase 1!**

See `SECONDARY_OBJECTIVES.md` for complete details.