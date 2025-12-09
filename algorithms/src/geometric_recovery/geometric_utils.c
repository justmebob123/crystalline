/**
 * @file geometric_utils.c
 * @brief Utility functions for k↔position mapping with +1 bit extension
 */

#include "geometric_recovery.h"
#include "prime_float_math.h"
#include <string.h>

/**
 * Map k (BIGNUM) to position in high-dimensional space
 * 
 * CRITICAL: Uses 257+ bits (256 + 1 for boundary crossing)
 * Each additional scalar adds +1 bit.
 */
void map_k_to_position(
    BIGNUM* k,
    double* position_out,
    uint32_t num_dimensions
) {
    // Extract k bytes (use 257 bits = 33 bytes)
    unsigned char k_bytes[33];
    memset(k_bytes, 0, 33);
    
    int len = BN_num_bytes(k);
    if (len > 33) len = 33;
    BN_bn2bin(k, k_bytes + (33 - len));
    
    // Map 257 bits to num_dimensions
    // Each dimension gets 257/num_dimensions bits
    uint32_t bits_per_dim = 257 / num_dimensions;
    if (bits_per_dim < 1) bits_per_dim = 1;
    
    for (uint32_t d = 0; d < num_dimensions; d++) {
        uint64_t value = 0;
        uint32_t bit_start = d * bits_per_dim;
        
        // Extract bits for this dimension
        for (uint32_t b = 0; b < bits_per_dim && bit_start + b < 257; b++) {
            uint32_t byte_idx = (bit_start + b) / 8;
            uint32_t bit_idx = (bit_start + b) % 8;
            
            if (byte_idx < 33 && (k_bytes[byte_idx] & (1 << (7 - bit_idx)))) {
                value |= (1ULL << b);
            }
        }
        
        // Normalize to [-1, 1] (centered at 0 for quadrant detection)
        uint64_t max_value = (1ULL << bits_per_dim) - 1;
        position_out[d] = 2.0 * ((double)value / (double)max_value) - 1.0;
    }
}

/**
 * Map position to k (BIGNUM) with +1 bit extension
 * 
 * CRITICAL: Generates 257-bit k (256 + 1 for boundary)
 */
BIGNUM* map_position_to_k(
    const double* position,
    uint32_t num_dimensions
) {
    // Allocate 257 bits = 33 bytes
    unsigned char k_bytes[33];
    memset(k_bytes, 0, 33);
    
    uint32_t bits_per_dim = 257 / num_dimensions;
    if (bits_per_dim < 1) bits_per_dim = 1;
    
    for (uint32_t d = 0; d < num_dimensions; d++) {
        // Convert position from [-1, 1] to [0, 1]
        double normalized = (position[d] + 1.0) / 2.0;
        if (normalized < 0.0) normalized = 0.0;
        if (normalized > 1.0) normalized = 1.0;
        
        // Convert to integer value
        uint64_t max_value = (1ULL << bits_per_dim) - 1;
        uint64_t value = (uint64_t)(normalized * max_value);
        
        // Set bits
        uint32_t bit_start = d * bits_per_dim;
        for (uint32_t b = 0; b < bits_per_dim && bit_start + b < 257; b++) {
            if (value & (1ULL << b)) {
                uint32_t byte_idx = (bit_start + b) / 8;
                uint32_t bit_idx = (bit_start + b) % 8;
                if (byte_idx < 33) {
                    k_bytes[byte_idx] |= (1 << (7 - bit_idx));
                }
            }
        }
    }
    
    // Create BIGNUM from bytes
    BIGNUM* k = BN_new();
    BN_bin2bn(k_bytes, 33, k);
    
    return k;
}

/**
 * Compute Hamming distance between two k values
 * 
 * Returns distance in range [0, 1] where 0 = identical, 1 = completely different
 */
double compute_k_hamming_distance(BIGNUM* k1, BIGNUM* k2) {
    unsigned char k1_bytes[33], k2_bytes[33];
    memset(k1_bytes, 0, 33);
    memset(k2_bytes, 0, 33);
    
    int len1 = BN_num_bytes(k1);
    int len2 = BN_num_bytes(k2);
    if (len1 > 33) len1 = 33;
    if (len2 > 33) len2 = 33;
    
    BN_bn2bin(k1, k1_bytes + (33 - len1));
    BN_bn2bin(k2, k2_bytes + (33 - len2));
    
    uint32_t different_bits = 0;
    for (int i = 0; i < 33; i++) {
        unsigned char xor_byte = k1_bytes[i] ^ k2_bytes[i];
        for (int b = 0; b < 8; b++) {
            if (xor_byte & (1 << b)) different_bits++;
        }
    }
    
    // 257 bits total
    return (double)different_bits / 257.0;
}