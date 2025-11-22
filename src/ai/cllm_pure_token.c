/*
 * Pure Crystalline CLLM - Token Operations
 */

#include "../../include/cllm_pure_crystalline.h"
#include "../../include/bigint_core.h"
#include "../../include/bigfixed_core.h"
#include "../../include/prime_math_custom.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PHI 1.618033988749895
#define GOLDEN_ANGLE (2.0 * M_PI / (PHI * PHI))
#define PRIME_CACHE_SIZE 10000

static uint64_t prime_cache[PRIME_CACHE_SIZE];
static bool prime_cache_initialized = false;

static void init_prime_cache(void) {
    if (prime_cache_initialized) return;
    
    prime_cache[0] = 2;
    prime_cache[1] = 3;
    
    uint32_t count = 2;
    uint64_t candidate = 5;
    
    while (count < PRIME_CACHE_SIZE) {
        bool is_prime = true;
        uint64_t sqrt_cand = (uint64_t)sqrt((double)candidate);
        
        for (uint32_t i = 0; i < count && prime_cache[i] <= sqrt_cand; i++) {
            if (candidate % prime_cache[i] == 0) {
                is_prime = false;
                break;
            }
        }
        
        if (is_prime) {
            prime_cache[count++] = candidate;
        }
        
        candidate += 2;
    }
    
    prime_cache_initialized = true;
}

bool crystalline_is_prime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    uint64_t sqrt_n = (uint64_t)sqrt((double)n);
    for (uint64_t i = 3; i <= sqrt_n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

uint64_t crystalline_get_nth_prime(uint32_t n) {
    init_prime_cache();
    
    if (n < PRIME_CACHE_SIZE) {
        return prime_cache[n];
    }
    
    uint64_t count = PRIME_CACHE_SIZE;
    uint64_t candidate = prime_cache[PRIME_CACHE_SIZE - 1] + 2;
    
    while (count <= n) {
        if (crystalline_is_prime(candidate)) {
            if (count == n) return candidate;
            count++;
        }
        candidate += 2;
    }
    
    return candidate;
}

void crystalline_factorize(uint64_t number, uint64_t* factors, uint8_t* num_factors) {
    if (!factors || !num_factors) return;
    
    *num_factors = 0;
    
    if (number <= 1) return;
    
    if (crystalline_is_prime(number)) {
        factors[0] = number;
        *num_factors = 1;
        return;
    }
    
    while (number % 2 == 0 && *num_factors < MAX_PRIME_FACTORS) {
        factors[(*num_factors)++] = 2;
        number /= 2;
    }
    
    for (uint64_t i = 3; i * i <= number && *num_factors < MAX_PRIME_FACTORS; i += 2) {
        while (number % i == 0) {
            factors[(*num_factors)++] = i;
            number /= i;
        }
    }
    
    if (number > 1 && *num_factors < MAX_PRIME_FACTORS) {
        factors[(*num_factors)++] = number;
    }
}

void crystalline_compute_ulam_position(uint64_t prime, BigFixed coords[3], int precision) {
    if (!coords) return;
    
    init_prime_cache();
    
    // Initialize BigFixed structures if needed
    for (int i = 0; i < 3; i++) {
        if (!coords[i].integer_part) {
            coords[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
            big_init(coords[i].integer_part);
        }
        if (!coords[i].fractional_part) {
            coords[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
            big_init(coords[i].fractional_part);
        }
    }
    
    uint32_t prime_index = 0;
    for (uint32_t i = 0; i < PRIME_CACHE_SIZE; i++) {
        if (prime_cache[i] == prime) {
            prime_index = i;
            break;
        }
        if (prime_cache[i] > prime) break;
    }
    
    if (prime_index == 0 && prime > prime_cache[PRIME_CACHE_SIZE - 1]) {
        prime_index = (uint32_t)(prime / log((double)prime));
    }
    
    double radius = sqrt((double)prime_index);
    double angle = GOLDEN_ANGLE * prime_index;
    
    double x = radius * cos(angle);
    double y = radius * sin(angle);
    double z = log((double)prime + 1.0);
    
    double perturbation = 0.01 * (prime % 100);
    x += perturbation * cos(angle + M_PI/4);
    y += perturbation * sin(angle + M_PI/4);
    z += perturbation * 0.1;
    
    big_fixed_from_double(&coords[0], x);
    big_fixed_from_double(&coords[1], y);
    big_fixed_from_double(&coords[2], z);
}

CrystallineToken* crystalline_token_create(uint32_t token_id, const char* token_str, uint64_t prime) {
    CrystallineToken* token = (CrystallineToken*)calloc(1, sizeof(CrystallineToken));
    if (!token) return NULL;
    
    token->token_id = token_id;
    if (token_str) {
        strncpy(token->token_str, token_str, 63);
        token->token_str[63] = '\0';
    }
    
    token->prime = prime;
    token->is_root = crystalline_is_prime(prime);
    
    crystalline_factorize(prime, token->prime_factors, &token->num_factors);
    
    // Initialize BigFixed coordinates
    for (int i = 0; i < 3; i++) {
        token->lattice_coords[i].integer_part = NULL;
        token->lattice_coords[i].fractional_part = NULL;
    }
    
    crystalline_compute_ulam_position(prime, token->lattice_coords, 256);
    
    if (token->is_root) {
        token->root_token_id = token_id;
    } else {
        token->root_token_id = 0;
    }
    
    token->num_neighbors = 0;
    token->usage_count = 0;
    token->root_score = 1.0;
    
    return token;
}

void crystalline_token_free(CrystallineToken* token) {
    if (!token) return;
    
    // Free BigFixed coordinates
    for (int i = 0; i < 3; i++) {
        if (token->lattice_coords[i].integer_part) {
            big_free(token->lattice_coords[i].integer_part);
            free(token->lattice_coords[i].integer_part);
        }
        if (token->lattice_coords[i].fractional_part) {
            big_free(token->lattice_coords[i].fractional_part);
            free(token->lattice_coords[i].fractional_part);
        }
    }
    
    free(token);
}

void crystalline_lattice_distance(const BigFixed pos1[3], const BigFixed pos2[3], BigFixed* distance) {
    if (!pos1 || !pos2 || !distance) return;
    
    // Initialize distance if needed
    if (!distance->integer_part) {
        distance->integer_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(distance->integer_part);
    }
    if (!distance->fractional_part) {
        distance->fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(distance->fractional_part);
    }
    
    BigFixed diff[3];
    BigFixed diff_sq[3];
    BigFixed sum;
    
    // Initialize temporary BigFixed structures
    for (int i = 0; i < 3; i++) {
        diff[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
        diff[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(diff[i].integer_part);
        big_init(diff[i].fractional_part);
        
        diff_sq[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
        diff_sq[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(diff_sq[i].integer_part);
        big_init(diff_sq[i].fractional_part);
    }
    
    sum.integer_part = (BigInt*)malloc(sizeof(BigInt));
    sum.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(sum.integer_part);
    big_init(sum.fractional_part);
    
    for (int i = 0; i < 3; i++) {
        big_fixed_sub(&diff[i], &pos1[i], &pos2[i]);
        big_fixed_mul(&diff_sq[i], &diff[i], &diff[i]);
    }
    
    big_fixed_from_int(&sum, 0);
    for (int i = 0; i < 3; i++) {
        BigFixed temp;
        temp.integer_part = (BigInt*)malloc(sizeof(BigInt));
        temp.fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(temp.integer_part);
        big_init(temp.fractional_part);
        
        big_fixed_add(&temp, &sum, &diff_sq[i]);
        
        big_free(sum.integer_part);
        big_free(sum.fractional_part);
        free(sum.integer_part);
        free(sum.fractional_part);
        
        sum = temp;
    }
    
    double sum_double = big_fixed_to_double(&sum);
    double dist_double = sqrt(sum_double);
    big_fixed_from_double(distance, dist_double);
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        big_free(diff[i].integer_part);
        big_free(diff[i].fractional_part);
        free(diff[i].integer_part);
        free(diff[i].fractional_part);
        
        big_free(diff_sq[i].integer_part);
        big_free(diff_sq[i].fractional_part);
        free(diff_sq[i].integer_part);
        free(diff_sq[i].fractional_part);
    }
    
    big_free(sum.integer_part);
    big_free(sum.fractional_part);
    free(sum.integer_part);
    free(sum.fractional_part);
}

void crystalline_prime_similarity(uint64_t prime1, uint64_t prime2, BigFixed* similarity) {
    if (!similarity) return;
    
    // Initialize if needed
    if (!similarity->integer_part) {
        similarity->integer_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(similarity->integer_part);
    }
    if (!similarity->fractional_part) {
        similarity->fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(similarity->fractional_part);
    }
    
    if (prime1 == prime2) {
        big_fixed_from_double(similarity, 1.0);
        return;
    }
    
    uint64_t a = prime1, b = prime2;
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    uint64_t gcd = a;
    
    if (gcd == 1) {
        big_fixed_from_double(similarity, 0.5);
    } else {
        double sim = 1.0 / (double)gcd;
        if (sim > 1.0) sim = 1.0;
        big_fixed_from_double(similarity, sim);
    }
}

void crystalline_phase_alignment(uint64_t prime1, uint64_t prime2, BigFixed* alignment) {
    if (!alignment) return;
    
    // Initialize if needed
    if (!alignment->integer_part) {
        alignment->integer_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(alignment->integer_part);
    }
    if (!alignment->fractional_part) {
        alignment->fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(alignment->fractional_part);
    }
    
    double phase_diff = 2.0 * M_PI * (double)(prime1 - prime2) / (double)(prime1 + prime2);
    double align = (1.0 + cos(phase_diff)) / 2.0;
    
    big_fixed_from_double(alignment, align);
}
