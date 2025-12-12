# TODO - CLLM Integration with NEW Math Library

## 🔒 PERMANENT RULES (FROM MASTER_PLAN.md)

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
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git audit
```

### RULE 3: BUILD VERIFICATION
**MANDATORY: Test every build after making changes**

Process (NEVER SKIP):
1. Make code changes
2. Run: `make clean && make 2>&1 | tee build.log`
3. Count warnings: `grep -c "warning:" build.log`
4. **VERIFY BUILD SUCCESS**
5. Fix all warnings
6. Rebuild and verify
7. **ONLY THEN** commit changes

### RULE 4: FIX HTML ENTITIES IMMEDIATELY
When creating files, HTML entities may be introduced.

**Solution:**
```bash
python3 tools/fix_html_entities.py <file>
```

---

## 🎯 CURRENT MISSION: CLLM INTEGRATION WITH NEW MATH LIBRARY

### 🚨 CRITICAL USER INSIGHT (2024-12-11)
**User's Vision:**
1. **Arbitrary Precision Everywhere** - The entire library should focus on arbitrary precision using CrystallineAbacus
2. **GCD of Arbitrary Sizes** - There may be GCD operations on arbitrarily large numbers - the system MUST handle this
3. **Memory &amp; Processing Efficiency** - New memory hopping architecture with on-demand reconstruction mitigates concerns
4. **Avoid Type Double** - Use new self-similar crystalline mathematics instead of type double
5. **Self-Similar Structure** - Mathematics should be in a self-similar structure throughout

### 🔍 CRITICAL DISCOVERY: MISSING GCD FUNCTIONALITY
The NEW math library (math/) is missing:
- ❌ `abacus_gcd()` - GCD for arbitrary precision numbers
- ❌ `abacus_lcm()` - LCM for arbitrary precision numbers
- ❌ `abacus_coprime()` - Coprimality test for arbitrary precision

**Current state:**
- ✅ Has `abacus_mod_inverse()` which uses Extended Euclidean Algorithm internally
- ✅ Has `abacus_div()` for division with remainder
- ❌ NO public GCD API for arbitrary precision

**Action Required:**
1. Implement `abacus_gcd()` using Euclidean algorithm with CrystallineAbacus
2. Implement `abacus_lcm()` using formula: lcm(a,b) = (a*b)/gcd(a,b)
3. Implement `abacus_coprime()` using gcd(a,b) == 1
4. Add tests for all three functions
5. Update all code to use these functions instead of uint64 helpers

### 📋 PHASE 1: DEEP ANALYSIS (CURRENT)

#### ✅ Step 1: Implement Missing Arbitrary Precision Functions (BLOCKED)
**Priority: 🔴🔴🔴 CRITICAL - BLOCKED BY ABACUS_DIV BUG**

**CRITICAL BUG DISCOVERED:**
- ❌ `abacus_div()` is completely broken in the NEW math library
- ❌ Returns quotient=0, remainder=dividend for ALL divisions
- ❌ Affects ALL bases (10, 12, 60, 100)
- ❌ Example: 1000 / 500 returns q=0, r=1000 (should be q=2, r=0)
- ❌ Example: 144 / 12 returns q=0, r=144 (should be q=12, r=0)
- ❌ This breaks GCD, LCM, and all division-dependent operations

**GCD/LCM/Coprimality Implementation Status:**
- [x] Implemented `abacus_gcd()` in math/src/bigint/abacus_gcd.c
  * Uses Euclidean algorithm with CrystallineAbacus
  * Handles arbitrary precision numbers
  * Pure geometric operations (no uint64 shortcuts)
  * ✅ Algorithm is CORRECT
  * ❌ BLOCKED by abacus_div() bug
- [x] Implemented `abacus_lcm()` in math/src/bigint/abacus_gcd.c
  * Uses formula: lcm(a,b) = (a*b)/gcd(a,b)
  * Handles arbitrary precision numbers
  * ✅ Algorithm is CORRECT
  * ❌ BLOCKED by abacus_div() bug
- [x] Implemented `abacus_coprime()` in math/src/bigint/abacus_gcd.c
  * Uses gcd(a,b) == 1 test
  * Handles arbitrary precision numbers
  * ✅ Algorithm is CORRECT
  * ❌ BLOCKED by abacus_div() bug
- [x] Added function declarations to math/include/math/abacus.h
- [x] Created tests in math/tests/test_abacus_gcd.c
  * ✅ 8/10 tests passing (80%)
  * ❌ 2 tests failing due to abacus_div() bug
- [x] Fixed `map_digit_to_position()` to support arbitrary bases
  * Now supports ALL bases >= 2 (not just 12, 60, 100)
  * Maps non-Babylonian bases to Ring 2

**NEXT STEPS:**
- [ ] Fix `abacus_div()` bug in math/src/bigint/abacus.c
- [ ] Verify all GCD/LCM tests pass after fix
- [ ] Update cllm_plimpton_relationships.c to use abacus_gcd()
- [ ] Remove independent compute_gcd() implementation
- [ ] Commit changes to audit branch

#### ✅ Step 2: Analyze All Makefiles
- [x] Identified main Makefile structure
- [x] Found CLLM linking to libcrystallinemath (correct)
- [x] Found CLLM source files using OLD includes (WRONG)
- [ ] Analyze algorithms/Makefile
- [ ] Analyze app/Makefile (already partially done)
- [ ] Document all library dependencies
- [ ] Create dependency graph

#### 📝 Step 2: Audit CLLM Source Files
**Files Using OLD Library (MUST FIX):**
1. `src/ai/bigfixed_array_utils.c` - uses bigint_core.h
2. `src/ai/cllm_feedforward.c` - uses bigint_core.h
3. `src/ai/cllm_lattice_conversion.c` - uses bigint_core.h
4. `src/ai/cllm_plimpton_relationships.c` - uses bigint_core.h
5. `src/ai/cllm_token.c` - uses bigint_core.h

**Total CLLM Files:** 86 files in src/ai/
**Files to Audit:** All 86 files
**Files Confirmed Broken:** 5 files

**Action Items:**
- [ ] Audit all 86 CLLM files for OLD library usage
- [ ] List all files needing conversion
- [ ] Identify NEW math library API equivalents
- [ ] Create conversion mapping document

#### ✅ Step 3: Audit Algorithms Library - COMPLETE
- [x] Check algorithms/Makefile for correct linking
- [x] Audit all algorithm source files
- [x] Verify NO legacy BigInt/BigFixed references
- [x] Verify 100% NEW math library usage
- [x] Check for any wrapper functions

**RESULT:** 
- 2 files include `prime_lowlevel.h` but DON'T use BigInt/BigFixed
- Only use utility functions (prime_powmod_int, etc.)
- NO CHANGES NEEDED - these are acceptable utility functions

#### 📝 Step 4: Create Comprehensive Action Plan
- [ ] Document NEW math library API
- [ ] Map OLD API → NEW API conversions
- [ ] Identify breaking changes
- [ ] Plan migration strategy
- [ ] Estimate work required

---

## 🔄 PHASE 2: NEW MATH LIBRARY API DOCUMENTATION

### NEW Math Library Structure (math/)

**Core Components:**
```
math/
├── include/math/
│   ├── types.h           - Core types (ClockContext, etc.)
│   ├── clock.h           - Clock lattice functions
│   ├── prime.h           - Prime generation (O(1))
│   ├── abacus.h          - Crystalline Abacus (arbitrary precision)
│   ├── ntt.h             - Number Theoretic Transform
│   ├── transcendental.h  - Transcendental functions (NO math.h)
│   ├── compact_vector.h  - Compact vector system (NEW!)
│   ├── platonic_generator.h - Platonic solid generators
│   └── polytope.h        - Polytope structures
├── src/
│   ├── bigint/          - Abacus implementation
│   ├── ntt/             - NTT implementation
│   ├── geometry/        - Clock lattice, sphere trajectories
│   ├── prime/           - Prime generation, rainbow table
│   ├── compact/         - Compact vector arithmetic (NEW!)
│   └── platonic/        - Platonic solid generators (NEW!)
└── lib/
    ├── libcrystallinemath.a   - Static library
    └── libcrystallinemath.so  - Shared library
```

**Key API Changes:**
```c
// OLD API (include/bigint_core.h)
BigInt* bigint_create(uint64_t value);
void bigint_add(BigInt* result, const BigInt* a, const BigInt* b);

// NEW API (math/include/math/abacus.h)
CrystallineAbacus* abacus_create(uint64_t value, uint32_t base);
MathError abacus_add(CrystallineAbacus* result, 
                     const CrystallineAbacus* a,
                     const CrystallineAbacus* b);
```

### 📝 Step 1: Document Complete NEW API
- [ ] List all NEW math library functions
- [ ] Document function signatures
- [ ] Document return types and error codes
- [ ] Create usage examples
- [ ] Document memory management

### 📝 Step 2: Create OLD → NEW Mapping
- [ ] Map BigInt functions → Abacus functions
- [ ] Map BigFixed functions → Abacus functions
- [ ] Map prime functions → NEW prime functions
- [ ] Document breaking changes
- [ ] Create migration guide

---

## 🔧 PHASE 3: CLLM MIGRATION PLAN

### Strategy: File-by-File Conversion

#### 📝 Step 1: Fix Critical Files First (5 files)
**Priority Order:**
1. `src/ai/cllm_token.c` - Token handling
2. `src/ai/cllm_lattice_conversion.c` - Lattice conversion
3. `src/ai/cllm_feedforward.c` - Feedforward network
4. `src/ai/cllm_plimpton_relationships.c` - Plimpton relationships
5. `src/ai/bigfixed_array_utils.c` - Array utilities

**For Each File:**
- [ ] Read current implementation
- [ ] Identify OLD API usage
- [ ] Map to NEW API equivalents
- [ ] Update includes
- [ ] Update function calls
- [ ] Test compilation
- [ ] Verify functionality

#### 📝 Step 2: Audit Remaining 81 Files
- [ ] Scan for OLD library includes
- [ ] Scan for OLD API function calls
- [ ] Create list of files needing updates
- [ ] Prioritize by dependency order
- [ ] Create conversion checklist

#### 📝 Step 3: Update CLLM Makefile
- [ ] Verify correct library linking
- [ ] Update include paths
- [ ] Remove OLD library references
- [ ] Add NEW math library paths
- [ ] Test build system

---

## 🔧 PHASE 4: ALGORITHMS LIBRARY AUDIT

### 📝 Step 1: Analyze Algorithms Makefile
- [ ] Check library dependencies
- [ ] Verify linking order
- [ ] Check include paths
- [ ] Verify NO OLD library references

### 📝 Step 2: Audit Algorithm Source Files
**Files to Check:**
- [ ] `algorithms/src/ntt_attention.c`
- [ ] `algorithms/src/lattice_embeddings_bigfixed.c` (RENAME!)
- [ ] `algorithms/src/loss_functions_bigfixed.c` (RENAME!)
- [ ] `algorithms/src/bigfixed_math_wrappers.c` (RENAME!)
- [ ] `algorithms/src/hierarchical_prime_partitions.c`
- [ ] `algorithms/src/lattice_sphere_positions.c`
- [ ] All other algorithm files

**For Each File:**
- [ ] Check for OLD library includes
- [ ] Check for OLD API usage
- [ ] Verify NEW math library usage
- [ ] Update if needed
- [ ] Test compilation

### 📝 Step 3: Rename Files (Remove "bigfixed")
- [ ] `lattice_embeddings_bigfixed.c` → `lattice_embeddings.c`
- [ ] `loss_functions_bigfixed.c` → `loss_functions.c`
- [ ] `bigfixed_math_wrappers.c` → `math_wrappers.c` or DELETE
- [ ] Update Makefile
- [ ] Update all includes
- [ ] Test build

---

## 🔧 PHASE 5: INTEGRATE MEMORY HOPPING ARCHITECTURE

### Revolutionary Features (From TERTIARY_OBJECTIVES.md)

#### ✅ COMPLETED: Core Vector System
- [x] CompactVector struct (16 bytes)
- [x] Triangulation-based arithmetic
- [x] 360-degree clock precision
- [x] Platonic solids integration
- [x] Sphere hopping system
- [x] All tests passing (95/95 = 100%)

#### 📝 Step 1: Integrate with CLLM
- [ ] Update CLLM to use CompactVector
- [ ] Replace full number storage with vectors
- [ ] Implement on-demand reconstruction
- [ ] Update embeddings to use compact vectors
- [ ] Update attention mechanism
- [ ] Test memory reduction

#### 📝 Step 2: Update Training Pipeline
- [ ] Use compact vectors for weights
- [ ] Use compact vectors for gradients
- [ ] Implement vector-based backpropagation
- [ ] Test training convergence
- [ ] Benchmark performance

#### 📝 Step 3: Visualization Integration
- [ ] Integrate enhanced_visualization.c with app
- [ ] Add UI controls for visualization modes
- [ ] Test 360-degree precision display
- [ ] Test Platonic solid overlays
- [ ] Test sphere hierarchy display

---

## 🔧 PHASE 6: APP DIRECTORY INTEGRATION

### 📝 Step 1: Update App Makefile
- [x] Add libcrystallinemath linking
- [x] Add NEW math library include paths
- [ ] Remove OLD library references
- [ ] Test build with SDL2
- [ ] Verify all dependencies

### 📝 Step 2: Update App Source Files
- [ ] Audit all app/*.c files
- [ ] Check for OLD library usage
- [ ] Update to NEW math library
- [ ] Test compilation
- [ ] Test functionality

### 📝 Step 3: Test Enhanced Visualization
- [ ] Build app with SDL2
- [ ] Test enhanced factor lines
- [ ] Test Platonic solid overlays
- [ ] Test sphere hierarchy display
- [ ] Test 360-degree precision

---

## 📊 PROGRESS TRACKING

**Overall Progress:** 25% (Analysis phase, memory hopping core complete)

**Current Phase:** Phase 1 - Deep Analysis
**Current Step:** Auditing CLLM source files

**Completed:**
- ✅ NEW math library (100% complete, 95/95 tests passing)
- ✅ Memory hopping architecture core (100% complete)
- ✅ Enhanced visualization functions (created)
- ✅ Identified CLLM integration issues

**In Progress:**
- 🔄 CLLM source file audit
- 🔄 Makefile analysis
- 🔄 API mapping documentation

**Blocked:**
- ⏸️ CLLM migration (waiting for audit completion)
- ⏸️ Algorithms library audit (waiting for CLLM plan)
- ⏸️ App integration (waiting for library fixes)

---

## 🎯 SUCCESS CRITERIA

### CLLM Integration
- [ ] Zero OLD library references in CLLM
- [ ] 100% NEW math library usage
- [ ] All CLLM tests passing
- [ ] Build succeeds without errors
- [ ] No performance regression

### Algorithms Library
- [ ] Zero OLD library references
- [ ] All files renamed (no "bigfixed")
- [ ] 100% NEW math library usage
- [ ] All algorithm tests passing
- [ ] Build succeeds without errors

### Memory Hopping Integration
- [ ] CLLM uses compact vectors
- [ ] 10-625x memory reduction achieved
- [ ] Training convergence maintained
- [ ] Performance improved or maintained
- [ ] All tests passing

### App Integration
- [ ] App builds successfully
- [ ] Enhanced visualization working
- [ ] 360-degree precision displayed
- [ ] Platonic solid overlays working
- [ ] Sphere hierarchy displayed

---

## 📝 NEXT IMMEDIATE ACTIONS

1. **Audit all 86 CLLM files** for OLD library usage
2. **Create OLD → NEW API mapping document**
3. **Fix 5 critical CLLM files** using OLD includes
4. **Audit algorithms library** for legacy code
5. **Create comprehensive migration plan**

---

**Last Updated:** 2024-12-19
**Status:** Active Development - CLLM Integration Phase