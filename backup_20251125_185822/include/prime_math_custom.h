#ifndef PRIME_MATH_CUSTOM_H
#define PRIME_MATH_CUSTOM_H

/*
 * prime_math_custom.h - Auto-generated header file
 * Source: prime_math_custom.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "prime_basic.h"
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "../include/prime_math_custom.h"
#include "../include/prime_lowlevel.h"

/* Function declarations */
double prime_sqrt(double x);
double prime_exp(double x);
double prime_log(double x);
double prime_log10(double x);
double prime_log2(double x);
double prime_pow(double x, double y);
double prime_sin(double x);
double prime_cos(double x);
double prime_tan(double x);
double prime_asin(double x);
double prime_acos(double x);
double prime_atan(double x);
double prime_atan2(double y, double x);
double prime_sinh(double x);
double prime_cosh(double x);
double prime_tanh(double x);
double prime_floor(double x);
double prime_ceil(double x);
double prime_round(double x);
double prime_trunc(double x);
double prime_fmod(double x, double y);
double prime_remainder(double x, double y);
double prime_fmax(double a, double b);
double prime_fmin(double a, double b);

/* BigInt conversion functions */
double bigint_to_double(const BigInt *n);
int bigint_to_int(const BigInt *n);
uint64_t bigint_to_uint64(const BigInt *n);
char* big_to_string(const BigInt *n);

#endif /* PRIME_MATH_CUSTOM_H */