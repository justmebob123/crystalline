#include <stdio.h>
#include <stdlib.h>
#include "include/bigint_core.h"
#include "include/bigint_ntt.h"

int main() {
    printf("Debugging NTT Inverse\n\n");
    
    NTTContext ctx;
    ntt_init(&ctx, 8);
    
    // Test with simple input that should give [1,1,1,1,1,1,1,1] after forward NTT
    BigInt input[8];
    for (int i = 0; i < 8; i++) {
        big_init(&input[i]);
        big_from_int(&input[i], 1);  // All ones
    }
    
    printf("Testing inverse of [1,1,1,1,1,1,1,1]\n");
    printf("This should give [8,0,0,0,0,0,0,0] (or [8 mod 65537, 0, 0, ...])\n\n");
    
    BigInt output[8];
    ntt_inverse(&ctx, output, input, 8);
    
    printf("Result: ");
    for (int i = 0; i < 8; i++) {
        char* s = big_to_string(&output[i]);
        printf("%s ", s);
        free(s);
    }
    printf("\n");
    
    // Check n_inv
    BigInt n_bigint, n_inv;
    big_init(&n_bigint);
    big_init(&n_inv);
    big_from_int(&n_bigint, 8);
    
    if (big_mod_inverse(&n_inv, &n_bigint, &ctx.prime)) {
        printf("\n1/8 mod 65537 = ");
        char* s = big_to_string(&n_inv);
        printf("%s\n", s);
        free(s);
        
        // Verify: (8 * n_inv) mod 65537 should be 1
        BigInt test;
        big_init(&test);
        big_mul(&n_bigint, &n_inv, &test);
        big_mod(&test, &ctx.prime, &test);
        printf("8 * (1/8) mod 65537 = ");
        char* t = big_to_string(&test);
        printf("%s\n", t);
        free(t);
        big_free(&test);
    } else {
        printf("Failed to compute 1/8 mod 65537\n");
    }
    
    for (int i = 0; i < 8; i++) {
        big_free(&input[i]);
        big_free(&output[i]);
    }
    big_free(&n_bigint);
    big_free(&n_inv);
    ntt_free(&ctx);
    
    return 0;
}
