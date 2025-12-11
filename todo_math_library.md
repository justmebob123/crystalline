# 🔒 MASTER PLAN RULES - PERMANENT REFERENCE 🔒

## ⚠️ CRITICAL: THIS WORK MUST ONLY USE THE 'audit' FEATURE BRANCH ⚠️

---

## 🔴 RULE 0: ALWAYS READ THESE RULES FIRST 🔴
**MANDATORY WITH EVERY RESPONSE**

Before ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Read AUDIT.md for current architectural state
3. Read SECONDARY_OBJECTIVES.md for detailed tasks
4. Update todo.md with current progress

## 🔴 RULE 1: CRITICAL REFERENCE FILES 🔴

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

## 🔴 RULE 2: GIT OPERATIONS 🔴

**ALWAYS use correct authentication:**
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git audit
```

## 🔴 RULE 3: MASTER_PLAN.md IS READ-ONLY 🔴

**⚠️ DO NOT EDIT WITHOUT EXPLICIT APPROVAL ⚠️**

## 🔴 RULE 4: BUILD VERIFICATION 🔴

**MANDATORY: Test every build after making changes**

Process (NEVER SKIP):
1. Make code changes
2. Run: `make clean && make 2>&1 | tee build.log`
3. Count warnings: `grep -c "warning:" build.log`
4. **VERIFY BUILD SUCCESS**
5. Fix all warnings
6. Rebuild and verify
7. **ONLY THEN** commit changes

## 🔴 RULE 5: FIX HTML ENTITIES IMMEDIATELY 🔴

```bash
python3 tools/fix_html_entities.py <file>
```

---

# Crystalline Math Library Refactoring - TODO

## Phase 1: Create Feature Branch and New Math Directory ✅

### Task 1.1: Create Feature Branch
- [x] Create 'audit' branch
- [x] Mark all audit files with branch requirement
- [x] Create todo_math_library.md with permanent rules

### Task 1.2: Create New Math Directory Structure
- [ ] Create /math directory in repo root
- [ ] Create /math/src directory
- [ ] Create /math/include directory
- [ ] Create /math/Makefile
- [ ] Create /math/README.md

### Task 1.3: Copy Crystalline Lattice Files
- [ ] Identify all core math files from crystalline/
- [ ] Copy to /math with proper organization
- [ ] Commit initial structure

---

## Phase 2: Analyze Current Implementation

### Task 2.1: Inventory Core Math Components
- [ ] List all bigfixed_* functions
- [ ] List all bigint_* functions
- [ ] List all prime_* transcendental functions
- [ ] List all lattice_* geometric functions
- [ ] List all clock_* functions
- [ ] Identify deterministic prime generation functions

### Task 2.2: Identify Legacy Code
- [ ] Find old prime generation methods (trial division, sieving)
- [ ] Find float usage (should be double or bigfixed)
- [ ] Find math.h dependencies
- [ ] Find inconsistent naming patterns

### Task 2.3: Analyze Dependencies
- [ ] Map which functions call which
- [ ] Identify circular dependencies
- [ ] Find external library dependencies (OpenSSL, GMP)

---

## Phase 3: Design New Library Structure

### Task 3.1: Define Module Organization
- [ ] Core arithmetic (add, sub, mul, div)
- [ ] Transcendental functions (sqrt, sin, cos, log, exp)
- [ ] Arbitrary precision (bigfixed, bigint)
- [ ] Geometric functions (lattice, clock)
- [ ] Prime generation (deterministic clock-based)
- [ ] Cryptographic wrappers (OpenSSL)

### Task 3.2: Define Naming Conventions
**Standard Prefixes:**
- `math_` - Basic arithmetic (double precision)
- `bigfixed_` - Arbitrary precision fixed-point
- `bigint_` - Arbitrary precision integer
- `lattice_` - Geometric lattice operations
- `clock_` - Clock lattice operations
- `prime_` - Prime number operations

**NO ridiculous prefixes like "prime_sqrt" - use proper module names:**
- `math_sqrt()` - double precision
- `bigfixed_sqrt()` - arbitrary precision
- NOT "prime_sqrt()" unless it's specifically prime-related

### Task 3.3: Define API Design Principles
- [ ] Self-contained (no external math.h)
- [ ] Consistent error handling
- [ ] Context-based where appropriate
- [ ] Thread-safe operations
- [ ] SIMD-optimized where possible
- [ ] Prefer double over float
- [ ] Prefer bigfixed for arbitrary precision

---

## Phase 4: Implement Core Modules

### Task 4.1: Core Arithmetic Module
**File: math/src/core/arithmetic.c**
**Header: math/include/math/arithmetic.h**

Functions to implement:
- [ ] `double math_add(double a, double b)`
- [ ] `double math_sub(double a, double b)`
- [ ] `double math_mul(double a, double b)`
- [ ] `double math_div(double a, double b)`
- [ ] `double math_mod(double a, double b)`
- [ ] `double math_abs(double x)`
- [ ] `double math_min(double a, double b)`
- [ ] `double math_max(double a, double b)`

### Task 4.2: Transcendental Functions Module
**File: math/src/core/transcendental.c**
**Header: math/include/math/transcendental.h**

Functions to implement:
- [ ] `double math_sqrt(double x)` - Square root
- [ ] `double math_cbrt(double x)` - Cube root
- [ ] `double math_pow(double x, double y)` - Power
- [ ] `double math_exp(double x)` - Exponential
- [ ] `double math_log(double x)` - Natural logarithm
- [ ] `double math_log10(double x)` - Base-10 logarithm
- [ ] `double math_log2(double x)` - Base-2 logarithm
- [ ] `double math_sin(double x)` - Sine
- [ ] `double math_cos(double x)` - Cosine
- [ ] `double math_tan(double x)` - Tangent
- [ ] `double math_asin(double x)` - Arcsine
- [ ] `double math_acos(double x)` - Arccosine
- [ ] `double math_atan(double x)` - Arctangent
- [ ] `double math_atan2(double y, double x)` - Two-argument arctangent
- [ ] `double math_sinh(double x)` - Hyperbolic sine
- [ ] `double math_cosh(double x)` - Hyperbolic cosine
- [ ] `double math_tanh(double x)` - Hyperbolic tangent

### Task 4.3: Rounding and Utility Functions
**File: math/src/core/rounding.c**
**Header: math/include/math/rounding.h**

Functions to implement:
- [ ] `double math_floor(double x)`
- [ ] `double math_ceil(double x)`
- [ ] `double math_round(double x)`
- [ ] `double math_trunc(double x)`
- [ ] `double math_fmod(double x, double y)`
- [ ] `double math_remainder(double x, double y)`

### Task 4.4: BigFixed Arbitrary Precision Module
**File: math/src/bigfixed/bigfixed_core.c**
**Header: math/include/math/bigfixed.h**

Core operations:
- [ ] `BigFixed* bigfixed_new(int precision)`
- [ ] `void bigfixed_free(BigFixed* bf)`
- [ ] `BigFixed* bigfixed_from_double(double value, int precision)`
- [ ] `double bigfixed_to_double(const BigFixed* bf)`
- [ ] `int bigfixed_add(BigFixed* result, const BigFixed* a, const BigFixed* b)`
- [ ] `int bigfixed_sub(BigFixed* result, const BigFixed* a, const BigFixed* b)`
- [ ] `int bigfixed_mul(BigFixed* result, const BigFixed* a, const BigFixed* b)`
- [ ] `int bigfixed_div(BigFixed* result, const BigFixed* a, const BigFixed* b)`

Transcendental operations:
- [ ] `int bigfixed_sqrt(BigFixed* result, const BigFixed* x)`
- [ ] `int bigfixed_exp(BigFixed* result, const BigFixed* x)`
- [ ] `int bigfixed_log(BigFixed* result, const BigFixed* x)`
- [ ] `int bigfixed_sin(BigFixed* result, const BigFixed* x)`
- [ ] `int bigfixed_cos(BigFixed* result, const BigFixed* x)`

### Task 4.5: BigInt Arbitrary Precision Integer Module
**File: math/src/bigint/bigint_core.c**
**Header: math/include/math/bigint.h**

Core operations:
- [ ] `BigInt* bigint_new(void)`
- [ ] `void bigint_free(BigInt* bi)`
- [ ] `BigInt* bigint_from_int64(int64_t value)`
- [ ] `int64_t bigint_to_int64(const BigInt* bi)`
- [ ] `int bigint_add(BigInt* result, const BigInt* a, const BigInt* b)`
- [ ] `int bigint_sub(BigInt* result, const BigInt* a, const BigInt* b)`
- [ ] `int bigint_mul(BigInt* result, const BigInt* a, const BigInt* b)`
- [ ] `int bigint_div(BigInt* quotient, BigInt* remainder, const BigInt* a, const BigInt* b)`
- [ ] `int bigint_mod(BigInt* result, const BigInt* a, const BigInt* m)`
- [ ] `int bigint_gcd(BigInt* result, const BigInt* a, const BigInt* b)`
- [ ] `int bigint_mod_exp(BigInt* result, const BigInt* base, const BigInt* exp, const BigInt* mod)`

### Task 4.6: Clock Lattice Module
**File: math/src/geometry/clock_lattice.c**
**Header: math/include/math/clock_lattice.h**

Deterministic prime generation:
- [ ] `void clock_init(ClockContext* ctx)`
- [ ] `int clock_map_prime_to_position(uint64_t prime, ClockPosition* pos)`
- [ ] `uint64_t clock_position_to_prime(const ClockPosition* pos)`
- [ ] `int clock_is_valid_position(const ClockPosition* pos)`
- [ ] `uint64_t clock_next_prime(const ClockPosition* current)`
- [ ] `uint64_t clock_prev_prime(const ClockPosition* current)`

### Task 4.7: Prime Number Module
**File: math/src/prime/prime_generation.c**
**Header: math/include/math/prime.h**

Deterministic generation (NEW - clock-based):
- [ ] `uint64_t prime_nth(uint64_t n)` - Get nth prime deterministically
- [ ] `uint64_t prime_next(uint64_t p)` - Next prime after p
- [ ] `uint64_t prime_prev(uint64_t p)` - Previous prime before p
- [ ] `int prime_is_prime(uint64_t n)` - Check if prime (using clock validation)
- [ ] `uint64_t prime_count_below(uint64_t n)` - Count primes below n

Legacy functions to REMOVE:
- [ ] Remove trial division implementations
- [ ] Remove sieve implementations
- [ ] Remove probabilistic tests

### Task 4.8: Lattice Geometry Module
**File: math/src/geometry/lattice.c**
**Header: math/include/math/lattice.h**

Functions:
- [ ] `void lattice_init(LatticeContext* ctx)`
- [ ] `double lattice_distance(const Point* a, const Point* b)`
- [ ] `void lattice_map_to_sphere(const Point* p, SpherePoint* sp)`
- [ ] `void lattice_stereographic_project(const SpherePoint* sp, Point* p)`

---

## Phase 5: Remove Legacy Code

### Task 5.1: Remove Trial Division
- [ ] Find all trial division implementations
- [ ] Replace with clock-based validation
- [ ] Remove old code
- [ ] Update tests

### Task 5.2: Remove Sieve Implementations
- [ ] Find all sieve implementations
- [ ] Replace with deterministic generation
- [ ] Remove old code
- [ ] Update tests

### Task 5.3: Replace Float with Double
- [ ] Find all float usage
- [ ] Replace with double
- [ ] Update function signatures
- [ ] Update tests

### Task 5.4: Remove math.h Dependencies
- [ ] Find all #include <math.h>
- [ ] Replace with internal implementations
- [ ] Verify no external math dependencies
- [ ] Update tests

---

## Phase 6: Testing and Validation

### Task 6.1: Unit Tests
- [ ] Test all arithmetic functions
- [ ] Test all transcendental functions
- [ ] Test bigfixed operations
- [ ] Test bigint operations
- [ ] Test clock lattice functions
- [ ] Test prime generation

### Task 6.2: Performance Tests
- [ ] Benchmark against math.h
- [ ] Benchmark against GMP
- [ ] Optimize hot paths
- [ ] Add SIMD where appropriate

### Task 6.3: Integration Tests
- [ ] Test with existing codebase
- [ ] Verify no regressions
- [ ] Test edge cases
- [ ] Test error handling

---

## Phase 7: Documentation

### Task 7.1: API Documentation
- [ ] Document all public functions
- [ ] Add usage examples
- [ ] Document error codes
- [ ] Create migration guide

### Task 7.2: Design Documentation
- [ ] Document clock lattice algorithm
- [ ] Document deterministic prime generation
- [ ] Document bigfixed implementation
- [ ] Document design decisions

---

## Success Criteria

- [ ] Zero math.h dependencies
- [ ] Zero external math library dependencies (except OpenSSL for crypto)
- [ ] All functions use double or bigfixed (no float)
- [ ] Deterministic prime generation working
- [ ] All legacy code removed
- [ ] Consistent naming conventions
- [ ] Complete test coverage
- [ ] Performance within 10% of math.h
- [ ] Clean build with zero warnings
- [ ] Complete documentation

---

**Current Phase:** Phase 1 - Creating Feature Branch and Directory Structure  
**Branch:** audit (REQUIRED)  
**Status:** IN PROGRESS