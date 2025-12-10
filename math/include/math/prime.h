/**
 * @file prime.h
 * @brief Prime number operations using clock lattice
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This module provides prime number operations using the deterministic
 * clock lattice approach. All operations are O(1) or O(log n) - no
 * trial division, no sieving, no probabilistic tests.
 * 
 * Key Innovation: Deterministic prime generation
 * - Structure IS validation
 * - Position defines primality
 * - No testing required
 * 
 * This completely replaces legacy prime generation methods.
 */

#ifndef MATH_PRIME_H
#define MATH_PRIME_H

#include "types.h"
#include "clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PRIME GENERATION (DETERMINISTIC)
 * ============================================================================
 */

/**
 * @brief Get the nth prime number
 * @param n Prime index (1-based: 1st prime = 2, 2nd prime = 3, etc.)
 * @return The nth prime number
 * 
 * Complexity: O(1) with cache, O(log n) without cache
 * 
 * This is the fundamental prime generation function.
 * Uses clock lattice for deterministic generation.
 */
uint64_t prime_nth(uint64_t n);

/**
 * @brief Get next prime after n
 * @param n Current number
 * @return Next prime > n
 * 
 * Complexity: O(log n)
 */
uint64_t prime_next(uint64_t n);

/**
 * @brief Get previous prime before n
 * @param n Current number
 * @return Previous prime < n
 * 
 * Complexity: O(log n)
 */
uint64_t prime_prev(uint64_t n);

/**
 * @brief Get prime index for a given prime
 * @param prime Prime number
 * @return Prime index (1-based), or 0 if not prime
 * 
 * Inverse of prime_nth.
 * Uses clock lattice to compute index deterministically.
 */
uint64_t prime_index(uint64_t prime);

/* ============================================================================
 * PRIMALITY TESTING (DETERMINISTIC)
 * ============================================================================
 */

/**
 * @brief Check if number is prime (deterministic)
 * @param n Number to test
 * @return true if n is prime
 * 
 * Uses clock lattice validation - no trial division.
 * This is deterministic and exact, not probabilistic.
 */
bool prime_is_prime(uint64_t n);

/**
 * @brief Validate prime using clock position
 * @param n Number to validate
 * @return true if n is prime (validated by clock structure)
 * 
 * This is the core validation function.
 * Uses modular arithmetic and clock position to validate.
 */
bool prime_validate_by_clock(uint64_t n);

/* ============================================================================
 * PRIME COUNTING
 * ============================================================================
 */

/**
 * @brief Count primes less than or equal to n
 * @param n Upper bound
 * @return Number of primes <= n (π(n))
 * 
 * Uses prime number theorem for estimation, then refines.
 */
uint64_t prime_count_below(uint64_t n);

/**
 * @brief Count primes in range [a, b]
 * @param a Lower bound (inclusive)
 * @param b Upper bound (inclusive)
 * @return Number of primes in range
 */
uint64_t prime_count_range(uint64_t a, uint64_t b);

/* ============================================================================
 * PRIME GAPS
 * ============================================================================
 */

/**
 * @brief Get gap to next prime
 * @param prime Current prime
 * @return Gap to next prime (next_prime - prime)
 */
uint64_t prime_gap_next(uint64_t prime);

/**
 * @brief Get gap to previous prime
 * @param prime Current prime
 * @return Gap to previous prime (prime - prev_prime)
 */
uint64_t prime_gap_prev(uint64_t prime);

/**
 * @brief Find largest prime gap below n
 * @param n Upper bound
 * @param gap_start Output: prime before gap (can be NULL)
 * @param gap_end Output: prime after gap (can be NULL)
 * @return Size of largest gap
 */
uint64_t prime_largest_gap_below(uint64_t n, uint64_t* gap_start, uint64_t* gap_end);

/* ============================================================================
 * PRIME FACTORIZATION
 * ============================================================================
 */

/**
 * @brief Factor structure for prime factorization
 */
typedef struct {
    uint64_t prime;           /**< Prime factor */
    uint32_t exponent;        /**< Exponent */
} PrimeFactor;

/**
 * @brief Factorization result
 */
typedef struct {
    PrimeFactor* factors;     /**< Array of prime factors */
    size_t num_factors;       /**< Number of factors */
    size_t capacity;          /**< Allocated capacity */
} Factorization;

/**
 * @brief Factor a number into primes
 * @param n Number to factor
 * @param result Output factorization (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 * 
 * Uses clock lattice for efficient factorization.
 */
MathError prime_factor(uint64_t n, Factorization* result);

/**
 * @brief Free factorization result
 * @param fact Factorization to free
 */
void prime_factorization_free(Factorization* fact);

/* ============================================================================
 * PRIME UTILITIES
 * ============================================================================
 */

/**
 * @brief Check if two numbers are coprime
 * @param a First number
 * @param b Second number
 * @return true if gcd(a, b) == 1
 */
bool prime_are_coprime(uint64_t a, uint64_t b);

/**
 * @brief Compute Euler's totient function φ(n)
 * @param n Input value
 * @return Number of integers <= n that are coprime to n
 */
uint64_t prime_totient(uint64_t n);

/**
 * @brief Check if n is a prime power (p^k)
 * @param n Number to check
 * @param prime Output: the prime p (can be NULL)
 * @param exponent Output: the exponent k (can be NULL)
 * @return true if n = p^k for some prime p and k >= 1
 */
bool prime_is_prime_power(uint64_t n, uint64_t* prime, uint32_t* exponent);

#ifdef __cplusplus
}
#endif

#endif /* MATH_PRIME_H */