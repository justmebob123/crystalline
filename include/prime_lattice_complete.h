#ifndef PRIME_LATTICE_COMPLETE_H
#define PRIME_LATTICE_COMPLETE_H

/*
 * prime_lattice_complete.h - Auto-generated header file
 * Source: prime_lattice_complete.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "prime_lattice_geometry.h"
#include "prime_lowlevel.h"
#include "prime_math_custom.h"

/* Type definitions */
typedef struct {
    uint64_t a;  // p² - q²
    uint64_t b;  // 2pq
    uint64_t c;  // p² + q²
    int layer;   // n mod 3 (for 3-renewal)
} PythagoreanTriple;

typedef struct {
    double coordinates[12];
    uint64_t prime;
    uint64_t index;
} M12Point;

typedef struct {
    uint64_t residues[15];
    uint64_t value;
} LatticeEmbed15D;

typedef struct {
    ClockPosition clock;        // Basic clock position
    double theta;               // θ(n,k,λ,ω,ψ)
    double r;                   // r(p)
    double O_exp;               // O(n,k,λ)
    double L_value;             // L(n,d,k,λ)
    M12Point m12;               // M₁₂ projection
    LatticeEmbed15D embed;      // 15D embedding
    PythagoreanTriple psi;      // ψ(p,q) triple
} CompleteClockMapping;

typedef struct {
    double L_value;
    const char *artifact;
    const char *description;
} KnownArtifact;

/* Function declarations */
double Z_n_d(uint64_t n, uint64_t d, const char *lambda_phon);
double P_n_d_k(uint64_t n, uint64_t d, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q);
void map_prime_complete(uint64_t prime, uint64_t n, CompleteClockMapping *mapping,
                        const char *lambda_phon);

/* Arbitrary precision lattice formula */
void L_lattice_bigfixed(BigFixed *result, uint64_t n, uint64_t d, int k, 
                        const char *lambda_phon, uint16_t omega, uint64_t p, uint64_t q,
                        int precision_bits);

#endif /* PRIME_LATTICE_COMPLETE_H */