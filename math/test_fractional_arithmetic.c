#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("=== Fractional Arithmetic Tests ===\n\n");
    
    // Test 1: 1.5 + 2.25 = 3.75
    printf("Test 1: 1.5 + 2.25 = 3.75\n");
    CrystallineAbacus* a = abacus_from_double(1.5, 10, 2);
    CrystallineAbacus* b = abacus_from_double(2.25, 10, 2);
    CrystallineAbacus* result = abacus_new(10);
    
    if (a && b && result) {
        printf("  a (1.5): ");
        abacus_print(a);
        printf("  b (2.25): ");
        abacus_print(b);
        
        MathError err = abacus_add(result, a, b);
        if (err == MATH_SUCCESS) {
            printf("  result: ");
            abacus_print(result);
            
            double val;
            abacus_to_double(result, &val);
            printf("  value: %.2f (expected 3.75)\n", val);
            printf("  %s\n\n", (val >= 3.74 && val <= 3.76) ? "PASS" : "FAIL");
        } else {
            printf("  ERROR: Addition failed\n\n");
        }
    }
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    
    // Test 2: 0.9 + 0.2 = 1.1
    printf("Test 2: 0.9 + 0.2 = 1.1\n");
    a = abacus_from_double(0.9, 10, 2);
    b = abacus_from_double(0.2, 10, 2);
    result = abacus_new(10);
    
    if (a && b && result) {
        MathError err = abacus_add(result, a, b);
        if (err == MATH_SUCCESS) {
            double val;
            abacus_to_double(result, &val);
            printf("  value: %.2f (expected 1.10)\n", val);
            printf("  %s\n\n", (val >= 1.09 && val <= 1.11) ? "PASS" : "FAIL");
        }
    }
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    
    // Test 3: 1.0 - 0.1 = 0.9
    printf("Test 3: 1.0 - 0.1 = 0.9\n");
    a = abacus_from_double(1.0, 10, 2);
    b = abacus_from_double(0.1, 10, 2);
    result = abacus_new(10);
    
    if (a && b && result) {
        MathError err = abacus_sub(result, a, b);
        if (err == MATH_SUCCESS) {
            double val;
            abacus_to_double(result, &val);
            printf("  value: %.2f (expected 0.90)\n", val);
            printf("  %s\n\n", (val >= 0.89 && val <= 0.91) ? "PASS" : "FAIL");
        }
    }
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    
    printf("=== Tests Complete ===\n");
    return 0;
}
