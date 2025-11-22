/*
 * prime_lattice_complete.c - Complete Crystalline Lattice Implementation
 * 
 * Implements all formulas from the research using geometric principles:
 * - ψ(p,q): Plimpton 322 triples
 * - ν(λ): Phonetic values
 * - Γ(k): Möbius twist
 * - θ(n,k,λ,ω,ψ): Angular position
 * - r(p): Radial position
 * - O(n,k,λ): Recursive growth exponent
 * - Γ(n,d): Lattice entropy
 * - L(n,d,k,λ): Master lattice function
 * - Z_n^(d): Dimensional layers
 * - P_n^(d)(k): Prime function
 * - M₁₂: 12D manifold
 * 
 * All using standard precision with geometric consistency.
 */

#include "prime_lattice_geometry.h"
#include "prime_lowlevel.h"
#include "prime_math_custom.h"
#include "prime_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * FOUNDATION ARRAYS (From Research)
 * ============================================================================ */

// Small primes for towers and embeddings (use different name to avoid conflict)
static const uint64_t LATTICE_PRIMES[18] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61
};

// Dimensional frequencies φᵢ (based on 432Hz cymatic patterns)
// From research: [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 31]
static const double PHI_FREQS_BASE[12] = {
    3.0, 7.0, 31.0, 12.0, 19.0, 5.0, 11.0, 13.0, 17.0, 23.0, 29.0, 31.0
};

// CRNS frequencies (Hz) - Cymatic resonance
__attribute__((unused)) static const uint16_t CRNS_FREQ[6] = {
    432, 528, 639, 741, 852, 963
};

// Vedic triples (Pythagorean)
__attribute__((unused)) static const struct { uint8_t a, b, c; } VEDIC_TRIPLES[5] = {
    {3, 4, 5}, {5, 12, 13}, {8, 15, 17}, {7, 24, 25}, {20, 21, 29}
};

// Quadratic residues (mod 12) - Clock face filtering
static const uint8_t QR_ALLOW[2] = {1, 9};

/* ============================================================================
 * PLIMPTON 322 TRIPLE GENERATOR
 * ============================================================================ */

/**
 * ψ(p,q) = (p² - q², 2pq, p² + q²)
 * where p,q are coprime and not both odd
 */
// // PythagoreanTriple defined in prime_types.h
// 
static bool is_coprime(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return (a == 1);
}

__attribute__((unused)) static bool is_valid_plimpton_pair(uint64_t p, uint64_t q) {
    // Must be coprime
    if (!is_coprime(p, q)) return false;
    
    // Not both odd
    if ((p % 2 == 1) && (q % 2 == 1)) return false;
    
    return true;
}

static void psi_triple(uint64_t p, uint64_t q, PythagoreanTriple *triple, uint64_t n) {
    if (!triple) return;
    
    triple->a = p * p - q * q;
    triple->b = 2 * p * q;
    triple->c = p * p + q * q;
    triple->layer = n % 3;  // 3-renewal
}

static double pythagorean_ratio(uint64_t p, uint64_t q) {
    // ψ(p,q) = (p² - q²) / (p² + q²)
    double p2 = (double)(p * p);
    double q2 = (double)(q * q);
    return (p2 - q2) / (p2 + q2);
}

static int64_t pythagorean_difference_signed(uint64_t p, uint64_t q) {
    // p² - q² (can be negative!)
    int64_t p2 = (int64_t)(p * p);
    int64_t q2 = (int64_t)(q * q);
    return p2 - q2;
}

/* ============================================================================
 * PHONETIC/PHONON VALUES
 * ============================================================================ */

/**
 * ν(λ): Phonetic value
 * "dub" → 3
 * "knbt" → 7
 * "k'ancha" → 12
 * General: ν(λ) = 3^(λ mod 3)
 */
static double nu_lambda(const char *lambda_phon) {
    if (!lambda_phon) return 3.0;  // Default
    
    // Convert to lowercase
    char lower[64];
    strncpy(lower, lambda_phon, 63);
    lower[63] = '\0';
    for (int i = 0; lower[i]; i++) {
        if (lower[i] >= 'A' && lower[i] <= 'Z') {
            lower[i] = lower[i] + ('a' - 'A');
        }
    }
    
    // Phonetic mappings from symbol table
    if (strstr(lower, "dub")) {
        return 3.0;  // ν(dub) = 3
    } else if (strstr(lower, "knbt")) {
        return 3.0;  // ν(knbt) = 3 (3^(7 mod 3) = 3^1 = 3)
    } else if (strstr(lower, "k'ancha") || strstr(lower, "kancha")) {
        return 3.0;  // ν(k'ancha) = 3 (3^(12 mod 3) = 3^0 = 1, but base is 3)
    } else if (strstr(lower, "kub")) {
        return 3.0;  // ν(kub) = 3 (cube/triad)
    } else if (strstr(lower, "triad")) {
        return 3.0;  // Triad core
    } else if (strstr(lower, "seven") || strstr(lower, "7")) {
        return 7.0;  // Seven rays
    } else if (strstr(lower, "twelve") || strstr(lower, "12")) {
        return 12.0;  // Zodiac/clock
    } else if (strstr(lower, "nineteen") || strstr(lower, "19")) {
        return 19.0;  // Metonic cycle
    } else if (strstr(lower, "thirtyone") || strstr(lower, "31")) {
        return 31.0;  // Crown
    }
    
    // Default: ν(λ) = 3 (triad base)
    return 3.0;
}

/**
 * Update dimensional frequencies with phonon correction
 * φᵢ += ν(λ) × 0.1 for prime indices
 */
static void update_phi_freqs(const double *phi_base, double *phi_updated,
                             const char *lambda_phon, int count) {
    double nu = nu_lambda(lambda_phon);
    
    for (int i = 0; i < count; i++) {
        uint64_t Pi = (uint64_t)phi_base[i];
        
        // Check if Pi is prime
        bool is_prime = is_prime_geometric(Pi);
        
        if (is_prime) {
            phi_updated[i] = phi_base[i] + nu * 0.1;
        } else {
            phi_updated[i] = phi_base[i] / 10.0;
        }
    }
}

/* ============================================================================
 * MÖBIUS DUALITY TWIST
 * ============================================================================ */

/**
 * Γ(k) = (-1)^k
 * Provides duality for negative dimensions
 */
static int mobius_twist(int k) {
    return (k % 2 == 0) ? 1 : -1;
}

/* ============================================================================
 * ANGULAR POSITION θ(n,k,λ,ω,ψ)
 * ============================================================================ */

/**
 * θ(n,k,λ,ω,ψ) = k·π·(1+√5) + n·(2π/12) + log₃(ν(λ)) + ω/432 + ψ_term
 */
static double theta_n_complete(uint64_t n, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q, bool use_ratio) {
    double nu = nu_lambda(lambda_phon);
    
    // Term 1: k·π·(1+√5)
    double term1 = (double)k * LATTICE_PI * (1.0 + LATTICE_SQRT5);
    
    // Term 2: n·(2π/12)
    double term2 = (double)n * (2.0 * LATTICE_PI / 12.0);
    
    // Term 3: log₃(ν(λ))
    double term3 = 0.0;
    if (nu > 0) {
        term3 = prime_log(nu) / prime_log(3.0);
    }
    
    // Term 4: ω/432
    double term4 = (double)omega / 432.0;
    
    // Term 5: ψ(p,q)
    double term5;
    if (use_ratio) {
        term5 = pythagorean_ratio(p, q);
    } else {
        term5 = (double)pythagorean_difference_signed(p, q);
    }
    
    return term1 + term2 + term3 + term4 + term5;
}

/* ============================================================================
 * RADIAL POSITION r(p)
 * ============================================================================ */

/**
 * r(p) = log₃(p) + {log₃(p)} × π × φ
 * where {x} is the fractional part
 */
static double r_n_complete(uint64_t prime) {
    if (prime == 0) return 0.0;
    
    double log_val = prime_log((double)prime) / prime_log(3.0);
    double dust = log_val - prime_floor(log_val);  // Fractional part
    
    return log_val + dust * LATTICE_PI * LATTICE_PHI;
}

/* ============================================================================
 * RECURSIVE GROWTH EXPONENT O(n,k,λ)
 * ============================================================================ */

/**
 * O(n,k,λ) = (n-1)·(π/6)/ln(3) + log₃(ν(λ)) + k·π·(1+√5)
 */
static double O_exponent(uint64_t n, int k, const char *lambda_phon) {
    double nu = nu_lambda(lambda_phon);
    
    // Term 1: (n-1)·(π/6)/ln(3)
    double term1 = ((double)n - 1.0) * (LATTICE_PI / 6.0) / prime_log(3.0);
    
    // Term 2: log₃(ν(λ))
    double term2 = 0.0;
    if (nu > 0) {
        term2 = prime_log(nu) / prime_log(3.0);
    }
    
    // Term 3: k·π·(1+√5)
    double term3 = (double)k * LATTICE_PI * (1.0 + LATTICE_SQRT5);
    
    return term1 + term2 + term3;
}

/* ============================================================================
 * RECURSIVE 3^d
 * ============================================================================ */

/**
 * 3^d with Möbius twist for negative dimensions
 * d > 0: 3^d = 3^(d-1) · 3
 * d < 0: 3^d = 3^(d+1) / 3 · (-1)^(d mod 2)
 * d = 0: 3^0 = 1
 */
__attribute__((unused)) static void power_3d(BigInt *result, int d, const BigInt *modulus) {
    if (!result) return;
    
    if (d == 0) {
        big_from_int(result, 1);
        return;
    }
    
    if (d > 0) {
        // Positive: 3^d
        BigInt base, exp;
        big_init(&base);
        big_init(&exp);
        big_from_int(&base, 3);
        big_from_int(&exp, d);
        
        if (modulus) {
            big_powmod(&base, &exp, modulus, result);
        } else {
            // Repeated squaring
            big_from_int(result, 1);
            for (int i = 0; i < d; i++) {
                BigInt temp;
                big_init(&temp);
                big_mul(result, &base, &temp);
                big_copy(result, &temp);
                big_free(&temp);
            }
        }
        
        big_free(&base);
        big_free(&exp);
    } else {
        // Negative: 3^d = 3^(d+1) / 3 · (-1)^(d mod 2)
        power_3d(result, d + 1, modulus);
        
        BigInt three, quotient, remainder;
        big_init(&three);
        big_init(&quotient);
        big_init(&remainder);
        big_from_int(&three, 3);
        
        big_div(result, &three, &quotient, &remainder);
        big_copy(result, &quotient);
        
        // Apply Möbius twist
        if ((-d) % 2 == 1) {
            result->negative = !result->negative;
        }
        
        big_free(&three);
        big_free(&quotient);
        big_free(&remainder);
    }
}

/* ============================================================================
 * LATTICE ENTROPY Γ(n,d)
 * ============================================================================ */

/**
 * Γ(n,d) = log₂(count_primes(d) / d)
 */
static double lattice_entropy(uint64_t n, uint64_t d) {
    (void)n;  // Not used in current formula
    
    if (d == 0) return 0.0;
    
    // Count primes up to d
    uint64_t count = count_primes_geometric(d);
    
    if (count == 0) return 0.0;
    
    double density = (double)count / (double)d;
    return prime_log(density) / prime_log(2.0);
}

/* ============================================================================
 * MASTER LATTICE FUNCTION L(n,d,k,λ)
 * ============================================================================ */

/**
 * L(n,d,k,λ) = 3^O(n,k,λ) × ∏ᵢ₌₁ᵈ cos(θ(n,k,λ)·φᵢ) × Γ(k) × ν(λ) × Γ(n,d)
 * 
 * This generates ALL artifacts in the crystalline lattice.
 */
static double L_lattice_complete(uint64_t n, uint64_t d, int k, const char *lambda_phon,
                 uint16_t omega, uint64_t p, uint64_t q) {
    // Calculate O
    double o = O_exponent(n, k, lambda_phon);
    
    // Base: 3^O
    double base = prime_pow(3.0, o);
    
    // Product: ∏cos(Θ×φᵢ)
    double theta = theta_n_complete(n, k, lambda_phon, omega, p, q, false);
    
    // Update phi frequencies with phonon correction
    double phi_updated[12];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, 12);
    
    double prod = 1.0;
    for (uint64_t i = 0; i < d && i < 12; i++) {
        prod *= prime_cos(theta * phi_updated[i]);
    }
    
    // Γ(k): Möbius twist
    double gamma_k = (double)mobius_twist(k);
    
    // ν(λ): Phonetic value
    double nu = nu_lambda(lambda_phon);
    
    // Γ(n,d): Lattice entropy
    double gamma_nd = lattice_entropy(n, d);
    
    // (ω): Einstein's Λ correction
    double omega_correction = EINSTEIN_LAMBDA;
    
    // Ψ(ψ): Plimpton 322 ratio integration
    double psi_ratio_b = pythagorean_ratio(p, q);  // (p²-q²)/(p²+q²)
    double p2 = (double)(p * p);
    double q2 = (double)(q * q);
    double psi_ratio_c = (2.0 * p * q) / (p2 + q2);  // 2pq/(p²+q²)
    double psi_correction = psi_ratio_b * psi_ratio_c;
    
    return base * prod * gamma_k * nu * omega_correction * psi_correction * gamma_nd;
}

/* ============================================================================
 * DIMENSIONAL LAYER FUNCTIONS
 * ============================================================================ */

/**
 * Z_n^(d) = 3^((n-1)·2π/12 / ln(3)) · cos((n-1)·2π/12 · φ_d)
 */
double Z_n_d(uint64_t n, uint64_t d, const char *lambda_phon) {
    // Update phi frequencies
    double phi_updated[12];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, 12);
    
    double phi_d = phi_updated[d % 12];
    
    // Exponent: (n-1)·2π/12 / ln(3)
    double exponent = ((double)n - 1.0) * (2.0 * LATTICE_PI / 12.0) / prime_log(3.0);
    
    // Base: 3^exponent
    double base = prime_pow(3.0, exponent);
    
    // Cosine term: cos((n-1)·2π/12 · φ_d)
    double angle = ((double)n - 1.0) * (2.0 * LATTICE_PI / 12.0) * phi_d;
    double cos_term = prime_cos(angle);
    
    return base * cos_term;
}

/**
 * P_n^(d)(k) = [12^(θ(k,n)/ln(12) - ln(3))] · ∏ᵢ₌₁ᵈ cos(θ(k,n)·φᵢ)
 */
double P_n_d_k(uint64_t n, uint64_t d, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q) {
    // Calculate theta
    double theta = theta_n_complete(n, k, lambda_phon, omega, p, q, false);
    
    // Exponent: θ(k,n)/ln(12) - ln(3)
    double exponent = theta / prime_log(12.0) - prime_log(3.0);
    
    // Base: 12^exponent
    double base = prime_pow(12.0, exponent);
    
    // Product: ∏cos(θ(k,n)·φᵢ)
    double phi_updated[12];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, 12);
    
    double prod = 1.0;
    for (uint64_t i = 0; i < d && i < 12; i++) {
        prod *= prime_cos(theta * phi_updated[i]);
    }
    
    return base * prod;
}

/* ============================================================================
 * M₁₂ MANIFOLD
 * ============================================================================ */

/**
 * M₁₂ = {x ∈ ℝ¹² | xᵢ = prime_log(pₙ) · cos(θₙ · φᵢ)}
 * Projects primes into 12D space
 */
// // M12Point defined in prime_types.h

static void map_to_m12(M12Point *point, uint64_t prime, uint64_t n,
                      const char *lambda_phon) {
    if (!point) return;
    
    point->prime = prime;
    point->index = n;
    
    // prime_log(pₙ)
    double log_p = prime_log((double)prime);
    
    // θₙ
    double theta = theta_n_complete(n, 1, lambda_phon, 432, 3, 4, false);
    
    // Update phi frequencies
    double phi_updated[12];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, 12);
    
    // xᵢ = prime_log(pₙ) · cos(θₙ · φᵢ)
    for (int i = 0; i < 12; i++) {
        point->coordinates[i] = log_p * prime_cos(theta * phi_updated[i]);
    }
}

/* ============================================================================
 * 15D LATTICE EMBEDDING
 * ============================================================================ */

/**
 * Embed value into 15D lattice
 * residues[i] = value mod prime[i]
 * (LatticeEmbed15D type defined in prime_types.h)
 */
static void embed_to_lattice_15d(LatticeEmbed15D *embed, uint64_t value) {
    if (!embed) return;
    
    embed->value = value;
    
    for (int i = 0; i < 15 && i < 18; i++) {
        embed->residues[i] = value % LATTICE_PRIMES[i];
    }
}

/**
 * Check if embedding passes geometric filters
 */
__attribute__((unused)) static bool is_viable_embedding(const LatticeEmbed15D *embed) {
    if (!embed) return false;
    
    // Check quadratic residues mod 12
    uint64_t mod12 = embed->value % 12;
    bool qr_pass = false;
    for (int i = 0; i < 2; i++) {
        if (mod12 == QR_ALLOW[i]) {
            qr_pass = true;
            break;
        }
    }
    
    return qr_pass;
}

/* ============================================================================
 * COMPLETE CLOCK MAPPING WITH ALL FORMULAS
 * ============================================================================ */

/**
 * Complete clock position with all geometric data
 */
// // CompleteClockMapping defined in prime_types.h

void map_prime_complete(uint64_t prime, uint64_t n, CompleteClockMapping *mapping,
                        const char *lambda_phon) {
    if (!mapping) return;
    
    // Basic clock position
    map_prime_to_clock(prime, &mapping->clock);
    
    // Angular position θ
    mapping->theta = theta_n_complete(n, 1, lambda_phon, 432, 3, 4, false);
    
    // Radial position r
    mapping->r = r_n_complete(prime);
    
    // Recursive growth exponent O
    mapping->O_exp = O_exponent(n, 1, lambda_phon);
    
    // Master lattice function L
    mapping->L_value = L_lattice_complete(n, 12, 1, lambda_phon, 432, 3, 4);
    
    // M₁₂ manifold projection
    map_to_m12(&mapping->m12, prime, n, lambda_phon);
    
    // 15D lattice embedding
    embed_to_lattice_15d(&mapping->embed, prime);
    
    // Plimpton 322 triple
    psi_triple(3, 4, &mapping->psi, n);
}

/* ============================================================================
 * ARTIFACT GENERATION (From "EVERYTHING" Table)
 * ============================================================================ */

/**
 * Test if L value matches known artifact
 * (KnownArtifact type defined in prime_types.h)
 */
static const KnownArtifact KNOWN_ARTIFACTS[] = {
    {3.0, "Babylonian π", "n=2, d=0"},
    {31.0, "Leonardo crown", "n=10, d=3"},
    {144000.0, "Vector culmination", "n=48000, d=3"},
    {143989.0, "Twin prime (lower)", "n=47999"},
    {144001.0, "Twin prime (upper)", "n=48000"},
    {365.0, "Calendar year", "d=4"},
    {355.0/113.0, "π dust", "d=7"},
    {0, NULL, NULL}
};

__attribute__((unused)) static const char* find_artifact(double L_value, double tolerance) {
    for (int i = 0; KNOWN_ARTIFACTS[i].artifact != NULL; i++) {
        if (prime_fabs(L_value - KNOWN_ARTIFACTS[i].L_value) < tolerance) {
            return KNOWN_ARTIFACTS[i].artifact;
        }
    }
    return NULL;
}