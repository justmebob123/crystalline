# MASTER PLAN - CRYSTALLINE CLLM INTEGRATION

---

## 🔒 RULES (HIGHEST PRIORITY - READ FIRST)

### ⭐ RULE 0: ALWAYS READ THESE RULES FIRST ⭐
**MANDATORY WITH EVERY RESPONSE**

Before ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Read AUDIT.md for current architectural state
3. Read SECONDARY_OBJECTIVES.md for detailed tasks
4. Update todo.md with current progress

### RULE 1: CRITICAL REFERENCE FILES
**Before making ANY code changes, ALWAYS check these files:**

1. **app/ui/sphere_visualization.c** - 3D kissing spheres visualization
   - PURE crystalline mathematics ONLY (NO math.h)
   - Uses ONLY prime_* functions from crystalline library
   - Implements icosahedral geometry with golden ratio
   - 13 spheres: 1 control + 12 workers

2. **src/geometry/clock_lattice.c** - Babylonian clock structure
   - Ring structure: 12, 60, 60, 100
   - Prime position mapping

3. **src/ai/cllm_kissing_spheres.c** - Kissing spheres implementation
   - 12 neighbors per point
   - Shared memory structure

**CRITICAL**: Never use math.h or standard library functions. Only use prime_* functions.

### RULE 2: GIT OPERATIONS
**ALWAYS use correct authentication:**
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

### RULE 3: THIS FILE IS READ-ONLY
**⚠️ DO NOT EDIT WITHOUT EXPLICIT APPROVAL ⚠️**

This file contains OBJECTIVES ONLY - NO status updates, NO ephemeral information.

**NEVER ADD:**
- ❌ Status updates or completion markers
- ❌ Progress percentages or tracking
- ❌ "Current focus" or "what I'm working on"
- ❌ Known issues or bug tracking
- ❌ New objectives without asking first

**ALWAYS REMEMBER:**
- ✅ This file contains STATIC STRUCTURAL DESIGN only
- ✅ Status tracking happens in todo.md ONLY
- ✅ Ask user before adding ANY new objectives

### RULE 4: BUILD VERIFICATION
**MANDATORY: Test every build after making changes**

Process (NEVER SKIP):
1. Make code changes
2. Run: `make clean && make 2>&1 | tee build.log`
3. Count warnings: `grep -c "warning:" build.log`
4. **VERIFY BUILD SUCCESS**
5. Fix all warnings
6. Rebuild and verify
7. **ONLY THEN** commit changes

### RULE 5: FIX HTML ENTITIES IMMEDIATELY
When creating files, HTML entities may be introduced.

**Solution:**
```bash
python3 tools/fix_html_entities.py <file>
```

Use immediately after creating any C/C++ source file.

---

## 🎯 CORE OBJECTIVES

### OBJECTIVE 1: Library Distribution Architecture
- Build both shared (.so) and static (.a) libraries
- Maintain modular architecture: crystalline → algorithms → cllm → crawler
- Support flexible deployment

### OBJECTIVE 2: Crystalline Training Pipeline
**Sub-objectives:**
- 2A: Integrate Crystalline GCD Optimizations
- 2B: Remove ALL Legacy Loss Functions
- 2C: Rename "Crystalline" to Default
- 2D: Remove ALL "Standard" and "Legacy" Code
- 2E: Crystalline Math Everywhere (NO math.h)
- 2F: Static Libraries as Primary
- 2G: Kissing Spheres as ONLY Threading
- 2H: Remove ALL Conditional Compilation

### OBJECTIVE 3: Kissing Spheres UI Integration
- Integrate sphere visualization into training tab
- Display real-time sphere statistics
- Show 12-fold symmetry structure
- Show node zero (control thread) status

### OBJECTIVE 4: LLM Tab Integration
- Verify uses new training pipeline models
- Add model loading from kissing spheres checkpoints
- Add inference performance metrics

### OBJECTIVE 5: Verify Crystalline Math Integration
- Verify NO math.h usage in core libraries
- Verify training uses crystalline math
- Performance comparison: crystalline vs standard

### OBJECTIVE 6: Verify SIMD Integration
- Verify SIMD used in forward/backward pass
- Check SIMD usage in attention mechanism
- Performance metrics for SIMD acceleration

### OBJECTIVE 7: Verify 12-Fold Symmetry
- Verify enforced in thread allocation
- Verify enforced in sphere creation
- Verify used in positional encoding

### OBJECTIVE 8: Implement Node Zero (Control Thread)
- Design control thread architecture
- Ensure control thread never processes batches
- Implement coordination logic

### OBJECTIVE 9: Verify Recursive Sphere Geometry
- Analyze recursive sphere implementation
- Verify integration with training
- Verify hierarchy levels calculation

### OBJECTIVE 10: Infinite Recursive Self-Similar 12-Fold Symmetry
**Critical Understanding:**
- Each thread can become control thread for 12 children
- Creates fractal hierarchy with infinite depth possible
- Each level maintains 12-fold symmetry
- Thread count adapts dynamically to CPU availability

### OBJECTIVE 11: Recursive Control Threads
- Every thread can be control thread for 12 children
- Control threads NEVER process batches
- Only leaf worker threads process batches
- Dynamic depth based on workload

### OBJECTIVE 12: Integrate Recursive Spheres with Threading
- Map each thread to a sphere in hierarchy
- Use sphere geometry for thread coordination
- Implement sphere-based work distribution

### OBJECTIVE 13: Mathematical Framework Integration
**Sub-objectives:**
- 13A: Integrate L(n,d,k,λ) Lattice Formula
- 13B: Integrate θ(n,k,λ,ω,ψ) Angular Position
- 13C: Initialize 12 Kissing Sphere Neighbors
- 13D: Implement NTT-Based O(n log n) Attention
- 13E: Apply Cymatic Frequency Resonance

### OBJECTIVE 14: Create Missing Analysis Tools
**Required Tools:**
1. tools/init_lattice_embeddings
2. tools/benchmark_ntt_attention
3. tools/validate_kissing_spheres
4. tools/analyze_cymatic_resonance
5. tools/visualize_angular_positions

### OBJECTIVE 15: Comprehensive Testing
- Unit tests for all components
- Integration tests for full pipeline
- Performance benchmarks
- Quality evaluation

### OBJECTIVE 16: Repository Validation
- Validate EVERY file for correctness
- Remove compiled files from repository
- Clean up technical debt
- Consolidate redundant code

### OBJECTIVE 17: UI and CLI Analysis
- Deep analysis of all UI tabs
- Validate global input system
- Analyze all CLI tools
- Ensure feature parity

### OBJECTIVE 18: Clean Up Technical Debt
- Identify incomplete integrations
- Eliminate adapter layers
- Fix broken dependencies
- Consolidate duplicate functionality

### OBJECTIVE 19: Prevent Unused Design Creation
- Audit all design documents
- Establish execution-first rules
- Delete unused designs
- Validate all implementations

### OBJECTIVE 20: File-by-File Repository Audit
- Create complete file inventory
- Audit each file individually
- Fix or remove each file
- Establish file maintenance rules

### OBJECTIVE 21: Babylonian Clock Lattice
**Status: PARTIALLY COMPLETE**
- Implement TRUE crystalline lattice structure
- Division by zero as fundamental truth
- Riemann sphere mapping
- Stereographic projection

---

## 📋 ARCHITECTURAL PRINCIPLES

### Core Design Principles
1. **Pure Crystalline Mathematics**: NO math.h, only prime_* functions
2. **12-Fold Symmetry**: Throughout all structures
3. **Kissing Spheres**: ONLY threading model
4. **Recursive Hierarchy**: Infinite self-similar structure
5. **No Conditional Compilation**: One codebase, one design
6. **No Legacy Code**: Remove all "standard" implementations
7. **Static Libraries Primary**: Shared libraries optional
8. **Execution-First**: Validate before documenting

### Threading Architecture
- 1 control thread (Node 0)
- 12 worker threads per level
- Infinite recursive depth possible
- Dynamic scaling based on CPU availability
- Control threads NEVER process batches
- Only leaf workers process batches

### Memory Structure
- Crystalline lattice abacus
- 12-fold memory structure
- Shared memory between spheres
- Cache locality based on sphere proximity

### Mathematical Foundation
- L(n,d,k,λ) lattice formula for embeddings
- θ(n,k,λ,ω,ψ) angular position for attention
- NTT-based O(n log n) attention
- Cymatic frequency resonance (432 Hz, 528 Hz, etc.)
- GCD-based similarity (20-400x faster)

---

## 🔧 BUILD SYSTEM

### Library Structure
```
libcrystalline.so/.a  - Core crystalline math
libalgorithms.so/.a   - Algorithms and optimizations
libcllm.so/.a         - CLLM model implementation
libcrawler.so/.a      - Web crawler integration
```

### Compilation Flags
```
-Wall -Wextra -O2 -mavx2 -mfma
```

### Build Commands
```bash
make clean          # Clean all artifacts
make -j$(nproc)     # Build all libraries
make app            # Build application
make tools          # Build all tools
```

---

## 📚 DOCUMENTATION STRUCTURE

### Primary Documents
- **MASTER_PLAN.md**: This file - objectives and architecture
- **AUDIT.md**: Current architectural state and violations
- **SECONDARY_OBJECTIVES.md**: Detailed implementation tasks
- **todo.md**: Current progress and status updates

### Reference Documents
- **README.md**: Project overview and setup
- **docs/**: Detailed documentation for each component

---

## ✅ SUCCESS CRITERIA

### Build Quality
- Zero errors
- Zero warnings
- Clean compilation
- All tests passing

### Code Quality
- No math.h usage
- No legacy code
- No conditional compilation
- No adapter layers
- No duplicate functionality

### Architecture Quality
- Pure crystalline mathematics
- 12-fold symmetry throughout
- Kissing spheres threading only
- Recursive hierarchy implemented
- All mathematical formulas integrated

### Performance Quality
- 5-20x overall speedup
- 10-100x speedup for long sequences (NTT)
- 20% lower final loss
- 30% faster convergence

---

**Last Updated**: 2024
**Status**: Active Development
**Priority**: Follow objectives in order unless blocked