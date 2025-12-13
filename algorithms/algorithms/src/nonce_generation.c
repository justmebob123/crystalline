/**
 * @file nonce_generation.c
 * @brief Implementation of deterministic nonce generation
 */

#include "nonce_generation.h"
#include "symbolic_field_theory.h"
#include "mathematical_formulas.h"
#include "prime_lowlevel.h"
#include "cllm_mathematical_constants.h"
#include <stdlib.h>
#include <string.h>

void nonce_config_init(NonceConfig* config, uint64_t block_height, 
                       uint32_t difficulty_bits) {
    if (config == NULL) return;
    
    config->block_height = block_height;
    config->difficulty_bits = difficulty_bits;
    config->tetration_depth = 3;        // Standard depth
    config->max_reassessments = 5;      // Up to 5 attempts
    config->entropy_cut_min = 0.18;     // 18% minimum reduction
    config->entropy_cut_max = 0.45;     // 45% maximum reduction
}

bool nonce_generate_deterministic(const NonceConfig* config, NonceResult* result) {
    if (config == NULL || result == NULL) {
        return false;
    }
    
    // Initialize result
    memset(result, 0, sizeof(NonceResult));
    result->success = false;
    
    // Step 1: Use SFT to find seed prime from block_height
    // Search in range [block_height, block_height + 10000]
    uint64_t primes[100];
    size_t prime_count = sft_deterministic_prime_map(
        config->block_height,
        config->block_height + 10000,
        primes,
        100
    );
    
    if (prime_count == 0) {
        // Fallback: use block_height itself if it's prime, or next prime
        uint64_t candidate = config->block_height;
        if (candidate < 2) candidate = 2;
        
        // Find next prime using simple search
        while (candidate < config->block_height + 10000) {
            bool is_prime = true;
            if (candidate <= 1) {
                is_prime = false;
            } else if (candidate <= 3) {
                is_prime = true;
            } else if (candidate % 2 == 0 || candidate % 3 == 0) {
                is_prime = false;
            } else {
                for (uint64_t i = 5; i * i <= candidate; i += 6) {
                    if (candidate % i == 0 || candidate % (i + 2) == 0) {
                        is_prime = false;
                        break;
                    }
                }
            }
            
            if (is_prime) {
                primes[0] = candidate;
                prime_count = 1;
                break;
            }
            candidate++;
        }
        
        if (prime_count == 0) {
            return false; // Failed to find seed prime
        }
    }
    
    // Use first prime as seed
    result->seed_prime = primes[0];
    
    // Step 2: Build tetration stack
    uint64_t modulus = UINT64_MAX; // Use max value as modulus
    uint64_t tetration_value = nonce_build_tetration_stack(
        result->seed_prime,
        config->tetration_depth,
        modulus
    );
    
    // Step 3: Apply entropy reduction based on difficulty
    uint64_t nonce_candidate = nonce_apply_difficulty_bounds(
        tetration_value,
        config->difficulty_bits,
        config->entropy_cut_min,
        config->entropy_cut_max
    );
    
    // Step 4: Validate and reassess if needed
    result->reassessments = 0;
    result->nonce = nonce_candidate;
    
    while (result->reassessments < config->max_reassessments) {
        if (nonce_validate_difficulty(result->nonce, config->difficulty_bits)) {
            result->success = true;
            result->final_entropy = nonce_calculate_entropy_reduction(
                config->difficulty_bits,
                config->entropy_cut_min,
                config->entropy_cut_max
            );
            return true;
        }
        
        // Reassess with adjusted parameters
        result->reassessments++;
        result->nonce = nonce_reassess(config, result->reassessments, result->nonce);
    }
    
    // If we get here, we failed to find valid nonce within max reassessments
    // Return the best candidate we have
    result->success = false;
    return false;
}

uint64_t nonce_build_tetration_stack(uint64_t seed_prime, uint32_t depth,
                                      uint64_t modulus) {
    if (depth == 0 || seed_prime == 0) {
        return 0;
    }
    
    if (depth == 1) {
        return seed_prime % modulus;
    }
    
    // Build stack iteratively to avoid overflow
    uint64_t result = seed_prime;
    
    for (uint32_t i = 1; i < depth; i++) {
        // result = seed_prime^result (mod modulus)
        result = prime_powmod_int(seed_prime, result, modulus);
    }
    
    return result;
}

uint64_t nonce_apply_difficulty_bounds(uint64_t tetration_value,
                                        uint32_t difficulty_bits,
                                        double entropy_cut_min,
                                        double entropy_cut_max) {
    // Calculate entropy reduction factor
    double reduction = nonce_calculate_entropy_reduction(
        difficulty_bits,
        entropy_cut_min,
        entropy_cut_max
    );
    
    // Apply reduction to tetration value
    // Higher difficulty = smaller search space
    uint64_t max_value = UINT64_MAX;
    if (difficulty_bits < 64) {
        max_value = (1ULL << difficulty_bits) - 1;
    }
    
    // Scale tetration value to fit within difficulty bounds
    uint64_t bounded = tetration_value % max_value;
    
    // Apply entropy reduction
    bounded = (uint64_t)((double)bounded * (1.0 - reduction));
    
    return bounded;
}

double nonce_calculate_entropy_reduction(uint32_t difficulty_bits,
                                          double cut_min, double cut_max) {
    if (difficulty_bits == 0) {
        return cut_min;
    }
    
    // Use golden ratio for smooth scaling
    // Higher difficulty = more entropy reduction
    double phi = PHI; // Golden ratio from constants
    
    // Scale difficulty to [0, 1] range (assuming max 64 bits)
    double normalized = (double)difficulty_bits / 64.0;
    
    // Apply golden ratio scaling
    double scaled = normalized * phi;
    if (scaled > 1.0) scaled = 1.0;
    
    // Interpolate between min and max
    return cut_min + (cut_max - cut_min) * scaled;
}

uint64_t nonce_reassess(const NonceConfig* config, uint32_t attempt,
                        uint64_t previous_nonce) {
    if (config == NULL || attempt == 0) {
        return previous_nonce;
    }
    
    // Adjust entropy parameters based on attempt number
    // Use golden ratio for adjustment
    double phi = PHI;
    double adjustment = (double)attempt / (double)config->max_reassessments;
    adjustment *= phi;
    if (adjustment > 1.0) adjustment = 1.0;
    
    // Increase entropy cut with each attempt
    double new_cut_min = config->entropy_cut_min * (1.0 + adjustment * 0.1);
    double new_cut_max = config->entropy_cut_max * (1.0 + adjustment * 0.1);
    
    if (new_cut_min > 0.9) new_cut_min = 0.9;
    if (new_cut_max > 0.95) new_cut_max = 0.95;
    
    // Rebuild with adjusted parameters
    uint64_t modulus = UINT64_MAX;
    uint64_t tetration_value = nonce_build_tetration_stack(
        config->block_height + attempt, // Vary seed slightly
        config->tetration_depth,
        modulus
    );
    
    uint64_t new_nonce = nonce_apply_difficulty_bounds(
        tetration_value,
        config->difficulty_bits,
        new_cut_min,
        new_cut_max
    );
    
    // Mix with previous nonce using XOR
    return new_nonce ^ previous_nonce;
}

bool nonce_validate_difficulty(uint64_t nonce, uint32_t difficulty_bits) {
    if (difficulty_bits == 0) {
        return true; // No difficulty requirement
    }
    
    if (difficulty_bits >= 64) {
        return false; // Impossible difficulty
    }
    
    // Check if nonce has required number of leading zeros
    // This is a simplified check - actual Bitcoin mining would
    // hash the block header and check the hash
    
    // Count leading zeros in nonce
    uint32_t leading_zeros = 0;
    uint64_t mask = 1ULL << 63;
    
    while (leading_zeros < 64 && (nonce & mask) == 0) {
        leading_zeros++;
        mask >>= 1;
    }
    
    // For this simplified version, we check if the nonce value
    // is small enough to meet the difficulty target
    uint64_t max_value = (1ULL << (64 - difficulty_bits)) - 1;
    
    return nonce <= max_value;
}
