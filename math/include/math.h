/**
 * @file math.h
 * @brief Crystalline Math Library - Main Header
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This is the main header for the Crystalline Math Library.
 * Include this file to access all mathematical functions.
 * 
 * The Crystalline Math Library is a self-contained, production-quality
 * mathematical library designed to completely replace <math.h> and provide
 * arbitrary precision arithmetic.
 * 
 * Design Principles:
 * - Self-contained (no math.h dependencies)
 * - Arbitrary precision support (bigfixed, bigint)
 * - Deterministic prime generation (clock lattice)
 * - Clean, standardized API
 * - Production quality code
 * - Thread-safe operations
 * 
 * Module Organization:
 * - arithmetic.h: Basic double-precision arithmetic
 * - transcendental.h: Transcendental functions (sqrt, sin, cos, log, exp)
 * - bigfixed.h: Arbitrary precision fixed-point
 * - bigint.h: Arbitrary precision integer
 * - clock.h: Clock lattice for deterministic operations
 * - prime.h: Prime number operations
 
 * 
 * Usage:
 * 
 * Basic math:
 *   #include <math.h>  // OLD
 *   #include "math.h"  // NEW
 *   
 *   double y = sqrt(x);      // OLD
 *   double y = math_sqrt(x); // NEW
 * 
 * Arbitrary precision:
 *   BigFixed* x = bigfixed_from_double(2.0, 256);
 *   BigFixed* result = bigfixed_new(256);
 *   bigfixed_sqrt(result, x);
 * 
 * Prime numbers:
 *   uint64_t p = prime_nth(1000);  // Get 1000th prime
 *   bool is_prime = prime_is_prime(1009);
 */

#ifndef CRYSTALLINE_MATH_H
#define CRYSTALLINE_MATH_H

/* Core types */
#include "math/types.h"

/* Basic arithmetic */
#include "math/arithmetic.h"

/* Transcendental functions */
#include "math/transcendental.h"

/* Arbitrary precision */
#include "math/bigfixed.h"
#include "math/bigint.h"

/* Clock lattice and prime numbers */
#include "math/clock.h"
#include "math/prime.h"
#include "math/rainbow.h"

/* Crystalline Abacus */
#include "math/abacus.h"



#endif /* CRYSTALLINE_MATH_H */