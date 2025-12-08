#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef struct {
    uint64_t target;
    int dimension;
} PlatonicTarget;

static const PlatonicTarget PLATONIC_TARGETS[] = {
    {8, 3},      // 2^3 = 8 (Cube)
    {27, 3},     // 3^3 = 27 (Tetrahedron)  
    {125, 3},    // 5^3 = 125 (Dodecahedron)
    {343, 3}     // 7^3 = 343 (Octahedron)
};
#define NUM_PLATONIC_TARGETS 4

static inline double geometric_resonance(uint64_t n) {
    double score = 0.0;
    double sigma = 100.0;
    
    for (int i = 0; i < NUM_PLATONIC_TARGETS; i++) {
        double dist = (double)n - (double)PLATONIC_TARGETS[i].target;
        score += exp(-(dist * dist) / sigma);
    }
    
    return score;
}

int main() {
    uint64_t test_primes[] = {1009, 1013, 1019, 1021, 1031};
    
    for (int i = 0; i < 5; i++) {
        double res = geometric_resonance(test_primes[i]);
        printf("Prime %lu: resonance = %.10f (threshold 0.001)\n", 
               test_primes[i], res);
    }
    
    return 0;
}
