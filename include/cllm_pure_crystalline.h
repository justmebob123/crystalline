/*
 * Pure Crystalline CLLM - Core Types and Structures
 * 
 * PURE implementation using Babylonian arbitrary precision mathematics.
 * ALL internal calculations use BigInt/BigFixed.
 * Floats ONLY for input/output conversion.
 */

#ifndef CLLM_PURE_CRYSTALLINE_H
#define CLLM_PURE_CRYSTALLINE_H

#include "bigint_core.h"
#include "bigfixed_core.h"
#include "prime_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRIME_FACTORS 16
#define MAX_NEIGHBORS 12
#define MAX_DERIVED_TOKENS 64

/*
 * CrystallineToken - Pure Prime-Based Token Representation
 */
typedef struct {
    uint32_t token_id;
    char token_str[64];
    uint64_t prime;
    uint64_t prime_factors[MAX_PRIME_FACTORS];
    uint8_t num_factors;
    bool is_root;
    BigFixed lattice_coords[3];
    uint32_t neighbors[MAX_NEIGHBORS];
    uint8_t num_neighbors;
    uint32_t root_token_id;
    uint64_t usage_count;
    double root_score;
} CrystallineToken;

/*
 * Prime Operations
 */
bool crystalline_is_prime(uint64_t n);
uint64_t crystalline_get_nth_prime(uint32_t n);
void crystalline_factorize(uint64_t number, uint64_t* factors, uint8_t* num_factors);

/*
 * Token Operations
 */
CrystallineToken* crystalline_token_create(uint32_t token_id, const char* token_str, uint64_t prime);
void crystalline_token_free(CrystallineToken* token);
void crystalline_compute_ulam_position(uint64_t prime, BigFixed coords[3], int precision);

/*
 * Lattice Operations
 */
void crystalline_lattice_distance(const BigFixed pos1[3], const BigFixed pos2[3], BigFixed* distance);
void crystalline_prime_similarity(uint64_t prime1, uint64_t prime2, BigFixed* similarity);
void crystalline_phase_alignment(uint64_t prime1, uint64_t prime2, BigFixed* alignment);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_PURE_CRYSTALLINE_H */
