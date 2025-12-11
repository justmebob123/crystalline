#include "math/clock.h"
#include "math/prime.h"
#include <stdio.h>

int main() {
    ClockPosition pos;
    
    printf("Testing clock_position_to_prime()...\n\n");
    
    // Test small primes (Ring 0)
    printf("Ring 0 (small primes):\n");
    for (uint32_t i = 0; i < 12; i++) {
        pos.ring = 0;
        pos.position = i;
        pos.angle = 0;
        pos.radius = 1.0;
        
        uint64_t prime = clock_position_to_prime(&pos);
        if (prime > 0) {
            printf("  Position %u -> Prime %llu\n", i, (unsigned long long)prime);
        }
    }
    
    // Test bidirectional mapping
    printf("\nBidirectional mapping test:\n");
    uint64_t test_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    for (int i = 0; i < 10; i++) {
        uint64_t original_prime = test_primes[i];
        
        // Map prime to position
        if (clock_map_prime_to_position(original_prime, &pos) == MATH_SUCCESS) {
            // Map position back to prime
            uint64_t recovered_prime = clock_position_to_prime(&pos);
            
            if (recovered_prime == original_prime) {
                printf("  ✓ Prime %llu -> Position (%u, %u) -> Prime %llu\n",
                       (unsigned long long)original_prime,
                       pos.ring, pos.position,
                       (unsigned long long)recovered_prime);
            } else {
                printf("  ✗ Prime %llu -> Position (%u, %u) -> Prime %llu (MISMATCH)\n",
                       (unsigned long long)original_prime,
                       pos.ring, pos.position,
                       (unsigned long long)recovered_prime);
            }
        }
    }
    
    return 0;
}
