#include <stdio.h>
#include <stdlib.h>
#include "include/bigint_core.h"

int main() {
    printf("Testing GCD first\n");
    
    BigInt a, m, gcd_val;
    big_init(&a);
    big_init(&m);
    big_init(&gcd_val);
    
    big_from_int(&a, 8);
    big_from_int(&m, 65537);
    
    big_gcd(&a, &m, &gcd_val);
    
    printf("gcd(8, 65537) = ");
    char* s = big_to_string(&gcd_val);
    printf("%s\n", s);
    free(s);
    
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    
    printf("Comparing with 1: %d\n", big_cmp(&gcd_val, &one));
    
    if (big_cmp(&gcd_val, &one) == 0) {
        printf("✓ GCD is 1, inverse should exist\n");
    } else {
        printf("✗ GCD is not 1\n");
    }
    
    big_free(&a);
    big_free(&m);
    big_free(&gcd_val);
    big_free(&one);
    
    return 0;
}
