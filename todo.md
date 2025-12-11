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

### Phase 2: Migrate NTT Attention (PRIORITY) - IN PROGRESS
- [x] Update includes to use NEW math library
- [x] Replace BigInt with Abacus in conversion functions
- [x] Update NTT context initialization
- [x] Update array allocations to use Abacus
- [ ] Complete main computation loop migration
- [ ] Update cleanup code
- [ ] Test and verify

**Status:** Partially migrated - headers updated, conversion functions updated, 
need to complete main computation loop and cleanup.

### Phase 3: Migrate Lattice Embeddings
- [ ] Update `algorithms/src/lattice_embeddings_bigfixed.c`
- [ ] Replace BigFixed with Abacus
- [ ] Update function signatures
- [ ] Test and verify

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