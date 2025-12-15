#include "math/ntt.h"
#include "math/abacus.h"
#include "math/arithmetic.h"
#include <stdio.h>

void print_abacus_value(const char* label, const CrystallineAbacus* a) {
    printf("%s: ", label);
    
    /* Try to convert to uint64 if small enough */
    CrystallineAbacus* temp = abacus_copy(a);
    uint64_t val = 0;
    bool fits = true;
    
    /* Simple extraction - just for debugging */
    printf("[abacus value]\n");
    
    abacus_free(temp);
}

int main() {
    printf("=== Test Prime and Root Finding ===\n\n");
    
    /* Test with n=256 */
    size_t n = 256;
    
    printf("Finding prime for n=%zu...\n", n);
    CrystallineAbacus* prime = abacus_new(60);
    MathError err = ntt_find_prime(prime, n, 64);
    printf("ntt_find_prime returned: %d\n", err);
    
    if (err != MATH_SUCCESS) {
        printf("FAIL: Could not find prime\n");
        abacus_free(prime);
        return 1;
    }
    
    print_abacus_value("Prime", prime);
    
    /* Now try to find primitive root */
    printf("\nFinding primitive root for n=%zu...\n", n);
    CrystallineAbacus* root = abacus_new(60);
    err = ntt_find_primitive_root(root, n, prime);
    printf("ntt_find_primitive_root returned: %d\n", err);
    
    if (err != MATH_SUCCESS) {
        printf("FAIL: Could not find primitive root\n");
        
        /* Try with a known good prime for n=256 */
        printf("\n=== Trying with known prime 65537 ===\n");
        CrystallineAbacus* known_prime = abacus_from_uint64(65537, 60);
        err = ntt_find_primitive_root(root, n, known_prime);
        printf("ntt_find_primitive_root returned: %d\n", err);
        
        if (err == MATH_SUCCESS) {
            printf("SUCCESS with known prime!\n");
            print_abacus_value("Root", root);
        } else {
            printf("FAIL even with known prime\n");
        }
        
        abacus_free(known_prime);
    } else {
        printf("SUCCESS: Found primitive root\n");
        print_abacus_value("Root", root);
    }
    
    abacus_free(root);
    abacus_free(prime);
    
    return 0;
}