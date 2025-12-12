# CLLM MIGRATION ACTION PLAN
## Complete Migration to NEW Math Library

**Date**: 2024-12-11  
**Branch**: audit  
**Goal**: Migrate CLLM from OLD library (BigInt/BigFixed) to NEW library (CrystallineAbacus)

---

## EXECUTIVE SUMMARY

### Current State
The Crystalline CLLM is a **FULLY IMPLEMENTED** revolutionary geometric computing system with:
- ✅ All 5 Platonic solids implemented (2,900 lines)
- ✅ Complete blind recovery system (23 algorithm files)
- ✅ Clock lattice mapping throughout
- ✅ Kissing spheres threading
- ✅ NTT attention (already migrated to NEW library)
- ✅ Harmonic integration
- ✅ Complete model file format

### The Challenge
**7 CLLM files** still use OLD library (BigInt/BigFixed):
1. `cllm_embeddings.c` - Uses BigFixed for embeddings
2. `bigfixed_array_utils.c` - BigFixed array utilities
3. `cllm_token.c` - Uses BigInt for rainbow table
4. `cllm_training_threaded.c` - Comments reference BigFixed
5. `cllm_production.c` - Comments reference BigFixed
6. `cllm_lattice_conversion.c` - BigFixed ↔ float conversion
7. `cllm_optimizer.c` - References BigFixed optimizer

### Migration Scope
- **Preserve**: All geometric architecture (Platonic, clock, spheres, recovery)
- **Replace**: OLD library (BigInt/BigFixed) → NEW library (CrystallineAbacus)
- **Complete**: Hierarchical prime generation (finish TODOs)

---

## PHASE 1: COMPLETE HIERARCHICAL PRIME GENERATION ⏳

### 1.1 Finish hierarchical_prime_partitions.c
**Location**: `algorithms/src/hierarchical_prime_partitions.c`

**TODOs to implement**:
1. `hierarchical_abacus_create()` - Create with CrystallineAbacus cache
2. `hierarchical_abacus_next_prime()` - Generate primes using Abacus
3. `hierarchical_abacus_is_prime()` - Primality check with caching
4. `hierarchical_abacus_in_partition()` - Range and mod 12 check
5. `hierarchical_abacus_cache_prime()` - Cache management
6. `hierarchical_abacus_cache_lookup()` - Cache search
7. `hierarchical_abacus_generate_primes()` - Bulk generation

**Key Requirements**:
- Use CrystallineAbacus for all prime storage
- Implement mod 12 filtering
- Implement range checking
- Implement caching with parent references
- Thread-safe operations (atomic counters)

**Estimated Time**: 1-2 days

### 1.2 Finish lattice_sphere_positions.c
**Location**: `algorithms/src/lattice_sphere_positions.c`

**TODOs to implement**:
1. `create_lattice_partition()` - Store ranges as CrystallineAbacus
2. `is_prime_in_partition()` - Range check with Abacus
3. `get_partition_size()` - Calculate using Abacus
4. `create_12_sphere_positions()` - Divide range into 12 partitions

**Key Requirements**:
- Use CrystallineAbacus for range boundaries
- Implement partition division (split range into 12 parts)
- Map to clock positions

**Estimated Time**: 1 day

---

## PHASE 2: MIGRATE CLLM CORE FILES (7 files) ⏳

### 2.1 File 1: cllm_token.c (PRIORITY)
**Current**: Uses BigInt for rainbow table lookups

**Migration**:
- Replace `BigInt* prime = rainbow_table_get_prime(n)` with Abacus version
- Update `crystalline_compute_ulam_position()` to use Abacus
- Update `CrystallineToken` structure if needed

**Estimated Time**: 1 day

### 2.2 File 2: bigfixed_array_utils.c
**Current**: BigFixed array utilities

**Migration**:
- Replace BigFixed with CrystallineAbacus
- Update allocation functions
- Update conversion functions

**Estimated Time**: 0.5 days

### 2.3 File 3: cllm_embeddings.c
**Current**: Uses BigFixed for embeddings

**Migration**:
- Replace BigFixed matrix with CrystallineAbacus
- Update embedding initialization
- Update lattice-based embeddings

**Estimated Time**: 1 day

### 2.4 File 4: cllm_lattice_conversion.c
**Current**: BigFixed ↔ float conversion

**Migration**:
- Replace with CrystallineAbacus ↔ float conversion
- Update `cllm_float_to_bigfixed()` → `cllm_float_to_abacus()`
- Update `cllm_bigfixed_to_float()` → `cllm_abacus_to_float()`

**Estimated Time**: 0.5 days

### 2.5 Files 5-7: Training/Production/Optimizer
**Current**: Comments reference BigFixed, but may not actually use it

**Action**: Verify actual usage, update if needed

**Estimated Time**: 0.5 days

---

## PHASE 3: RAINBOW TABLE MIGRATION ⏳

### 3.1 Current State
**Multiple implementations**:
- NEW: `math/src/prime/rainbow_table.c` (uses NEW library?)
- OLD: `src/geometry/prime_rainbow.c` (uses BigInt)

### 3.2 Strategy
**Option A**: Migrate OLD rainbow table to use NEW library
**Option B**: Use NEW library rainbow table exclusively
**Option C**: Replace with hierarchical prime generation

**Recommendation**: Option B + C
- Use NEW library rainbow table for small primes
- Use hierarchical generation for large primes
- Best of both worlds

**Estimated Time**: 1 day

---

## PHASE 4: INTEGRATION & TESTING ⏳

### 4.1 Integration Tasks
1. Update CLLM to use hierarchical prime generation
2. Replace rainbow table calls with hierarchical system
3. Verify all geometric properties preserved
4. Test blind recovery with NEW library
5. Test harmonic integration
6. Test NTT attention
7. Test threading model

**Estimated Time**: 2-3 days

### 4.2 Validation
1. Verify Euler's formula: V - E + F = 2
2. Verify 12-fold symmetry throughout
3. Verify blind recovery works (25% corruption tolerance)
4. Verify performance (5-20x speedup targets)
5. Verify all 5 Platonic models work

**Estimated Time**: 1-2 days

---

## PHASE 5: CLEANUP & DOCUMENTATION ⏳

### 5.1 Remove OLD Library
1. Delete `crystalline/` directory
2. Delete `src/core/` (BigInt/BigFixed implementations)
3. Delete `include/` OLD library headers
4. Update all includes
5. Verify clean build

**Estimated Time**: 1 day

### 5.2 Documentation
1. Update MASTER_PLAN.md
2. Update AUDIT.md
3. Create MIGRATION_COMPLETE.md
4. Update README.md
5. Document NEW library usage

**Estimated Time**: 1 day

---

## TOTAL TIMELINE

### Estimated Duration: 10-14 days

**Breakdown**:
- Phase 1: Hierarchical system (2-3 days)
- Phase 2: CLLM core files (3-4 days)
- Phase 3: Rainbow table (1 day)
- Phase 4: Integration & testing (3-5 days)
- Phase 5: Cleanup & docs (2 days)

### Risk Factors
- Type conflicts (already encountered and resolved)
- API differences between OLD and NEW libraries
- Integration complexity
- Testing coverage

### Success Criteria
- ✅ Zero OLD library dependencies
- ✅ All 5 Platonic models work
- ✅ Blind recovery functional (25% tolerance)
- ✅ Performance targets met (5-20x speedup)
- ✅ All tests passing
- ✅ Clean build (zero warnings)

---

## IMMEDIATE NEXT STEPS

### Step 1: Complete Hierarchical Prime Generation
**File**: `algorithms/src/hierarchical_prime_partitions.c`

**Implementation needed**:
```c
// Use CrystallineAbacus for:
- Prime storage (cache)
- Range boundaries (partition)
- Current candidate tracking
- Primality testing
- Mod 12 filtering
```

### Step 2: Migrate cllm_token.c
**Replace**:
```c
// OLD
BigInt* prime = rainbow_table_get_prime(n);

// NEW
CrystallineAbacus* prime = rainbow_table_get_prime_abacus(n);
```

### Step 3: Continue with remaining files

---

**Status**: Complete action plan ready
**Recommendation**: Begin with Phase 1 (hierarchical system)