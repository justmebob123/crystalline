#include <stdio.h>
#include <stdlib.h>
#include "include/bigint_core.h"

int main() {
    printf("Testing big_mod_inverse\n\n");
    
    BigInt a, m, result;
    big_init(&a);
    big_init(&m);
    big_init(&result);
    
    big_from_int(&a, 8);
    big_from_int(&m, 65537);
    
    printf("Computing 8^(-1) mod 65537\n");
    
    int success = big_mod_inverse(&result, &a, &m);
    
    if (success) {
        printf("Success! Result = ");
        char* s = big_to_string(&result);
        printf("%s\n", s);
        free(s);
        
        // Verify
        BigInt test;
        big_init(&test);
        big_mul(&a, &result, &test);
        big_mod(&test, &m, &test);
        
        printf("Verification: 8 * result mod 65537 = ");
        char* t = big_to_string(&test);
        printf("%s\n", t);
        free(t);
        big_free(&test);
    } else {
        printf("Failed!\n");
    }
    
    big_free(&a);
    big_free(&m);
    big_free(&result);
    
    return success ? 0 : 1;
}
