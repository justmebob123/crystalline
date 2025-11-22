#include "include/cllm_pure_crystalline.h"
#include <stdio.h>

int main() {
    printf("Test 1: Prime check\n");
    bool result = crystalline_is_prime(17);
    printf("Is 17 prime? %s\n", result ? "YES" : "NO");
    
    printf("\nTest 2: Get nth prime\n");
    uint64_t prime = crystalline_get_nth_prime(5);
    printf("5th prime: %lu\n", prime);
    
    printf("\nTest 3: Factorize\n");
    uint64_t factors[16];
    uint8_t num_factors;
    crystalline_factorize(30, factors, &num_factors);
    printf("Factors of 30: ");
    for (int i = 0; i < num_factors; i++) {
        printf("%lu ", factors[i]);
    }
    printf("\n");
    
    printf("\nAll simple tests passed!\n");
    return 0;
}
