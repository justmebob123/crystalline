#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    printf("Testing Fractional Bead Support\n");
    printf("================================\n\n");
    
    /* Test 1: Create from double */
    printf("Test 1: abacus_from_double(157.25, 12, 2)\n");
    CrystallineAbacus* a = abacus_from_double(157.25, 12, 2);
    if (!a) {
        printf("FAIL: Could not create abacus\n");
        return 1;
    }
    
    double value;
    if (abacus_to_double(a, &value) != MATH_SUCCESS) {
        printf("FAIL: Could not convert to double\n");
        return 1;
    }
    
    printf("  Result: %.2f\n", value);
    printf("  Expected: 157.25\n");
    if (fabs(value - 157.25) < 0.01) {
        printf("  PASS\n\n");
    } else {
        printf("  FAIL\n\n");
        return 1;
    }
    
    abacus_free(a);
    
    /* Test 2: Simple fractional addition */
    printf("Test 2: 1.5 + 2.25 = 3.75\n");
    CrystallineAbacus* b = abacus_from_double(1.5, 12, 2);
    CrystallineAbacus* c = abacus_from_double(2.25, 12, 2);
    CrystallineAbacus* result = abacus_new(12);
    
    if (abacus_add(result, b, c) != MATH_SUCCESS) {
        printf("FAIL: Addition failed\n");
        return 1;
    }
    
    if (abacus_to_double(result, &value) != MATH_SUCCESS) {
        printf("FAIL: Could not convert to double\n");
        return 1;
    }
    
    printf("  Result: %.2f\n", value);
    printf("  Expected: 3.75\n");
    if (fabs(value - 3.75) < 0.01) {
        printf("  PASS\n\n");
    } else {
        printf("  FAIL\n\n");
        return 1;
    }
    
    abacus_free(b);
    abacus_free(c);
    abacus_free(result);
    
    /* Test 3: Get/Set precision */
    printf("Test 3: Precision management\n");
    CrystallineAbacus* d = abacus_from_double(3.14159, 12, 5);
    int32_t prec = abacus_get_precision(d);
    printf("  Initial precision: %d\n", prec);
    
    if (abacus_set_precision(d, 2) != MATH_SUCCESS) {
        printf("FAIL: Could not set precision\n");
        return 1;
    }
    
    prec = abacus_get_precision(d);
    printf("  New precision: %d\n", prec);
    
    if (abacus_to_double(d, &value) != MATH_SUCCESS) {
        printf("FAIL: Could not convert to double\n");
        return 1;
    }
    
    printf("  Value after truncation: %.2f\n", value);
    printf("  PASS\n\n");
    
    abacus_free(d);
    
    printf("All fractional tests passed!\n");
    return 0;
}