#ifndef PRIME_MATH_H
#define PRIME_MATH_H

/*
 * prime_math.h - Auto-generated header file
 * Source: prime_math.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "prime_math.h"

/* Function declarations */
void prime_math_init_depth(int depth);
void prime_math_init(void);
void prime_math_cleanup(void);
bool prime_math_is_initialized(void);
int prime_math_get_depth(void);
void prime_math_extend_cache(int new_limit);
void big_prime_math_extend_cache(BigInt *new_limit);

/* Basic utility functions */
double prime_fabs(double x);
int prime_isnan(double x);
int prime_isinf(double x);

/* Fixed-point conversion functions */
uint64_t double_to_fixed(double x);
double fixed_to_double(uint64_t x);
double fixed_to_double_signed(int64_t x);

/* BigInt conversion functions */
double bigint_to_double(const BigInt *n);
int bigint_to_int(const BigInt *n);
uint64_t bigint_to_uint64(const BigInt *n);

/* BigInt arithmetic operations (use big_add, big_sub, etc. from bigint_core.h) */

#endif /* PRIME_MATH_H */