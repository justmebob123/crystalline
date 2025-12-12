# Final Reorganization Plan - Correct Library Placement

## Critical Findings

After comprehensive analysis (Phases 1-10), I've identified:

1. ✅ **Existing infrastructure is EXCELLENT** - Most functionality already exists
2. ❌ **My initial placement was WRONG** - Put general algorithms in AI layer
3. ✅ **Naming conventions are CONSISTENT** - Should follow existing patterns
4. ✅ **Constants are well-defined** - PHI, PI already exist
5. ⚠️ **Some functions need to EXTEND existing modules**, not create new ones

---

## Reorganization Strategy

### Strategy A: Extend Existing Modules (PREFERRED)

**Rationale**: Your codebase already has well-structured modules. Don't create redundancy.

**Actions**:
1. Add entropy functions to existing `algorithms/src/statistics.c`
2. Add cymatics functions to existing `algorithms/src/cymatic_modulation.c`
3. Create NEW `algorithms/src/tensor_ops.c` (doesn't exist yet)
4. Keep Ulam spiral in `src/geometry/` (correct placement)
5. Refactor Q→k mapping to use algorithms layer functions

### Strategy B: Create Separate Modules (ALTERNATIVE)

**Rationale**: Keep new functionality isolated for clarity.

**Actions**:
1. Create `algorithms/src/entropy.c` (separate from statistics)
2. Create `algorithms/src/signal_processing.c` (separate from cymatics)
3. Create `algorithms/src/tensor_ops.c`

**RECOMMENDATION**: Use Strategy A (extend existing) for better integration.

---

## Detailed Reorganization Plan

### STEP 1: Extend algorithms/statistics.c

**Add to `algorithms/include/statistics.h`**:
```c
/**
 * @brief Calculate Shannon entropy
 * 
 * Computes H = -Σ p(i) * log₂(p(i))
 * 
 * @param signal Input signal (will be normalized to probabilities)
 * @param size Signal length
 * @return Shannon entropy in bits
 */
double stats_shannon_entropy(const double* signal, size_t size);

/**
 * @brief Recursive entropy reduction
 * 
 * Reduces search space entropy by applying random cuts.
 * Used for search space reduction in optimization and cryptography.
 * 
 * @param initial_bits Initial entropy in bits
 * @param steps Number of reduction steps
 * @param cut_min Minimum cut fraction (0.0 to 1.0)
 * @param cut_max Maximum cut fraction (0.0 to 1.0)
 * @return Reduced entropy in bits
 */
double stats_entropy_reduction(double initial_bits, uint32_t steps,
                                double cut_min, double cut_max);

/**
 * @brief Compute modular probability distribution
 * 
 * Calculates: p[i] = (value % prime[i]) / prime[i]
 * 
 * @param value Input value
 * @param primes Array of primes
 * @param num_primes Number of primes
 * @param prob_dist Output probability distribution (pre-allocated)
 * @return Sum of probabilities
 */
double stats_modular_probability(uint64_t value, const uint64_t* primes,
                                  size_t num_primes, double* prob_dist);

/**
 * @brief Compute entropy residuals from layered data
 * 
 * Calculates layer-wise entropy and folds into golden ratio bounds.
 * 
 * @param layers Array of layer values
 * @param num_layers Number of layers
 * @param primes Array of primes for modular calculations
 * @param num_primes Number of primes
 * @param phi_scale Golden ratio scale factor
 * @return Entropy residual
 */
double stats_entropy_residuals(const uint64_t* layers, size_t num_layers,
                                const uint64_t* primes, size_t num_primes,
                                double phi_scale);
```

**Implementation**: Copy from my `cllm_entropy_functions.c`, rename functions

---

### STEP 2: Extend algorithms/cymatic_modulation.c

**Add to `algorithms/include/cymatic_modulation.h`**:
```c
/**
 * @brief Simulate cymatic wave pattern
 * 
 * Generates wave pattern: Σ [sin(p[i] * x) + cos(p[i+1] * x)]
 * 
 * @param output Output signal (pre-allocated)
 * @param len Signal length
 * @param primes Prime frequencies
 * @param num_primes Number of primes
 * @param shape_symmetry Geometric symmetry (12=dodeca, 20=icosa, 6=cube)
 */
void cymatic_simulate_wave(double* output, size_t len,
                           const uint64_t* primes, size_t num_primes,
                           uint32_t shape_symmetry);

/**
 * @brief Detect harmonic alignment
 * 
 * Checks if signal has frequency peaks aligned with target primes.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param target_primes Target prime frequencies
 * @param num_primes Number of target primes
 * @return true if >50% of primes show alignment
 */
bool cymatic_detect_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes, size_t num_primes);

/**
 * @brief Calculate resonance score
 * 
 * Measures signal strength at prime frequencies.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param primes Prime frequencies
 * @param num_primes Number of primes
 * @return Resonance score (0.0 to 1.0)
 */
double cymatic_resonance_score(const double* signal, size_t len,
                                const uint64_t* primes, size_t num_primes);

/**
 * @brief Find frequency peaks
 * 
 * Detects local maxima in signal.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param peak_indices Output peak indices (pre-allocated)
 * @param max_peaks Maximum peaks to find
 * @param threshold Minimum peak height
 * @return Number of peaks found
 */
size_t cymatic_find_peaks(const double* signal, size_t len,
                          size_t* peak_indices, size_t max_peaks,
                          double threshold);

/**
 * @brief Apply geometric modulation
 * 
 * Modulates signal based on geometric symmetry.
 * 
 * @param signal Signal to modulate (modified in place)
 * @param len Signal length
 * @param shape_symmetry Symmetry order (12, 20, 6, etc.)
 */
void cymatic_geometric_modulation(double* signal, size_t len, uint32_t shape_symmetry);
```

**Implementation**: Copy from my `cllm_cymatics_simulation.c`, rename functions

---

### STEP 3: Create algorithms/tensor_ops.c (NEW MODULE)

**Create `algorithms/include/tensor_ops.h`**:
```c
/**
 * @file tensor_ops.h
 * @brief General tensor operations for multi-dimensional data
 * 
 * Provides tensor distance, product, and manipulation functions
 * using crystalline mathematics.
 */

#ifndef TENSOR_OPS_H
#define TENSOR_OPS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate tensor distance with golden ratio geometry
 * 
 * For 2D: det(t1 - t2) = (dx * gy - dy * gx) mod p
 * where g = (φ, φ²) is golden ratio geometry
 * 
 * For N-D: Generalized determinant calculation
 * 
 * @param t1 First tensor
 * @param t2 Second tensor
 * @param dim Tensor dimension
 * @param p_mod Modulus for calculation
 * @return Tensor distance
 */
double tensor_distance(const double* t1, const double* t2,
                       size_t dim, uint64_t p_mod);

/**
 * @brief Generate random anchor tensors
 * 
 * Creates anchor points using golden ratio scaling.
 * 
 * @param anchors Output anchors (pre-allocated: num_anchors * dim)
 * @param num_anchors Number of anchors
 * @param dim Tensor dimension
 * @param seed Random seed
 */
void tensor_generate_anchors(double* anchors, size_t num_anchors,
                              size_t dim, uint64_t seed);

/**
 * @brief Estimate value from anchor triangulation
 * 
 * Uses distances to anchors to estimate target value.
 * 
 * @param query Query tensor
 * @param anchors Anchor tensors (num_anchors * dim)
 * @param num_anchors Number of anchors
 * @param dim Tensor dimension
 * @param p_mod Modulus
 * @return Estimated value
 */
uint64_t tensor_estimate_from_anchors(const double* query,
                                       const double* anchors,
                                       size_t num_anchors,
                                       size_t dim,
                                       uint64_t p_mod);

/**
 * @brief Tensor outer product
 * 
 * Computes: result[i,j] = a[i] * b[j]
 * 
 * @param a First vector
 * @param a_len Length of a
 * @param b Second vector
 * @param b_len Length of b
 * @param result Output tensor (pre-allocated: a_len * b_len)
 */
void tensor_outer_product(const double* a, size_t a_len,
                          const double* b, size_t b_len,
                          double* result);

#ifdef __cplusplus
}
#endif

#endif /* TENSOR_OPS_H */
```

---

### STEP 4: Refactor cllm_qk_mapping.c

**Update `include/ai/cllm_qk_mapping.h`**:
```c
// Remove tensor_distance, generate_anchors, estimate_from_anchors
// These are now in algorithms/tensor_ops.h

// Keep only AI-specific functions:
QtoK_Mapping* cllm_qk_mapper_create(size_t Q_dim, size_t num_candidates);
void cllm_qk_mapper_destroy(QtoK_Mapping* mapper);
uint64_t cllm_qk_find_best(const QtoK_Mapping* mapper, const double* Q_query);
size_t cllm_qk_find_top_n(const QtoK_Mapping* mapper, const double* Q_query,
                           uint64_t* top_k, size_t N);
bool cllm_qk_set_candidates(QtoK_Mapping* mapper, const uint64_t* candidates,
                             size_t num_candidates);
```

**Update `src/ai/cllm_qk_mapping.c`**:
```c
#include "ai/cllm_qk_mapping.h"
#include "tensor_ops.h"  // Use algorithms layer functions

// Implementations now call tensor_distance() from algorithms layer
```

---

## Summary of Changes

### Files to DELETE:
1. `include/ai/cllm_entropy_functions.h`
2. `src/ai/cllm_entropy_functions.c`
3. `include/ai/cllm_cymatics_simulation.h`
4. `src/ai/cllm_cymatics_simulation.c`

### Files to EXTEND:
1. `algorithms/include/statistics.h` - Add 4 entropy functions
2. `algorithms/src/statistics.c` - Add implementations
3. `algorithms/include/cymatic_modulation.h` - Add 5 cymatics functions
4. `algorithms/src/cymatic_modulation.c` - Add implementations

### Files to CREATE:
1. `algorithms/include/tensor_ops.h` - New tensor operations module
2. `algorithms/src/tensor_ops.c` - Implementations

### Files to REFACTOR:
1. `include/ai/cllm_qk_mapping.h` - Remove general functions, keep AI-specific
2. `src/ai/cllm_qk_mapping.c` - Use algorithms layer functions

### Files to KEEP AS-IS:
1. `include/prime_ulam_spiral.h` - ✅ Correct placement
2. `src/geometry/prime_ulam_spiral.c` - ✅ Correct placement

---

## Build Impact

**Before Reorganization**:
- 8 new files
- 1,182 lines of code
- 2 files in wrong layer (ai/ instead of algorithms/)

**After Reorganization**:
- 2 new files (tensor_ops.h/c)
- 4 extended files (statistics, cymatic_modulation)
- 2 kept files (ulam_spiral)
- 2 refactored files (qk_mapping)
- ~1,200 lines properly distributed

**Benefits**:
- ✅ Correct layer separation
- ✅ No redundancy
- ✅ Consistent naming
- ✅ Reusable across all layers
- ✅ Follows existing architecture

---

## Next Step: Execute Reorganization

**Ready to proceed?**

I will:
1. Delete incorrectly placed files
2. Extend existing algorithms modules
3. Create new tensor_ops module
4. Refactor Q→k mapping
5. Update all includes
6. Build and verify
7. Commit changes

**Estimated time**: 1-2 hours

**Proceed with reorganization?**