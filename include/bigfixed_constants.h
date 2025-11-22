#ifndef BIGFIXED_CONSTANTS_H
#define BIGFIXED_CONSTANTS_H

/*
 * bigfixed_constants.h - Auto-generated header file
 * Source: bigfixed_constants.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "prime_bigint_transcendental.h"
#include "bigfixed_core.h"

/* Type definitions */
typedef struct {
    BigFixed *value;
    int precision_bits;
    bool computed;
} CachedConstant;

/* Function declarations */
void big_sqrt(BigFixed *result, const BigInt *n, int precision_bits);
void big_pi(BigFixed *result, int precision_bits);
void big_e(BigFixed *result, int precision_bits);
void big_phi(BigFixed *result, int precision_bits);
void big_ln2(BigFixed *result, int precision_bits);
void big_ln3(BigFixed *result, int precision_bits);
void big_ln10(BigFixed *result, int precision_bits);

#endif /* BIGFIXED_CONSTANTS_H */