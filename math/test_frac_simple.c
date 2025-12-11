#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Creating 1.5 in base 12...\n");
    CrystallineAbacus* a = abacus_from_double(1.5, 12, 2);
    if (!a) {
        printf("ERROR: Failed to create abacus from 1.5\n");
        return 1;
    }
    printf("Success! Printing:\n");
    abacus_print(a);
    
    double val;
    MathError err = abacus_to_double(a, &val);
    if (err == MATH_SUCCESS) {
        printf("Converted back to double: %.2f\n", val);
    } else {
        printf("ERROR: Failed to convert to double\n");
    }
    
    printf("\n=== Testing Addition: 1.5 + 2.25 ===\n");
    CrystallineAbacus* b = abacus_from_double(2.25, 12, 2);
    CrystallineAbacus* result = abacus_new(12);
    
    if (b && result) {
        printf("b (2.25): ");
        abacus_print(b);
        
        err = abacus_add(result, a, b);
        if (err == MATH_SUCCESS) {
            printf("result: ");
            abacus_print(result);
            
            double result_val;
            abacus_to_double(result, &result_val);
            printf("Result value: %.2f (expected 3.75)\n", result_val);
            
            if (result_val >= 3.74 && result_val <= 3.76) {
                printf("✓ PASS\n");
            } else {
                printf("✗ FAIL\n");
            }
        }
    }
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    return 0;
}
