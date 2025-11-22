#ifndef PRIME_LOWLEVEL_H
#define PRIME_LOWLEVEL_H

/*
 * prime_lowlevel.h - Auto-generated header file
 * Source: prime_lowlevel.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "../include/prime_lowlevel.h"

/* Function declarations */
uint64_t prime_sqrt_int(uint64_t n);
uint32_t prime_log2_int(uint64_t n);
uint64_t prime_log_int(uint64_t n, uint64_t base);

/* Math utility functions */
double prime_fabs(double x);
int prime_isnan(double x);
int prime_isinf(double x);
uint64_t prime_pow_int(uint64_t base, uint64_t exp);
uint64_t prime_powmod_int(uint64_t base, uint64_t exp, uint64_t m);
uint64_t prime_gcd(uint64_t a, uint64_t b);
uint64_t prime_lcm(uint64_t a, uint64_t b);
uint64_t prime_gcd_extended(uint64_t a, uint64_t b, int64_t *x, int64_t *y);
uint64_t prime_modinv(uint64_t a, uint64_t m);
uint64_t prime_fixedpoint_mul(uint64_t a, uint64_t b);
uint64_t prime_fixedpoint_div(uint64_t a, uint64_t b);
uint64_t prime_fixedpoint_sqrt(uint64_t n);
uint64_t prime_pi_fixed(void);
uint64_t prime_phi_fixed(void);
uint64_t prime_e_fixed(void);
uint64_t prime_sqrt5_fixed(void);
void prime_cordic_sincos(uint64_t angle, int64_t *sin_result, int64_t *cos_result);
uint64_t prime_exp_fixed(int64_t x);
uint64_t prime_abs_int(int64_t x);
uint64_t prime_min_int(uint64_t a, uint64_t b);
uint64_t prime_max_int(uint64_t a, uint64_t b);
uint64_t prime_clamp_int(uint64_t value, uint64_t min, uint64_t max);
int prime_is_power_of_2(uint64_t n);
uint64_t prime_next_power_of_2(uint64_t n);

#endif /* PRIME_LOWLEVEL_H */