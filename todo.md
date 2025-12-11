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

### File 5: hierarchical_prime_partitions.c - DEFERRED ⏸️
- [ ] Complex file with deep OLD library integration
- [ ] Uses BigInt, CrystalAbacus, LatticePartition extensively
- [ ] Requires structural refactoring, not simple replacement
- [ ] Will be migrated in Phase 2 (after core types migrated)

### File 6: lattice_sphere_positions.c - DEFERRED ⏸️
- [ ] Complex file with deep OLD library integration
- [ ] Uses BigInt, ClockPosition, LatticePartition extensively
- [ ] Requires structural refactoring, not simple replacement
- [ ] Will be migrated in Phase 2 (after core types migrated)

## Phase 1 Complete - Core Math Operations Migrated ✅

### Successfully Migrated (4/6 files)
- ✅ ntt_attention.c - NTT operations with Abacus
- ✅ lattice_embeddings_bigfixed.c - Embedding generation with Abacus
- ✅ loss_functions_bigfixed.c - Loss computation with Abacus
- ✅ bigfixed_math_wrappers.c - Math wrappers with Abacus

### Deferred to Phase 2 (2/6 files)
- ⏸️ hierarchical_prime_partitions.c - Requires type migration first
- ⏸️ lattice_sphere_positions.c - Requires type migration first

### Why Deferred?
The remaining 2 files are deeply integrated with OLD library types:
- BigInt arrays and operations
- CrystalAbacus structures
- LatticePartition structures
- ClockPosition structures

These require migrating the core type definitions first, which is a larger architectural change.

## Phase 2 Plan - Type Migration
1. Migrate core types (BigInt → Abacus, CrystalAbacus → Abacus)
2. Migrate LatticePartition to use NEW math library
3. Migrate ClockPosition to use NEW math library
4. Then migrate the 2 deferred files
5. Remove ALL OLD crystalline library references

## Current Status
- ✅ 4/6 core math operation files migrated
- ✅ All migrated files compile successfully
- ✅ NEW math library fully integrated for core operations
- ⏸️ 2 files deferred pending type migration