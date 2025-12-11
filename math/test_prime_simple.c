#include "math/prime.h"
#include <stdio.h>

int main() {
    printf("Testing prime_is_prime(2)...\n");
    bool result = prime_is_prime(2);
    printf("Result: %d\n", result);
    
    printf("Testing prime_is_prime(3)...\n");
    result = prime_is_prime(3);
    printf("Result: %d\n", result);
    
    printf("Testing prime_is_prime(4)...\n");
    result = prime_is_prime(4);
    printf("Result: %d\n", result);
    
    printf("Testing prime_nth(1)...\n");
    uint64_t prime = prime_nth(1);
    printf("Result: %llu\n", (unsigned long long)prime);
    
    return 0;
}
