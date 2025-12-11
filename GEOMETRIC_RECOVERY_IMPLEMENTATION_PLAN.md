# Geometric Recovery Implementation Plan

## Status: IN PROGRESS

This document tracks the implementation of the complete geometric recovery system with all required components.

## Components Implemented ✅

### 1. Core Structures (geometric_recovery.h) ✅
- OscillationVector - directional tracking
- QuadrantPolarity - mirrored sudoku folding
- SharedVertex/SharedFace - shared geometry
- TorusIntersectionCurve - curve intersections
- FractalPartition - partition bounds
- TetrationTower - attractors
- ScaleLevel - multi-scale search
- SFTParams - systematic generation
- GeometricRecoveryContext - complete system

### 2. Oscillation Vector (oscillation_vector.c) ✅
- compute_oscillation_vector() - uses anchors as gradient
- determine_quadrant() - identifies quadrant
- Tracks distance, direction, velocity, polarity
- Determines if moving toward or away from target

### 3. Quadrant Polarity (quadrant_polarity.c) ✅
- create_quadrant_polarity() - creates polarity system
- apply_quadrant_polarity() - applies corrections
- fold_to_first_quadrant() - mirrored sudoku folding
- Handles Möbius folding at 2π

### 4. Shared Geometry (shared_geometry.c) ✅
- find_shared_vertices() - vertices with multiple k
- find_shared_faces() - faces containing shared vertices
- Implements "all k share a face or vertex" constraint

## Components To Implement 🔄

### 5. Torus Intersection Curves (torus_curves.c)
- [ ] find_torus_intersection_curve() - compute curves
- [ ] sample_torus_orbit() - sample along orbit
- [ ] search_torus_orbit() - search 2^40 points
- [ ] Curves pass through vertices and lie on faces

### 6. Fractal Partition (fractal_partition.c)
- [ ] compute_fractal_partition() - compute bounds
- [ ] Determine min/max dimensions
- [ ] Calculate partition size
- [ ] Provide confidence intervals

### 7. Tetration Attractors (tetration_attractors.c)
- [ ] create_tetration_towers() - 186 towers (6 bases × 31 depths)
- [ ] compute_tetration_score() - attractor strength
- [ ] bias_toward_attractor() - bias position
- [ ] Use scores to guide search

### 8. Multi-Scale Fractal Search (fractal_search.c)
- [ ] multi_scale_fractal_search() - search at multiple scales
- [ ] Recursive search at 1.0, 0.5, 0.25, ... scales
- [ ] Self-similar structure at every layer

### 9. SFT Integration (sft_integration.c)
- [ ] sft_generate_candidate() - systematic generation
- [ ] sft_systematic_search() - nonce-based search
- [ ] Combine with tetration bias

### 10. Q-Based Validation (q_validation.c)
- [ ] validate_candidate_k() - compute Q and measure distance
- [ ] measure_ec_point_distance() - EC point distance
- [ ] Enforce elliptic curve constraint

### 11. Utility Functions (geometric_utils.c)
- [ ] map_k_to_position() - k → position (257+ bits)
- [ ] map_position_to_k() - position → k (+1 bit extension)
- [ ] compute_k_hamming_distance() - Hamming distance

### 12. Main Recovery System (geometric_recovery_core.c)
- [ ] geometric_recovery_create() - create context
- [ ] geometric_recovery_add_anchor() - add anchors
- [ ] geometric_recovery_initialize() - detect tori, find geometry
- [ ] geometric_recovery_recover_k() - MAIN RECOVERY FUNCTION
- [ ] geometric_recovery_free() - cleanup

## Implementation Strategy

### Phase 1: Core Utilities (NEXT)
1. Implement geometric_utils.c (k↔position mapping with +1 bit)
2. Implement q_validation.c (Q-based validation)
3. Test basic k→Q→validation cycle

### Phase 2: Geometric Constraints
4. Implement torus_curves.c (intersection curves)
5. Implement fractal_partition.c (partition bounds)
6. Test shared geometry + curves

### Phase 3: Search Components
7. Implement tetration_attractors.c (186 towers)
8. Implement fractal_search.c (multi-scale)
9. Implement sft_integration.c (systematic)
10. Test complete search system

### Phase 4: Integration
11. Implement geometric_recovery_core.c (main system)
12. Create test program
13. Run full recovery test
14. Measure results

## Expected Results

### With All Components:
- **Directional tracking**: Know which way to adjust
- **Shared geometry**: Massive search space pruning
- **Torus curves**: Search along correct paths
- **Quadrant folding**: Reduce to Q1 only
- **+1 bit extension**: Handle boundaries correctly
- **Multi-scale**: Find at correct fractal level
- **Tetration bias**: Guide toward attractors
- **SFT**: Systematic exploration
- **Q validation**: Enforce EC constraint

### Target Performance:
- Phase 1 (52D, 10K anchors): **30-50% recovery** (up from 0%)
- Phase 2 (256D, 1M anchors): **80-95% recovery**
- Phase 3 (512D, 100M anchors): **99%+ recovery**

## Files Structure

```
algorithms/
├── include/
│   └── geometric_recovery.h ✅
├── src/
│   └── geometric_recovery/
│       ├── oscillation_vector.c ✅
│       ├── quadrant_polarity.c ✅
│       ├── shared_geometry.c ✅
│       ├── torus_curves.c 🔄
│       ├── fractal_partition.c 🔄
│       ├── tetration_attractors.c 🔄
│       ├── fractal_search.c 🔄
│       ├── sft_integration.c 🔄
│       ├── q_validation.c 🔄
│       ├── geometric_utils.c 🔄
│       └── geometric_recovery_core.c 🔄
└── tests/
    └── test_geometric_recovery.c 🔄
```

## Next Immediate Action

Implement geometric_utils.c and q_validation.c to establish the basic k↔Q validation cycle, then build up the geometric constraints.