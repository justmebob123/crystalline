/**
 * @file analyze_clock_patterns.c
 * @brief Analyze clock lattice patterns to derive deterministic prime formula
 * 
 * This tool extracts patterns from the rainbow table and clock lattice
 * to discover the O(1) formula for prime generation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "math/prime.h"
#include "math/clock.h"
#include "math/rainbow.h"

#define MAX_PRIMES 10000
#define MAX_MAGNITUDE 100

/* Statistics for each (ring, position) combination */
typedef struct {
    uint32_t ring;
    uint32_t position;
    uint32_t count;                    /* Number of primes at this position */
    uint64_t primes[MAX_MAGNITUDE];    /* Primes at different magnitude levels */
    uint64_t deltas[MAX_MAGNITUDE-1];  /* Differences between consecutive primes */
    double avg_delta;                   /* Average delta */
    double density;                     /* Prime density at this position */
} PositionStats;

/* Global statistics */
typedef struct {
    PositionStats ring0[12];
    PositionStats ring1[60];
    PositionStats ring2[60];
    PositionStats ring3[100];
    uint64_t total_primes_analyzed;
} ClockStats;

/**
 * @brief Initialize statistics structure
 */
void init_stats(ClockStats* stats) {
    memset(stats, 0, sizeof(ClockStats));
    
    /* Initialize ring 0 */
    for (int i = 0; i < 12; i++) {
        stats->ring0[i].ring = 0;
        stats->ring0[i].position = i;
    }
    
    /* Initialize ring 1 */
    for (int i = 0; i < 60; i++) {
        stats->ring1[i].ring = 1;
        stats->ring1[i].position = i;
    }
    
    /* Initialize ring 2 */
    for (int i = 0; i < 60; i++) {
        stats->ring2[i].ring = 2;
        stats->ring2[i].position = i;
    }
    
    /* Initialize ring 3 */
    for (int i = 0; i < 100; i++) {
        stats->ring3[i].ring = 3;
        stats->ring3[i].position = i;
    }
}

/**
 * @brief Add prime to statistics
 */
void add_prime_to_stats(ClockStats* stats, uint64_t prime, const ClockPosition* pos) {
    PositionStats* pstats = NULL;
    
    /* Get the appropriate position stats */
    if (pos->ring == 0 && pos->position < 12) {
        pstats = &stats->ring0[pos->position];
    } else if (pos->ring == 1 && pos->position < 60) {
        pstats = &stats->ring1[pos->position];
    } else if (pos->ring == 2 && pos->position < 60) {
        pstats = &stats->ring2[pos->position];
    } else if (pos->ring == 3 && pos->position < 100) {
        pstats = &stats->ring3[pos->position];
    }
    
    if (!pstats || pstats->count >= MAX_MAGNITUDE) {
        return;
    }
    
    /* Add prime */
    pstats->primes[pstats->count] = prime;
    
    /* Calculate delta if not first prime */
    if (pstats->count > 0) {
        pstats->deltas[pstats->count - 1] = prime - pstats->primes[pstats->count - 1];
    }
    
    pstats->count++;
    stats->total_primes_analyzed++;
}

/**
 * @brief Calculate statistics for a position
 */
void calculate_position_stats(PositionStats* pstats) {
    if (pstats->count == 0) {
        return;
    }
    
    /* Calculate average delta */
    uint64_t sum = 0;
    for (uint32_t i = 0; i < pstats->count - 1; i++) {
        sum += pstats->deltas[i];
    }
    
    if (pstats->count > 1) {
        pstats->avg_delta = (double)sum / (pstats->count - 1);
    }
    
    /* Calculate density (primes per unit magnitude) */
    if (pstats->count > 0) {
        uint64_t range = pstats->primes[pstats->count - 1] - pstats->primes[0];
        pstats->density = (double)pstats->count / (range + 1);
    }
}

/**
 * @brief Print statistics for a ring
 */
void print_ring_stats(const char* ring_name, PositionStats* positions, uint32_t count) {
    printf("\n=== %s Statistics ===\n", ring_name);
    printf("Position | Count | First Prime | Last Prime | Avg Delta | Density\n");
    printf("---------|-------|-------------|------------|-----------|--------\n");
    
    for (uint32_t i = 0; i < count; i++) {
        if (positions[i].count > 0) {
            printf("%8u | %5u | %11lu | %10lu | %9.2f | %.6f\n",
                   positions[i].position,
                   positions[i].count,
                   positions[i].primes[0],
                   positions[i].primes[positions[i].count - 1],
                   positions[i].avg_delta,
                   positions[i].density);
        }
    }
}

/**
 * @brief Analyze patterns and derive formula
 */
void analyze_patterns(ClockStats* stats) {
    printf("\n=== Pattern Analysis ===\n\n");
    
    /* Analyze Ring 0 (mod 12 patterns) */
    printf("Ring 0 Analysis (mod 12):\n");
    for (int i = 0; i < 12; i++) {
        if (stats->ring0[i].count > 2) {
            printf("  Position %2d: ", i);
            printf("Primes = ");
            for (uint32_t j = 0; j < (stats->ring0[i].count < 5 ? stats->ring0[i].count : 5); j++) {
                printf("%lu ", stats->ring0[i].primes[j]);
            }
            if (stats->ring0[i].count > 5) printf("...");
            printf("\n");
            
            /* Check for arithmetic progression */
            if (stats->ring0[i].count > 2) {
                uint64_t d1 = stats->ring0[i].deltas[0];
                uint64_t d2 = stats->ring0[i].deltas[1];
                if (d1 == d2) {
                    printf("    → Arithmetic progression with delta = %lu\n", d1);
                } else {
                    printf("    → Average delta = %.2f\n", stats->ring0[i].avg_delta);
                }
            }
        }
    }
    
    /* Look for formula patterns */
    printf("\n=== Formula Discovery ===\n\n");
    
    /* For each position with multiple primes, try to find the pattern */
    for (int i = 0; i < 12; i++) {
        if (stats->ring0[i].count >= 3) {
            printf("Position %d (mod 12 = %lu):\n", i, stats->ring0[i].primes[0] % 12);
            
            /* Try formula: p_n = base + n * delta */
            uint64_t base = stats->ring0[i].primes[0];
            uint64_t delta = stats->ring0[i].deltas[0];
            
            printf("  Testing formula: p_n = %lu + n * %lu\n", base, delta);
            
            bool formula_works = true;
            for (uint32_t j = 0; j < stats->ring0[i].count; j++) {
                uint64_t predicted = base + j * delta;
                uint64_t actual = stats->ring0[i].primes[j];
                
                if (predicted != actual) {
                    formula_works = false;
                    printf("    ✗ n=%u: predicted=%lu, actual=%lu (diff=%ld)\n",
                           j, predicted, actual, (int64_t)(actual - predicted));
                }
            }
            
            if (formula_works) {
                printf("    ✓ Formula works perfectly!\n");
            } else {
                /* Try to find correction factor */
                printf("    → Need correction factor\n");
                
                /* Analyze the differences */
                printf("    Corrections needed: ");
                for (uint32_t j = 0; j < stats->ring0[i].count; j++) {
                    uint64_t predicted = base + j * delta;
                    uint64_t actual = stats->ring0[i].primes[j];
                    int64_t correction = (int64_t)(actual - predicted);
                    printf("%ld ", correction);
                }
                printf("\n");
            }
        }
    }
}

/**
 * @brief Generate correction table
 */
void generate_correction_table(ClockStats* stats, const char* output_file) {
    FILE* fp = fopen(output_file, "w");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
        return;
    }
    
    fprintf(fp, "/* Auto-generated correction table for deterministic prime generation */\n\n");
    fprintf(fp, "#ifndef CLOCK_CORRECTION_TABLE_H\n");
    fprintf(fp, "#define CLOCK_CORRECTION_TABLE_H\n\n");
    fprintf(fp, "#include <stdint.h>\n\n");
    
    /* Ring 0 corrections */
    fprintf(fp, "/* Ring 0 (mod 12) correction factors */\n");
    fprintf(fp, "static const struct {\n");
    fprintf(fp, "    uint64_t base;      /* First prime at this position */\n");
    fprintf(fp, "    uint64_t delta;     /* Average spacing */\n");
    fprintf(fp, "    double density;     /* Prime density */\n");
    fprintf(fp, "} ring0_corrections[12] = {\n");
    
    for (int i = 0; i < 12; i++) {
        if (stats->ring0[i].count > 0) {
            fprintf(fp, "    {%lu, %lu, %.6f},  /* Position %d */\n",
                    stats->ring0[i].primes[0],
                    (uint64_t)stats->ring0[i].avg_delta,
                    stats->ring0[i].density,
                    i);
        } else {
            fprintf(fp, "    {0, 0, 0.0},  /* Position %d (no primes) */\n", i);
        }
    }
    
    fprintf(fp, "};\n\n");
    fprintf(fp, "#endif /* CLOCK_CORRECTION_TABLE_H */\n");
    
    fclose(fp);
    printf("\nCorrection table written to %s\n", output_file);
}

int main(int argc, char** argv) {
    printf("Clock Lattice Pattern Analyzer\n");
    printf("==============================\n\n");
    
    /* Initialize clock context */
    ClockContext clock_ctx;
    MathError err = clock_init(&clock_ctx);
    if (err != MATH_SUCCESS) {
        fprintf(stderr, "Error: Failed to initialize clock context\n");
        return 1;
    }
    
    /* Initialize statistics */
    ClockStats stats;
    init_stats(&stats);
    
    /* Analyze first N primes */
    uint64_t max_prime = (argc > 1) ? atoll(argv[1]) : 10000;
    printf("Analyzing primes up to %lu...\n", max_prime);
    
    /* Use hardcoded primes for initial analysis */
    uint64_t test_primes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
        157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
        239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
        331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
        421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503,
        509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
        613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701,
        709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811,
        821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911,
        919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997
    };
    size_t num_test_primes = sizeof(test_primes) / sizeof(test_primes[0]);
    
    printf("Analyzing %zu hardcoded primes...\n", num_test_primes);
    
    for (size_t i = 0; i < num_test_primes; i++) {
        uint64_t p = test_primes[i];
        if (p > max_prime) break;
        
        ClockPosition pos;
        MathError err = clock_map_prime_to_position(p, &pos);
        if (err == MATH_SUCCESS) {
            add_prime_to_stats(&stats, p, &pos);
            if (i < 10) {
                printf("  Prime %lu → ring=%u, pos=%u\n", p, pos.ring, pos.position);
            }
        } else {
            printf("Warning: Failed to map prime %lu to position (error %d)\n", p, err);
        }
    }
    printf("Successfully mapped primes\n");
    
    printf("Total primes analyzed: %lu\n", stats.total_primes_analyzed);
    
    /* Calculate statistics for all positions */
    for (int i = 0; i < 12; i++) calculate_position_stats(&stats.ring0[i]);
    for (int i = 0; i < 60; i++) calculate_position_stats(&stats.ring1[i]);
    for (int i = 0; i < 60; i++) calculate_position_stats(&stats.ring2[i]);
    for (int i = 0; i < 100; i++) calculate_position_stats(&stats.ring3[i]);
    
    /* Print statistics */
    print_ring_stats("Ring 0 (Hours, mod 12)", stats.ring0, 12);
    print_ring_stats("Ring 1 (Minutes, mod 60)", stats.ring1, 60);
    
    /* Analyze patterns */
    analyze_patterns(&stats);
    
    /* Generate correction table */
    const char* output_file = (argc > 2) ? argv[2] : "clock_correction_table.h";
    generate_correction_table(&stats, output_file);
    
    /* Cleanup */
    clock_cleanup(&clock_ctx);
    
    printf("\nAnalysis complete!\n");
    return 0;
}