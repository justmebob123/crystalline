/*
 * prime_math_custom.c - Implementation of custom math library
 * 
 * All functions moved to appropriate modules to avoid conflicts:
 * - Fixed-point functions: prime_lowlevel.c
 * - Double precision functions: prime_float_math.c  
 * - BigInt conversions: bigint_conversions.c, bigint_core.c
 * - Basic utilities: prime_basic.c
 * 
 * This file now serves as a compatibility stub only.
 */

#include "../include/prime_math_custom.h"
#include "../include/prime_lowlevel.h"
#include <stdint.h>
#include <stdio.h>

// All functions implemented elsewhere - this file is now empty
// to prevent multiple definition conflicts during linking.
