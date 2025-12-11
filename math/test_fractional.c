#include "math/abacus.h"
#include <stdio.h>

int main() {
    // Test: 36 / 7 = 5 remainder 1
    CrystallineAbacus* a = abacus_from_uint64(36, 12);
    CrystallineAbacus* b = abacus_from_uint64(7, 12);
    CrystallineAbacus* quotient = abacus_new(12);
    CrystallineAbacus* remainder = abacus_new(12);
    
    printf("Dividend (36):\n");
    abacus_print(a);
    printf("\nDivisor (7):\n");
    abacus_print(b);
    
    MathError err = abacus_div(quotient, remainder, a, b);
    
    printf("\nQuotient:\n");
    abacus_print(quotient);
    printf("\nRemainder:\n");
    abacus_print(remainder);
    
    uint64_t q_val, r_val;
    abacus_to_uint64(quotient, &q_val);
    abacus_to_uint64(remainder, &r_val);
    
    printf("\nQuotient value: %lu (expected 5)\n", q_val);
    printf("Remainder value: %lu (expected 1)\n", r_val);
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(quotient);
    abacus_free(remainder);
    
    return 0;
}
