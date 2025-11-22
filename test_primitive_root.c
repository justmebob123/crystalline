/*
 * test_primitive_root.c - Debug primitive root finding
 */

#include <stdio.h>
#include <stdlib.h>
#include "include/bigint_core.h"

void test_primitive_root_65537() {
    printf("Testing primitive root finding for p=65537, n=8\n\n");
    
    BigInt p, n_bigint, p_minus_1, one;
    big_init(&p);
    big_init(&n_bigint);
    big_init(&p_minus_1);
    big_init(&one);
    
    big_from_int(&p, 65537);
    big_from_int(&n_bigint, 8);
    big_from_int(&one, 1);
    
    printf("p = ");
    char* p_str = big_to_string(&p);
    printf("%s\n", p_str);
    free(p_str);
    
    // Compute p-1
    big_sub(&p, &one, &p_minus_1);
    printf("p-1 = ");
    char* pm1_str = big_to_string(&p_minus_1);
    printf("%s\n", pm1_str);
    free(pm1_str);
    
    // Check if n divides p-1
    BigInt quotient, remainder;
    big_init(&quotient);
    big_init(&remainder);
    
    big_div(&p_minus_1, &n_bigint, &quotient, &remainder);
    
    printf("(p-1) / n = ");
    char* q_str = big_to_string(&quotient);
    printf("%s\n", q_str);
    free(q_str);
    
    printf("(p-1) mod n = ");
    char* r_str = big_to_string(&remainder);
    printf("%s\n", r_str);
    free(r_str);
    
    if (big_is_zero(&remainder)) {
        printf("✓ n divides p-1\n\n");
        
        // Try generator 3
        printf("Testing generator g=3:\n");
        BigInt g, omega, test;
        big_init(&g);
        big_init(&omega);
        big_init(&test);
        
        big_from_int(&g, 3);
        
        // omega = g^((p-1)/n) mod p
        printf("Computing ω = 3^%s mod %s\n", q_str, p_str);
        big_powmod(&g, &quotient, &p, &omega);
        
        printf("ω = ");
        char* omega_str = big_to_string(&omega);
        printf("%s\n", omega_str);
        free(omega_str);
        
        // Test: omega^n mod p should equal 1
        printf("\nTesting ω^n mod p:\n");
        big_powmod(&omega, &n_bigint, &p, &test);
        
        printf("ω^n mod p = ");
        char* test_str = big_to_string(&test);
        printf("%s\n", test_str);
        free(test_str);
        
        if (big_cmp(&test, &one) == 0) {
            printf("✓ ω^n ≡ 1 (mod p)\n");
            
            // Test that it's primitive (ω^k ≠ 1 for 0 < k < n)
            printf("\nTesting primitivity:\n");
            int is_primitive = 1;
            
            for (int k = 1; k < 8; k++) {
                if (8 % k == 0 && k != 8) {
                    BigInt k_bigint, test_k;
                    big_init(&k_bigint);
                    big_init(&test_k);
                    
                    big_from_int(&k_bigint, k);
                    big_powmod(&omega, &k_bigint, &p, &test_k);
                    
                    printf("ω^%d mod p = ", k);
                    char* tk_str = big_to_string(&test_k);
                    printf("%s", tk_str);
                    free(tk_str);
                    
                    if (big_cmp(&test_k, &one) == 0) {
                        printf(" ✗ (equals 1, not primitive)\n");
                        is_primitive = 0;
                    } else {
                        printf(" ✓ (not 1)\n");
                    }
                    
                    big_free(&k_bigint);
                    big_free(&test_k);
                }
            }
            
            if (is_primitive) {
                printf("\n✓✓✓ Found primitive 8th root of unity!\n");
            } else {
                printf("\n✗ Not a primitive root\n");
            }
        } else {
            printf("✗ ω^n ≢ 1 (mod p)\n");
        }
        
        big_free(&g);
        big_free(&omega);
        big_free(&test);
    } else {
        printf("✗ n does not divide p-1\n");
    }
    
    big_free(&p);
    big_free(&n_bigint);
    big_free(&p_minus_1);
    big_free(&one);
    big_free(&quotient);
    big_free(&remainder);
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║              Primitive Root Finding Debug Test                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n\n");
    
    test_primitive_root_65537();
    
    return 0;
}
