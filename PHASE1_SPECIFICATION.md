# Phase 1: Pure Token Representation - Detailed Specification

## OBJECTIVE

Implement pure crystalline token representation using BigFixed coordinates and exact prime-based structure.

## DELIVERABLES

1. Header file: include/cllm_pure_crystalline.h
2. Implementation: src/ai/cllm_pure_token.c
3. Test suite: test_pure_token.c
4. Documentation: This file

## STRUCTURES

### CrystallineToken
```c
typedef struct {
    uint32_t token_id;              // Vocabulary index
    char token_str[64];             // Token string
    uint64_t prime;                 // Prime (if root) or composite
    uint64_t prime_factors[16];     // Factorization
    uint8_t num_factors;            // Number of factors
    bool is_root;                   // True if prime
    BigFixed lattice_coords[3];     // Exact x,y,z position
    uint32_t neighbors[12];         // 12 kissing spheres
    uint8_t num_neighbors;
    uint32_t root_token_id;         // Points to root if composite
    uint64_t usage_count;
    double root_score;
} CrystallineToken;
```

## FUNCTIONS TO IMPLEMENT

### Prime Operations
1. crystalline_is_prime(uint64_t n) -> bool
2. crystalline_get_nth_prime(uint32_t n) -> uint64_t
3. crystalline_factorize(uint64_t number, uint64_t* factors, uint8_t* num_factors)

### Token Operations
4. crystalline_token_create(uint32_t token_id, const char* token_str, uint64_t prime) -> CrystallineToken*
5. crystalline_token_free(CrystallineToken* token)
6. crystalline_compute_ulam_position(uint64_t prime, BigFixed coords[3], int precision)

### Lattice Operations
7. crystalline_lattice_distance(const BigFixed pos1[3], const BigFixed pos2[3], BigFixed* distance)
8. crystalline_prime_similarity(uint64_t prime1, uint64_t prime2, BigFixed* similarity)
9. crystalline_phase_alignment(uint64_t prime1, uint64_t prime2, BigFixed* alignment)

## IMPLEMENTATION DETAILS

### Ulam Spiral Positioning
- Use golden angle: 2π / φ² ≈ 137.5°
- Radius: sqrt(prime_index)
- Angle: golden_angle * prime_index
- x = radius * cos(angle)
- y = radius * sin(angle)
- z = log(prime + 1)
- Convert to BigFixed with 256-bit precision

### Prime Factorization
- Check divisibility by 2
- Check odd numbers up to sqrt(n)
- Store factors in array
- Mark token as root if prime

### Lattice Distance
- Euclidean distance in 3D
- sqrt((x1-x2)² + (y1-y2)² + (z1-z2)²)
- All operations in BigFixed

### Prime Similarity
- Compute GCD(prime1, prime2)
- If GCD = 1: similarity = 0.5 (coprime)
- If GCD > 1: similarity = 1/GCD
- If identical: similarity = 1.0

### Phase Alignment
- phase_diff = 2π * (p1-p2) / (p1+p2)
- alignment = (1 + cos(phase_diff)) / 2
- Range: [0, 1]

## TESTS

### Test 1: Prime Operations
- Test is_prime(17) = true
- Test is_prime(18) = false
- Test get_nth_prime(5) = 11
- Test factorize(30) = [2, 3, 5]

### Test 2: Token Creation
- Create root token (prime 5)
- Create composite token (10 = 2×5)
- Verify factorization
- Verify is_root flag

### Test 3: Ulam Positioning
- Position primes 2,3,5,7,11,13,17,19,23,29
- Verify coordinates are distinct
- Verify BigFixed precision

### Test 4: Lattice Distance
- Compute distance between prime 5 and 7
- Verify distance > 0
- Verify distance to self = 0

### Test 5: Prime Similarity
- Test similarity(5,5) = 1.0
- Test similarity(5,7) = 0.5 (coprime)
- Test similarity(6,10) > 0.5 (share factor 2)

### Test 6: Phase Alignment
- Test alignment(5,5) = 1.0
- Test alignment(5,7) in [0,1]
- Test alignment(2,97) in [0,1]

## SUCCESS CRITERIA

- All tests pass
- No memory leaks
- BigFixed coordinates exact
- Prime operations correct
- Compilation clean (no warnings)

## TIMELINE

Day 1: Implement structures and prime operations
Day 2: Implement token operations and tests

## NEXT PHASE

Phase 2: Pure Embeddings (CrystallineEmbeddings structure)
