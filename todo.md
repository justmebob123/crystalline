# Algorithms Library Migration - Replace OLD with NEW Math Library

## Current Status
The algorithms library currently uses OLD crystalline library (BigInt/BigFixed).
Need to replace with NEW math library (Crystalline Abacus).

## Files Using OLD Crystalline Library (Need Migration)

### High Priority - Core Math Operations
1. `algorithms/src/lattice_embeddings_bigfixed.c` - Uses BigFixed
2. `algorithms/src/loss_functions_bigfixed.c` - Uses BigFixed
3. `algorithms/src/bigfixed_math_wrappers.c` - Uses BigFixed
4. `algorithms/src/ntt_attention.c` - Uses BigInt NTT
5. `algorithms/src/hierarchical_prime_partitions.c` - Uses BigInt
6. `algorithms/src/lattice_sphere_positions.c` - Uses BigInt

## Migration Plan

### Phase 1: Verify NEW Math Library is Ready
- [x] NEW math library builds successfully
- [x] All 192 tests passing
- [x] Abacus supports ALL bases
- [x] NTT implemented with pure Abacus
- [x] Modular arithmetic available

### Phase 2: Migrate NTT Attention (PRIORITY) - COMPLETE ✅
- [x] Update includes to use NEW math library
- [x] Replace BigInt with Abacus in conversion functions
- [x] Update NTT context initialization
- [x] Update array allocations to use Abacus
- [x] Complete main computation loop migration
- [x] Update cleanup code
- [x] Update Makefile to include NEW math library
- [x] Test compilation - SUCCESS

**Status:** COMPLETE - File fully migrated and compiles successfully!

### Phase 3: Migrate Lattice Embeddings - IN PROGRESS
- [x] Create new file `lattice_embeddings_abacus.c`
- [x] Replace BigFixed with Abacus
- [x] Update function signatures in header
- [x] Update Makefile
- [ ] Fix type conflicts between OLD and NEW libraries
- [ ] Test and verify

**Issue:** Type conflicts between OLD crystalline library and NEW math library
- PrimeModular defined in both clock_lattice.h and math/types.h
- SphereCoord defined in both clock_lattice.h and math/types.h
- Need to resolve these conflicts before compilation succeeds

### Phase 4: Migrate Loss Functions
- [ ] Update `algorithms/src/loss_functions_bigfixed.c`
- [ ] Replace BigFixed with Abacus
- [ ] Update function signatures
- [ ] Test and verify

### Phase 5: Migrate Math Wrappers
- [ ] Update `algorithms/src/bigfixed_math_wrappers.c`
- [ ] Replace BigFixed with Abacus
- [ ] Update function signatures
- [ ] Test and verify

### Phase 6: Migrate Remaining Files
- [ ] Update `algorithms/src/hierarchical_prime_partitions.c`
- [ ] Update `algorithms/src/lattice_sphere_positions.c`
- [ ] Test and verify

### Phase 7: Update Build System
- [ ] Update algorithms/Makefile to link with NEW math library
- [ ] Remove OLD crystalline library dependencies
- [ ] Verify clean build

### Phase 8: Final Verification
- [ ] All algorithms library files build
- [ ] All tests pass
- [ ] No references to OLD crystalline library
- [ ] Commit changes

## Notes
- OLD crystalline library: crystalline/, src/, include/ (DO NOT MODIFY)
- NEW math library: math/ (USE THIS)
- Replace BigInt → Abacus
- Replace BigFixed → Abacus
- Use NEW math library headers: math/abacus.h, math/ntt.h, etc.