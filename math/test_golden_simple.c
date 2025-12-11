#include "math/platonic_generator_abacus.h"
#include "math/abacus.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdio.h>

int main(void) {
    printf("Testing golden ratio computation step by step...\n");
    
    // Test basic operations
    CrystallineAbacus* one = abacus_from_double(1.0, 12, 6);
    CrystallineAbacus* five = abacus_from_double(5.0, 12, 6);
    CrystallineAbacus* two = abacus_from_double(2.0, 12, 6);
    
    double one_val, five_val, two_val;
    abacus_to_double(one, &one_val);
    abacus_to_double(five, &five_val);
    abacus_to_double(two, &two_val);
    
    printf("one = %.2f, five = %.2f, two = %.2f\n", one_val, five_val, two_val);
    
    // Test sqrt(5)
    double sqrt_five_val = math_sqrt(5.0);
    printf("sqrt(5) = %.10f\n", sqrt_five_val);
    
    CrystallineAbacus* sqrt_five = abacus_from_double(sqrt_five_val, 12, 6);
    double sqrt_five_check;
    abacus_to_double(sqrt_five, &sqrt_five_check);
    printf("sqrt(5) via Abacus = %.10f\n", sqrt_five_check);
    
    // Test 1 + sqrt(5)
    CrystallineAbacus* numerator = abacus_from_double(0.0, 12, 6);
    abacus_add(numerator, one, sqrt_five);
    double numerator_val;
    abacus_to_double(numerator, &numerator_val);
    printf("1 + sqrt(5) = %.10f\n", numerator_val);
    
    // Test division
    CrystallineAbacus* result = abacus_from_double(0.0, 12, 6);
    abacus_div(result, NULL, numerator, two);
    double result_val;
    abacus_to_double(result, &result_val);
    printf("(1 + sqrt(5)) / 2 = %.10f\n", result_val);
    printf("Expected: 1.6180339887\n");
    
    abacus_free(one);
    abacus_free(five);
    abacus_free(two);
    abacus_free(sqrt_five);
    abacus_free(numerator);
    abacus_free(result);
    
    printf("\nTesting full golden ratio function...\n");
    CrystallineAbacus* phi = compute_golden_ratio_abacus(12, 6);
    if (!phi) {
        printf("ERROR: Failed to compute golden ratio\n");
        return 1;
    }
    
    double phi_val;
    abacus_to_double(phi, &phi_val);
    printf("Golden ratio: %.10f\n", phi_val);
    
    abacus_free(phi);
    
    printf("\nTesting dodecahedron generation...\n");
    PlatonicSolidAbacus* dodeca = platonic_generate_dodecahedron_abacus(12, 6);
    if (!dodeca) {
        printf("ERROR: Failed to generate dodecahedron\n");
        return 1;
    }
    
    printf("Dodecahedron generated successfully!\n");
    printf("  Vertices: %lu\n", (unsigned long)dodeca->num_vertices);
    
    platonic_abacus_free(dodeca);
    
    return 0;
}