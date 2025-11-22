#ifndef BIGFIXED_CORE_H
#define BIGFIXED_CORE_H

/*
 * BigFixed Core - Arbitrary Precision Fixed-Point Operations
 * 
 * CRITICAL: This library uses ARBITRARY PRECISION for all calculations.
 * BigFixed provides exact fractional arithmetic.
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>

/* Core type definitions */
#include "prime_types.h"

/* Function declarations */
BigFixed* big_fixed_create(int scale_bits);
void big_fixed_free(BigFixed *f);
BigFixed* big_fixed_copy(const BigFixed *f);
void big_fixed_assign(BigFixed *dest, const BigFixed *src);
void big_fixed_from_bigint(BigFixed *result, const BigInt *n);
void big_fixed_from_int(BigFixed *result, int64_t n);
void big_fixed_to_bigint(BigInt *result, const BigFixed *f);
void big_fixed_to_bigint_rounded(BigInt *result, const BigFixed *f);
int big_fixed_cmp(const BigFixed *a, const BigFixed *b);
bool big_fixed_is_zero(const BigFixed *f);
bool big_fixed_is_negative(const BigFixed *f);
void big_fixed_add(BigFixed *result, const BigFixed *a, const BigFixed *b);
void big_fixed_sub(BigFixed *result, const BigFixed *a, const BigFixed *b);
void big_fixed_mul(BigFixed *result, const BigFixed *a, const BigFixed *b);
void big_fixed_div(BigFixed *result, const BigFixed *a, const BigFixed *b);
void big_fixed_abs(BigFixed *result, const BigFixed *f);
void big_fixed_neg(BigFixed *result, const BigFixed *f);
void big_fixed_lshift(BigFixed *result, const BigFixed *f, int bits);
void big_fixed_rshift(BigFixed *result, const BigFixed *f, int bits);
void big_fixed_floor(BigInt *result, const BigFixed *f);
void big_fixed_ceil(BigInt *result, const BigFixed *f);
void big_fixed_round(BigInt *result, const BigFixed *f);
void big_fixed_trunc(BigInt *result, const BigFixed *f);
void big_fixed_frac(BigFixed *result, const BigFixed *f);
char* big_fixed_to_string(const BigFixed *f, int decimal_places);

#endif /* BIGFIXED_CORE_H */