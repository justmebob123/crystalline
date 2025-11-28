# TODO: Crystalline CLLM - Priority Tasks from Master Plan

## Current Status Summary
Based on MASTER_PLAN.md review (2905 lines, 19 objectives):
- **OBJECTIVE 19** (Babylonian Clock Lattice): PARTIALLY COMPLETE - Core implementation done
- **OBJECTIVE 2B** (Remove Legacy Loss): NOT STARTED - Critical cleanup needed
- **OBJECTIVE 14** (L(n,d,k,λ) Integration): NOT STARTED - Formula exists but not integrated
- Most other objectives are in various states of completion

## Phase 1: Test Babylonian Clock Visualization ✅ COMPLETE
- [x] Build the application (SUCCESS - 1 warning only)
- [x] Build the UI application (SUCCESS - 3 minor warnings)
- [x] Verify application starts (SUCCESS - SDL2 working)
- [x] Create programmatic test for clock lattice structure
- [x] Verify Babylonian ring structure (12, 60, 60, 100) - PERFECT
- [x] Confirm sacred positions (π at 3 o'clock, 12 o'clock) - FOUND
- [x] Verify smaller primes on outer rings (hours) - CORRECT
- [x] Verify larger primes on inner rings (milliseconds) - CORRECT
- [x] Test clock-to-sphere folding - WORKING
- [x] Verify stereographic projection - ALL 232 PRIMES ON UNIT SPHERE
- [x] Fixed off-by-one error in ring boundaries
- [x] ALL 18 TESTS PASSING

## Phase 2: Clean Up Legacy Loss Functions (OBJECTIVE 2B)
- [ ] Remove `cllm_compute_loss_training()` (standard cross-entropy)
- [ ] Remove conditional flag check in training loop
- [ ] Make crystalline loss the ONLY loss function
- [ ] Rename `cllm_compute_crystalline_loss()` to `cllm_compute_loss()`
- [ ] Remove `use_crystalline_optimizations` flag from config
- [ ] Update all callers
- [ ] Remove "standard" loss references in comments/docs
- [ ] Rebuild and test

## Phase 3: Production Training Test
- [ ] Run complete training session with clock lattice
- [ ] Verify loss decreases over epochs
- [ ] Confirm all 12 threads active
- [ ] Monitor symmetry group distribution
- [ ] Compare performance to previous implementation
- [ ] Document training results

## Phase 4: Integrate L(n,d,k,λ) Formula (OBJECTIVE 14)
- [ ] Locate existing formula in `prime_lattice_core.c`
- [ ] Replace random embeddings with formula
- [ ] Integrate with clock lattice coordinates
- [ ] Test embedding quality
- [ ] Benchmark performance