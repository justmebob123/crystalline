#ifndef PRIME_RAINBOW_H
#define PRIME_RAINBOW_H

/*
 * prime_rainbow.h - Auto-generated header file
 * Source: prime_rainbow.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>
#include "prime_types.h"
   #include "clock_lattice.h"

/* Local includes */
#include "prime_rainbow.h"
#include "crystal_abacus.h"
#include "../include/prime_math_custom.h"

/* Function declarations */
void rainbow_table_init(void);
void rainbow_table_cleanup(void);
double fast_prime_angle(int prime_index);
double fast_prime_radius(int prime_index);
double fast_prime_frequency(int prime_index);
int fast_prime_layer(int prime_index);
void fast_prime_fold_coords(int prime_index, double* x, double* y, double* z);

   // Helper functions using clock lattice
   PrimeModular fast_prime_modular(uint64_t prime);
   bool fast_prime_is_sacred(int prime_index);
   int fast_prime_position(int prime_index);
double big_fast_prime_angle(BigInt *prime);
double big_fast_prime_radius(BigInt *prime);
int big_fast_prime_layer(BigInt *prime);

// Prime storage functions - Rainbow Table as Complete Abacus
PrimeRainbowTable* rainbow_table_get(void);  // Get the global rainbow table instance
int rainbow_table_add_prime(BigInt* prime);
BigInt* rainbow_table_get_prime(int index);
int rainbow_table_get_count(void);
int rainbow_table_generate_primes(int target_count);
int rainbow_table_load_important_primes(void);

// Analysis functions
double rainbow_table_check_stability(PrimeRainbowTable* table);
double rainbow_table_self_similarity(PrimeRainbowTable* table);

#endif /* PRIME_RAINBOW_H */