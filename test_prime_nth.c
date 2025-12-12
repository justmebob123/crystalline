#include <stdio.h>
#include <stdint.h>
#include "math/prime.h"

int main() {
    printf("Testing prime_nth function...\n");
    
    for (int i = 1; i <= 10; i++) {
        printf("Calling prime_nth(%d)...\n", i);
        fflush(stdout);
        
        uint64_t prime = prime_nth(i);
        
        printf("  Prime #%d: %lu\n", i, prime);
        fflush(stdout);
    }
    
    printf("Test complete!\n");
    return 0;
}
