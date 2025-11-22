#ifndef BIGINT_ADVANCED_H
#define BIGINT_ADVANCED_H

#include "bigint_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * bigint_advanced.h - Advanced Number Theory Algorithms
 * 
 * Revolutionary algorithms for the Crystalline Lattice Math Library.
 * All algorithms implemented WITHOUT external dependencies.
 * 
 * Categories:
 * 1. Modular Arithmetic
 * 2. Primality Testing
 * 3. Factorization
 * 4. Number Theory Functions
 */

/* ============================================================================
 * MODULAR ARITHMETIC
 * ============================================================================ */

/**
 * Modular exponentiation: result = (base^exp) mod m
 * 
 * Uses binary exponentiation (square-and-multiply method).
 * 
 * Algorithm: O(log exp × log² m)
 * 
 * @param result Output: (base^exp) mod m
 * @param base Base value
 * @param exp Exponent
 * @param m Modulus
 */
void big_mod_exp(BigInt* result, const BigInt* base, const BigInt* exp, const BigInt* m);

/**
 * Modular inverse: result = a^(-1) mod m
 * 
 * Finds x such that (a × x) ≡ 1 (mod m)
 * Uses extended Euclidean algorithm.
 * 
 * @param result Output: modular inverse
 * @param a Input value
 * @param m Modulus
 * @return 1 if inverse exists (gcd(a,m)=1), 0 otherwise
 */
int big_mod_inverse(BigInt* result, const BigInt* a, const BigInt* m);

/* ============================================================================
 * EXTENDED EUCLIDEAN ALGORITHM
 * ============================================================================ */

/**
 * Extended Euclidean Algorithm
 * 
 * Finds gcd(a, b) and coefficients x, y such that:
 * ax + by = gcd(a, b)
 * 
 * This is the foundation for:
 * - Modular inverse
 * - Solving linear Diophantine equations
 * - RSA key generation
 * - Continued fractions
 * 
 * Algorithm: O(log min(a,b))
 * 
 * @param a First number
 * @param b Second number
 * @param gcd Output: gcd(a, b)
 * @param x Output: coefficient for a (can be NULL)
 * @param y Output: coefficient for b (can be NULL)
 */
void big_extended_gcd(const BigInt* a, const BigInt* b, 
                      BigInt* gcd, BigInt* x, BigInt* y);

/* ============================================================================
 * PRIMALITY TESTING
 * ============================================================================ */

/**
 * Miller-Rabin Primality Test
 * 
 * Probabilistic primality test with error probability ≤ 4^(-k)
 * where k is the number of iterations.
 * 
 * For k=20: error probability < 10^(-12)
 * For k=40: error probability < 10^(-24)
 * 
 * Algorithm:
 * 1. Write n-1 as 2^r × d where d is odd
 * 2. For k iterations:
 *    - Pick random a in [2, n-2]
 *    - Compute x = a^d mod n
 *    - If x = 1 or x = n-1, continue
 *    - Square x repeatedly r-1 times
 *    - If x ≠ n-1 at any point, n is composite
 * 3. If all iterations pass, n is probably prime
 * 
 * Complexity: O(k log³ n)
 * 
 * This is MUCH faster than trial division:
 * - Trial division: O(√n)
 * - Miller-Rabin: O(k log³ n)
 * 
 * For n = 10^100:
 * - Trial division: ~10^50 operations
 * - Miller-Rabin (k=20): ~60,000 operations
 * 
 * @param n Number to test
 * @param iterations Number of test iterations (typically 20-40)
 * @return 1 if probably prime, 0 if definitely composite
 */
int big_miller_rabin(const BigInt* n, int iterations);

/* ============================================================================
 * FACTORIZATION
 * ============================================================================ */

/**
 * Pollard's Rho Algorithm for Integer Factorization
 * 
 * Finds a non-trivial factor of n using cycle detection.
 * 
 * Algorithm:
 * 1. Choose function f(x) = (x² + c) mod n
 * 2. Generate sequence: x₀, x₁=f(x₀), x₂=f(x₁), ...
 * 3. Use Floyd's cycle detection (tortoise and hare)
 * 4. Compute gcd(|xₜₒᵣₜₒᵢₛₑ - xₕₐᵣₑ|, n)
 * 5. If gcd > 1 and gcd < n, found factor
 * 
 * Expected complexity: O(n^(1/4))
 * 
 * This is MUCH faster than trial division:
 * - Trial division: O(√n) = O(n^(1/2))
 * - Pollard's Rho: O(n^(1/4))
 * 
 * For n = 10^20:
 * - Trial division: ~10^10 operations
 * - Pollard's Rho: ~10^5 operations
 * 
 * @param n Number to factor
 * @param factor Output: found factor (if any)
 * @return 1 if factor found, 0 otherwise
 */
int big_pollard_rho(const BigInt* n, BigInt* factor);

/**
 * Complete factorization using Pollard's Rho
 * 
 * Factors n completely into prime factors.
 * 
 * @param n Number to factor
 * @param factors Output: array of prime factors
 * @param exponents Output: array of exponents
 * @param num_factors Output: number of distinct prime factors
 * @return 1 if successful, 0 otherwise
 */
int big_factor_complete(const BigInt* n, BigInt** factors, 
                        int** exponents, int* num_factors);

/* ============================================================================
 * CHINESE REMAINDER THEOREM
 * ============================================================================ */

/**
 * Chinese Remainder Theorem
 * 
 * Solves system of congruences:
 * x ≡ a₁ (mod m₁)
 * x ≡ a₂ (mod m₂)
 * ...
 * x ≡ aₙ (mod mₙ)
 * 
 * where m₁, m₂, ..., mₙ are pairwise coprime.
 * 
 * Algorithm:
 * 1. Compute M = m₁ × m₂ × ... × mₙ
 * 2. For each i: Mᵢ = M / mᵢ
 * 3. For each i: yᵢ = Mᵢ^(-1) mod mᵢ
 * 4. Result: x = Σ(aᵢ × Mᵢ × yᵢ) mod M
 * 
 * Applications:
 * - RSA with multiple primes
 * - Fast modular arithmetic
 * - Solving systems of modular equations
 * 
 * Complexity: O(n² log² M)
 * 
 * @param result Output: solution x
 * @param remainders Array of remainders [a₁, a₂, ..., aₙ]
 * @param moduli Array of moduli [m₁, m₂, ..., mₙ]
 * @param count Number of congruences
 * @return 1 if successful, 0 if moduli not coprime
 */
int big_crt(BigInt* result, const BigInt* remainders, 
            const BigInt* moduli, int count);

/* ============================================================================
 * NUMBER THEORY FUNCTIONS
 * ============================================================================ */

/**
 * Euler's Totient Function φ(n)
 * 
 * Counts integers k in [1, n] such that gcd(k, n) = 1
 * (i.e., counts numbers coprime to n)
 * 
 * Properties:
 * - φ(1) = 1
 * - φ(p) = p - 1 for prime p
 * - φ(p^k) = p^(k-1) × (p - 1)
 * - φ(mn) = φ(m) × φ(n) if gcd(m,n) = 1
 * 
 * Formula: φ(n) = n × ∏(1 - 1/pᵢ) for prime factors pᵢ
 * 
 * Applications:
 * - RSA encryption (key generation)
 * - Fermat's little theorem
 * - Euler's theorem
 * - Counting primitive roots
 * 
 * Complexity: O(√n) with trial division
 * 
 * @param result Output: φ(n)
 * @param n Input number
 */
void big_euler_phi(BigInt* result, const BigInt* n);

/**
 * Carmichael Function λ(n)
 * 
 * Smallest positive integer m such that a^m ≡ 1 (mod n)
 * for all a coprime to n.
 * 
 * Properties:
 * - λ(n) divides φ(n)
 * - λ(p^k) = φ(p^k) for odd prime p
 * - λ(2^k) = φ(2^k)/2 for k ≥ 3
 * - λ(mn) = lcm(λ(m), λ(n)) if gcd(m,n) = 1
 * 
 * Applications:
 * - RSA encryption (optimal exponent)
 * - Carmichael numbers
 * - Universal exponent
 * 
 * @param result Output: λ(n)
 * @param n Input number
 */
void big_carmichael(BigInt* result, const BigInt* n);

/**
 * Jacobi Symbol (a/n)
 * 
 * Generalization of Legendre symbol to composite n.
 * 
 * Properties:
 * - (a/n) ∈ {-1, 0, 1}
 * - (a/n) = 0 if gcd(a,n) > 1
 * - Multiplicative in both arguments
 * 
 * Applications:
 * - Primality testing (Solovay-Strassen)
 * - Quadratic residues
 * - Cryptography
 * 
 * Complexity: O(log² n)
 * 
 * @param a Numerator
 * @param n Denominator (must be odd)
 * @return Jacobi symbol value: -1, 0, or 1
 */
int big_jacobi(const BigInt* a, const BigInt* n);

/**
 * Legendre Symbol (a/p)
 * 
 * Determines if a is a quadratic residue modulo prime p.
 * 
 * Returns:
 * - 1 if a is a quadratic residue mod p
 * - -1 if a is a quadratic non-residue mod p
 * - 0 if a ≡ 0 (mod p)
 * 
 * Formula: (a/p) ≡ a^((p-1)/2) (mod p)
 * 
 * @param a Value to test
 * @param p Prime modulus
 * @return Legendre symbol value: -1, 0, or 1
 */
int big_legendre(const BigInt* a, const BigInt* p);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * Check if n is a perfect power: n = a^b for some a, b > 1
 * 
 * @param n Number to test
 * @param base Output: base a (if perfect power)
 * @param exponent Output: exponent b (if perfect power)
 * @return 1 if perfect power, 0 otherwise
 */
int big_is_perfect_power(const BigInt* n, BigInt* base, int* exponent);

/**
 * Compute nth root: result = ⌊n^(1/k)⌋
 * 
 * Uses Newton's method.
 * 
 * @param result Output: kth root of n
 * @param n Input number
 * @param k Root degree
 */
void big_nth_root(BigInt* result, const BigInt* n, int k);

/**
 * Check if n is a perfect square
 * 
 * @param n Number to test
 * @param root Output: square root (if perfect square)
 * @return 1 if perfect square, 0 otherwise
 */
int big_is_perfect_square(const BigInt* n, BigInt* root);

#ifdef __cplusplus
}
#endif

#endif // BIGINT_ADVANCED_H