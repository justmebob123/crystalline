#include "math/clock.h"
#include "math/prime.h"
#include <stdio.h>

int main() {
    ClockPosition pos;
    
    // Test small primes
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    for (int i = 0; i < 10; i++) {
        clock_map_prime_to_position(primes[i], &pos);
        printf("Prime %llu -> Ring %u, Position %u\n", 
               (unsigned long long)primes[i], pos.ring, pos.position);
    }
    
    return 0;
}
