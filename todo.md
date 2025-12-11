# Algorithms Library Migration - Replace OLD with NEW Math Library

## CRITICAL PRINCIPLE
**REPLACE files directly, NO suffixes, NO parallel versions**
- Delete OLD implementations
- Write NEW implementations in same file
- Use NEW math library (math/) everywhere
- Clean break from legacy code

## Files to Replace (6 total)

### File 1: ntt_attention.c - COMPLETE ✅
- [x] Replaced BigInt with Abacus
- [x] Uses NEW math library (math/ntt.h, math/abacus.h)
- [x] Compiles successfully
- [x] No OLD library dependencies

### File 2: lattice_embeddings_bigfixed.c - COMPLETE ✅
- [x] Delete OLD implementation
- [x] Write NEW implementation using Abacus
- [x] Update header file
- [x] Test compilation - SUCCESS

### File 3: loss_functions_bigfixed.c - COMPLETE ✅
- [x] Delete OLD implementation
- [x] Write NEW implementation using Abacus
- [x] Update header file
- [x] Test compilation - SUCCESS

### File 4: bigfixed_math_wrappers.c - COMPLETE ✅
- [x] Delete OLD implementation
- [x] Write NEW implementation using Abacus
- [x] Update header file
- [x] Test compilation - SUCCESS

### File 5: hierarchical_prime_partitions.c
- [ ] Delete OLD implementation
- [ ] Write NEW implementation using Abacus
- [ ] Update header file
- [ ] Test compilation

### File 6: lattice_sphere_positions.c
- [ ] Delete OLD implementation
- [ ] Write NEW implementation using Abacus
- [ ] Update header file
- [ ] Test compilation

## Build System Updates
- [ ] Update algorithms/Makefile to use NEW math library only
- [ ] Remove ALL OLD crystalline library references
- [ ] Verify clean build with zero warnings

## Final Verification
- [ ] All 6 files migrated
- [ ] All files compile
- [ ] Zero OLD library dependencies
- [ ] Commit to audit branch