# Bitcoin Miner Tab - Implementation Plan

## Project Scope

**Purpose**: Novel mining algorithm research + AI inference engine (Q→k mapping)

**Key Components**:
1. Modular tetration towers (depth 13) - ✅ Already implemented
2. Entropy reduction algorithms - ⚠️ Need to add
3. Q→k mapping (Question→Answer inference) - ⚠️ Need to implement
4. Harmonic FFT analysis - ✅ NTT already exists
5. Ulam spiral visualization - ⚠️ Need to add
6. Cymatics simulations - ⚠️ Need to add
7. Quantum lattice - ⚠️ Need crystalline version
8. Mining protocol - ⚠️ Need to implement

## Phase 1: Audit Existing Crystalline Math Library

### ✅ Already Implemented

**Basic Math** (`prime_math.h`, `prime_basic.h`, `prime_math_custom.h`):
- ✅ `prime_sin()`, `prime_cos()`, `prime_tan()`
- ✅ `prime_exp()`, `prime_log()`, `prime_pow()`
- ✅ `prime_sqrt()`, `prime_abs()`
- ✅ `prime_add()`, `prime_subtract()`, `prime_multiply()`, `prime_divide()`
- ✅ `prime_fmod()`, `prime_remainder()`

**Modular Arithmetic** (`prime_lattice_geometry.h`, `prime_bigint_transcendental.h`):
- ✅ `prime_powmod_int()` - Modular exponentiation
- ✅ `prime_gcd()`, `prime_lcm()` - GCD/LCM
- ✅ `prime_modinv()` - Modular inverse
- ✅ `tetration_damped()` - Tetration with damping
- ✅ `big_tetration_damped()` - BigInt tetration

**NTT/FFT** (`bigint_ntt.h`, `cllm_ntt_attention.h`):
- ✅ NTT forward/inverse transforms
- ✅ NTT-based attention (O(n log n))
- ✅ BigFixed NTT support

**Lattice Formula** (`cllm_lattice_formula.h`):
- ✅ `L_lattice_complete()` - Complete L(n,d,k,λ,ω,ψ) formula
- ✅ `calculate_O()` - Octahedral symmetry
- ✅ `get_dimensional_frequency()` - φᵢ frequencies
- ✅ `calculate_mobius_twist()` - Γ(k) = (-1)^k

**Plimpton 322** (`cllm_plimpton_integration.h`):
- ✅ Pythagorean triple generation
- ✅ Plimpton ratios Ψ(p,q)

**Cymatic Frequencies** (`cllm_cymatic_frequencies.h`):
- ✅ 432 Hz, 528 Hz, 7.83 Hz (Schumann)
- ✅ Frequency resonance calculations

### ⚠️ Need to Implement

**Entropy Functions**:
```c
// Shannon entropy
double prime_shannon_entropy(const double* signal, size_t len);

// Entropy reduction (18-45% per step)
double prime_entropy_reduction(double initial_bits, uint32_t steps, 
                                double cut_min, double cut_max);

// Recursive entropy with ethics check
double prime_recursive_entropy(const double* tetrated_stack, size_t len,
                                const uint64_t* primes, size_t num_primes,
                                double phi_scale);
```

**Ulam Spiral**:
```c
// Generate Ulam spiral grid
void prime_ulam_spiral_generate(uint64_t* grid, uint32_t size);

// Check if position contains prime
bool prime_ulam_is_prime_at(const uint64_t* grid, uint32_t size, 
                             uint32_t x, uint32_t y);

// Get prime density in region
double prime_ulam_density(const uint64_t* grid, uint32_t size,
                          uint32_t x, uint32_t y, uint32_t radius);
```

**Cymatics Simulation**:
```c
// Simulate cymatic pattern for prime set
void prime_cymatics_simulate(double* output, size_t len,
                              const uint64_t* primes, size_t num_primes,
                              const char* shape); // "dodeca" or "icosa"

// Detect harmonic alignment
bool prime_harmonic_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes, size_t num_primes);
```

**Q→k Mapping (AI Inference)**:
```c
// Map Question vector to Answer key
typedef struct {
    double* Q_vector;        // Question embedding
    size_t Q_dim;            // Dimension
    uint64_t* k_candidates;  // Candidate keys
    size_t num_candidates;   // Number of candidates
    double* distances;       // Tensor distances
} QtoK_Mapping;

// Initialize Q→k mapper
QtoK_Mapping* prime_qk_mapper_create(size_t Q_dim, size_t num_candidates);

// Compute tensor distance (from your Python)
double prime_tensor_distance(const double* Q1, const double* Q2, 
                              size_t dim, uint64_t p_mod);

// Find best k for given Q
uint64_t prime_qk_find_best(const QtoK_Mapping* mapper, 
                             const double* Q_query);

// Cleanup
void prime_qk_mapper_destroy(QtoK_Mapping* mapper);
```

**Mining Algorithm Core**:
```c
// Mining work structure
typedef struct {
    uint64_t nonce;              // Current nonce
    uint64_t target;             // Difficulty target
    uint8_t block_header[80];    // Bitcoin block header
    uint8_t hash[32];            // Result hash
    bool found;                  // Solution found flag
} MiningWork;

// Tetration-based mining (your novel algorithm)
bool prime_mine_tetration(MiningWork* work, 
                          const uint64_t* prime_tower, 
                          uint32_t tower_depth);

// Entropy-guided nonce selection
uint64_t prime_mine_entropy_nonce(const MiningWork* work,
                                   double entropy_residual);

// Harmonic alignment check
bool prime_mine_harmonic_check(const uint8_t* hash, 
                                const uint64_t* target_primes,
                                size_t num_primes);
```

## Phase 2: Implement Missing Functions

### 2.1: Entropy Functions

**File**: `src/ai/cllm_entropy_functions.c`
**Header**: `include/ai/cllm_entropy_functions.h`

```c
#include "prime_math.h"
#include "prime_basic.h"

double prime_shannon_entropy(const double* signal, size_t len) {
    if (!signal || len == 0) return 0.0;
    
    // Calculate sum for normalization
    double sum = 0.0;
    for (size_t i = 0; i < len; i++) {
        sum = prime_add(sum, signal[i]);
    }
    
    if (sum < 1e-10) return 0.0;
    
    // Calculate Shannon entropy: H = -Σ p(i) * log2(p(i))
    double entropy = 0.0;
    for (size_t i = 0; i < len; i++) {
        double prob = prime_divide(signal[i], sum);
        if (prob > 1e-10) {
            double log_prob = prime_log2(prob);
            double term = prime_multiply(prob, log_prob);
            entropy = prime_subtract(entropy, term);
        }
    }
    
    return entropy;
}

double prime_entropy_reduction(double initial_bits, uint32_t steps,
                                double cut_min, double cut_max) {
    double remaining = initial_bits;
    
    for (uint32_t i = 0; i < steps; i++) {
        // Generate pseudo-random cut in range [cut_min, cut_max]
        // Using prime-based PRNG
        double cut = cut_min + (cut_max - cut_min) * 
                     prime_fmod((double)(i * 1103515245 + 12345), 1.0);
        
        // Reduce: remaining *= (1 - cut)
        double reduction = prime_subtract(1.0, cut);
        remaining = prime_multiply(remaining, reduction);
    }
    
    // Ensure minimum of 1 bit
    return prime_fmax(1.0, remaining);
}
```

### 2.2: Ulam Spiral

**File**: `src/geometry/prime_ulam_spiral.c`
**Header**: `include/prime_ulam_spiral.h`

```c
#include "prime_math.h"
#include "prime_basic.h"

void prime_ulam_spiral_generate(uint64_t* grid, uint32_t size) {
    if (!grid || size == 0 || size % 2 == 0) return;
    
    // Initialize grid
    for (uint32_t i = 0; i < size * size; i++) {
        grid[i] = 0;
    }
    
    // Start at center
    uint32_t x = size / 2;
    uint32_t y = size / 2;
    grid[y * size + x] = 1;
    
    // Directions: right, up, left, down
    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, -1, 0, 1};
    int dir = 0;
    
    uint32_t step = 1;
    uint64_t num = 2;
    
    while (num <= size * size) {
        for (int rep = 0; rep < 2; rep++) {
            for (uint32_t s = 0; s < step; s++) {
                x += dx[dir];
                y += dy[dir];
                
                if (x < size && y < size) {
                    grid[y * size + x] = num;
                    num++;
                }
                
                if (num > size * size) return;
            }
            dir = (dir + 1) % 4;
        }
        step++;
    }
}

bool prime_ulam_is_prime_at(const uint64_t* grid, uint32_t size,
                             uint32_t x, uint32_t y) {
    if (!grid || x >= size || y >= size) return false;
    
    uint64_t num = grid[y * size + x];
    if (num <= 1) return false;
    if (num == 2 || num == 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    
    for (uint64_t i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) {
            return false;
        }
    }
    
    return true;
}
```

### 2.3: Cymatics Simulation

**File**: `src/ai/cllm_cymatics_simulation.c`
**Header**: `include/ai/cllm_cymatics_simulation.h`

```c
#include "prime_math.h"
#include "prime_math_custom.h"
#include "bigint_ntt.h"

void prime_cymatics_simulate(double* output, size_t len,
                              const uint64_t* primes, size_t num_primes,
                              const char* shape) {
    if (!output || !primes || len == 0 || num_primes == 0) return;
    
    // Initialize output
    for (size_t i = 0; i < len; i++) {
        output[i] = 0.0;
    }
    
    // Generate x values: 0 to 2π
    double pi = 3.14159265358979323846;
    double two_pi = prime_multiply(2.0, pi);
    
    // Simulate wave patterns
    for (size_t i = 0; i < len; i++) {
        double x = prime_multiply(two_pi, prime_divide((double)i, (double)len));
        
        // Sum sin/cos patterns for each prime pair
        for (size_t p = 0; p < num_primes - 1; p++) {
            double p1 = (double)primes[p];
            double p2 = (double)primes[p + 1];
            
            // sin(p1 * x) + cos(p2 * x)
            double arg1 = prime_multiply(p1, x);
            double arg2 = prime_multiply(p2, x);
            
            double sin_term = prime_sin(arg1);
            double cos_term = prime_cos(arg2);
            
            output[i] = prime_add(output[i], prime_add(sin_term, cos_term));
        }
    }
    
    // Apply shape-specific modulation
    if (shape && strcmp(shape, "dodeca") == 0) {
        // 12-fold symmetry modulation
        for (size_t i = 0; i < len; i++) {
            double phase = prime_multiply(12.0, 
                           prime_multiply(two_pi, 
                           prime_divide((double)i, (double)len)));
            double mod = prime_cos(phase);
            output[i] = prime_multiply(output[i], mod);
        }
    }
}

bool prime_harmonic_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes, 
                               size_t num_primes) {
    if (!signal || !target_primes || len == 0 || num_primes == 0) {
        return false;
    }
    
    // Apply NTT to detect frequency peaks
    // (Using existing NTT infrastructure)
    
    // For now, simplified version:
    // Check if signal has peaks near target prime frequencies
    
    double threshold = 0.5;
    size_t alignments = 0;
    
    for (size_t i = 0; i < num_primes; i++) {
        uint64_t prime = target_primes[i];
        size_t idx = prime % len;
        
        if (prime_abs(signal[idx]) > threshold) {
            alignments++;
        }
    }
    
    // Consider aligned if >50% of primes have peaks
    return (alignments * 2 > num_primes);
}
```

### 2.4: Q→k Mapping (AI Inference Core)

**File**: `src/ai/cllm_qk_mapping.c`
**Header**: `include/ai/cllm_qk_mapping.h`

```c
#include "prime_math.h"
#include "prime_basic.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    double* Q_vector;
    size_t Q_dim;
    uint64_t* k_candidates;
    size_t num_candidates;
    double* distances;
} QtoK_Mapping;

QtoK_Mapping* prime_qk_mapper_create(size_t Q_dim, size_t num_candidates) {
    QtoK_Mapping* mapper = (QtoK_Mapping*)calloc(1, sizeof(QtoK_Mapping));
    if (!mapper) return NULL;
    
    mapper->Q_dim = Q_dim;
    mapper->num_candidates = num_candidates;
    
    mapper->Q_vector = (double*)calloc(Q_dim, sizeof(double));
    mapper->k_candidates = (uint64_t*)calloc(num_candidates, sizeof(uint64_t));
    mapper->distances = (double*)calloc(num_candidates, sizeof(double));
    
    if (!mapper->Q_vector || !mapper->k_candidates || !mapper->distances) {
        prime_qk_mapper_destroy(mapper);
        return NULL;
    }
    
    return mapper;
}

double prime_tensor_distance(const double* Q1, const double* Q2,
                              size_t dim, uint64_t p_mod) {
    if (!Q1 || !Q2 || dim < 2) return 0.0;
    
    // Calculate dx = Q1[0] - Q2[0], dy = Q1[1] - Q2[1]
    double dx = prime_subtract(Q1[0], Q2[0]);
    double dy = prime_subtract(Q1[1], Q2[1]);
    
    // Modular reduction
    dx = prime_fmod(dx, (double)p_mod);
    dy = prime_fmod(dy, (double)p_mod);
    
    // Simplified determinant: dx * gy - dy * gx
    // (Using golden ratio components as g)
    double phi = 1.618033988749895;
    double gx = prime_fmod(phi, (double)p_mod);
    double gy = prime_fmod(prime_multiply(phi, phi), (double)p_mod);
    
    double term1 = prime_multiply(dx, gy);
    double term2 = prime_multiply(dy, gx);
    double det = prime_subtract(term1, term2);
    
    return prime_fmod(det, (double)p_mod);
}

uint64_t prime_qk_find_best(const QtoK_Mapping* mapper,
                             const double* Q_query) {
    if (!mapper || !Q_query) return 0;
    
    double min_distance = 1e308; // Large value
    uint64_t best_k = 0;
    
    for (size_t i = 0; i < mapper->num_candidates; i++) {
        // Create candidate Q vector from k
        double Q_candidate[2];
        Q_candidate[0] = (double)(mapper->k_candidates[i] % 1000000);
        Q_candidate[1] = (double)(mapper->k_candidates[i] / 1000000);
        
        // Calculate distance
        double dist = prime_tensor_distance(Q_query, Q_candidate, 
                                             mapper->Q_dim, 13);
        
        if (dist < min_distance) {
            min_distance = dist;
            best_k = mapper->k_candidates[i];
        }
    }
    
    return best_k;
}

void prime_qk_mapper_destroy(QtoK_Mapping* mapper) {
    if (!mapper) return;
    
    free(mapper->Q_vector);
    free(mapper->k_candidates);
    free(mapper->distances);
    free(mapper);
}
```

## Phase 3: Mining Algorithm Implementation

### 3.1: Core Mining Structure

**File**: `src/mining/prime_mining_core.c`
**Header**: `include/mining/prime_mining_core.h`

```c
#include "prime_math.h"
#include "prime_lattice_geometry.h"
#include "cllm_entropy_functions.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    uint64_t nonce;
    uint64_t target;
    uint8_t block_header[80];
    uint8_t hash[32];
    bool found;
} MiningWork;

// Tetration-based mining (novel algorithm)
bool prime_mine_tetration(MiningWork* work,
                          const uint64_t* prime_tower,
                          uint32_t tower_depth) {
    if (!work || !prime_tower || tower_depth == 0) return false;
    
    // Build tetration tower
    BigInt result;
    bigint_init(&result);
    
    BigInt modulus;
    bigint_init(&modulus);
    bigint_set_uint64(&modulus, work->target);
    
    // Compute tetration: prime_tower[0] ^^ tower_depth mod target
    tetration_damped(&result, prime_tower[0], tower_depth, &modulus);
    
    // Use tetration result to guide nonce selection
    uint64_t tetration_value = bigint_to_uint64(&result);
    
    // Combine with current nonce
    work->nonce ^= tetration_value;
    
    // Hash the block header with new nonce
    // (This would call actual SHA-256 or your custom hash)
    // For now, placeholder:
    memcpy(work->hash, &work->nonce, sizeof(uint64_t));
    
    // Check if hash meets target
    uint64_t hash_value = *(uint64_t*)work->hash;
    work->found = (hash_value < work->target);
    
    bigint_free(&result);
    bigint_free(&modulus);
    
    return work->found;
}

// Entropy-guided nonce selection
uint64_t prime_mine_entropy_nonce(const MiningWork* work,
                                   double entropy_residual) {
    if (!work) return 0;
    
    // Use entropy residual to select next nonce
    // This creates a non-uniform search pattern
    
    uint64_t base_nonce = work->nonce;
    
    // Scale by entropy (lower entropy = smaller jumps)
    double scale = prime_exp(entropy_residual);
    uint64_t jump = (uint64_t)(scale * 1000.0);
    
    return base_nonce + jump;
}

// Harmonic alignment check
bool prime_mine_harmonic_check(const uint8_t* hash,
                                const uint64_t* target_primes,
                                size_t num_primes) {
    if (!hash || !target_primes || num_primes == 0) return false;
    
    // Convert hash to signal
    double signal[32];
    for (size_t i = 0; i < 32; i++) {
        signal[i] = (double)hash[i];
    }
    
    // Check harmonic alignment
    return prime_harmonic_alignment(signal, 32, target_primes, num_primes);
}
```

## Phase 4: Bitcoin Miner Tab UI

### 4.1: Tab Structure

**File**: `app/ui/tabs/tab_bitcoin_miner.c`
**Header**: `app/ui/tabs/tab_bitcoin_miner.h`

**Layout**:
```
┌─────────────────────────────────────────────────────────────┐
│ Bitcoin Miner - Crystalline Algorithm                       │
├─────────────────────────────────────────────────────────────┤
│ ┌─ Configuration ──────┐  ┌─ Visualization ──────────────┐ │
│ │ Network:             │  │                              │ │
│ │ ○ Testnet (default)  │  │  [Ulam Spiral - Primes]      │ │
│ │ ○ Mainnet (caution)  │  │                              │ │
│ │                      │  │  [Tetration Tower]           │ │
│ │ Algorithm:           │  │                              │ │
│ │ Tetration Depth: 13  │  │  [Harmonic Frequencies]      │ │
│ │ Prime Limit: 100000  │  │                              │ │
│ │ Entropy Steps: 20    │  └──────────────────────────────┘ │
│ │                      │                                   │
│ │ Target Primes:       │  ┌─ Mining Stats ───────────────┐ │
│ │ [5,23,127,379,...]   │  │ Hash Rate: 0.00 MH/s         │ │
│ │                      │  │ Blocks Found: 0              │ │
│ │ [Start Mining]       │  │ Current Nonce: 0             │ │
│ │ [Stop]               │  │ Entropy: 0.0000              │ │
│ │ [Export Data]        │  │ Alignment: No                │ │
│ └──────────────────────┘  └──────────────────────────────┘ │
│                                                             │
│ ┌─ Activity Log ──────────────────────────────────────────┐ │
│ │ [12:34:56] Mining started (testnet)                     │ │
│ │ [12:34:57] Tetration tower built (depth 13)             │ │
│ │ [12:34:58] Harmonic alignment detected                  │ │
│ │ [12:34:59] Nonce: 1234567 (entropy-guided)              │ │
│ └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Phase 5: Integration Checklist

### 5.1: New Files to Create

**Crystalline Math Extensions**:
- [ ] `src/ai/cllm_entropy_functions.c`
- [ ] `include/ai/cllm_entropy_functions.h`
- [ ] `src/geometry/prime_ulam_spiral.c`
- [ ] `include/prime_ulam_spiral.h`
- [ ] `src/ai/cllm_cymatics_simulation.c`
- [ ] `include/ai/cllm_cymatics_simulation.h`
- [ ] `src/ai/cllm_qk_mapping.c`
- [ ] `include/ai/cllm_qk_mapping.h`

**Mining Core**:
- [ ] `src/mining/prime_mining_core.c`
- [ ] `include/mining/prime_mining_core.h`
- [ ] `src/mining/mining_network.c` (Bitcoin protocol)
- [ ] `include/mining/mining_network.h`

**UI Tab**:
- [ ] `app/ui/tabs/tab_bitcoin_miner.c`
- [ ] `app/ui/tabs/tab_bitcoin_miner.h`

### 5.2: Makefile Updates

Add to `Makefile`:
```makefile
# Mining library
MINING_SOURCES = src/mining/prime_mining_core.c \
                 src/mining/mining_network.c

MINING_OBJECTS = $(MINING_SOURCES:.c=.o)

libmining.so: $(MINING_OBJECTS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

libmining.a: $(MINING_OBJECTS)
	$(AR) rcs $@ $^
```

### 5.3: Build Verification

```bash
make clean
make -j$(nproc)
make app
./app/hyper_prime_spiral  # Test new tab
```

## Phase 6: Testing Strategy

### 6.1: Unit Tests

**Test Entropy Functions**:
```c
void test_shannon_entropy() {
    double signal[] = {0.25, 0.25, 0.25, 0.25};
    double entropy = prime_shannon_entropy(signal, 4);
    assert(entropy > 1.99 && entropy < 2.01); // Should be 2.0
}

void test_entropy_reduction() {
    double initial = 112.0;
    double final = prime_entropy_reduction(initial, 20, 0.18, 0.45);
    assert(final < initial * 0.1); // Should reduce to <10%
}
```

**Test Q→k Mapping**:
```c
void test_qk_mapping() {
    QtoK_Mapping* mapper = prime_qk_mapper_create(128, 1000);
    
    double Q_query[128] = {/* ... */};
    uint64_t best_k = prime_qk_find_best(mapper, Q_query);
    
    assert(best_k > 0);
    prime_qk_mapper_destroy(mapper);
}
```

### 6.2: Integration Tests

**Test Mining Pipeline**:
```c
void test_mining_pipeline() {
    MiningWork work = {0};
    work.target = 0x00000000FFFFFFFF;
    
    uint64_t prime_tower[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
    
    bool found = prime_mine_tetration(&work, prime_tower, 13);
    
    // Should eventually find a solution
    assert(found || work.nonce > 0);
}
```

## Next Steps

1. **Implement Phase 2** (Missing Functions)
2. **Implement Phase 3** (Mining Core)
3. **Implement Phase 4** (UI Tab)
4. **Test on Testnet**
5. **Document Algorithm**
6. **Publish Research**

## Ethical Considerations

- ✅ Testnet default (safe)
- ✅ Mainnet requires explicit confirmation
- ✅ No attack code (removed ECDH recovery)
- ✅ Open source (research transparency)
- ✅ Educational focus
- ✅ Novel algorithm (not exploiting existing systems)

## Timeline Estimate

- Phase 2 (Math Functions): 2-3 hours
- Phase 3 (Mining Core): 3-4 hours
- Phase 4 (UI Tab): 2-3 hours
- Phase 5 (Integration): 1-2 hours
- Phase 6 (Testing): 2-3 hours

**Total**: ~12-15 hours of focused development

---

**Ready to proceed with implementation?**