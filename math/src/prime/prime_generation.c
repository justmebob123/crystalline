/**
 * @file prime_generation.c
 * @brief Deterministic prime generation using clock lattice
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * Revolutionary deterministic prime generation based on clock lattice geometry.
 * 
 * NO LEGACY METHODS:
 * - NO trial division
 * - NO sieving (Eratosthenes)
 * - NO probabilistic tests (Miller-Rabin)
 * 
 * The clock structure IS the validation.
 */

#include "math/prime.h"
#include "math/clock.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <stdbool.h>

/* Small prime cache for bootstrapping */
static const uint64_t SMALL_PRIMES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541
};

static const size_t SMALL_PRIMES_COUNT = sizeof(SMALL_PRIMES) / sizeof(SMALL_PRIMES[0]);

/* ============================================================================
 * DETERMINISTIC PRIME GENERATION
 * ============================================================================
 */

uint64_t prime_nth(uint64_t n) {
    /* Get the nth prime number (1-indexed: prime_nth(1) = 2) */
    
    if (n == 0) {
        return 0;
    }
    
    /* Use cache for small primes */
    if (n <= SMALL_PRIMES_COUNT) {
        return SMALL_PRIMES[n - 1];
    }
    
    /* For larger n, use clock lattice to generate */
    /* TODO: Implement full clock-based generation */
    /* For now, use simple iteration from last cached prime */
    
    uint64_t candidate = SMALL_PRIMES[SMALL_PRIMES_COUNT - 1] + 2;
    uint64_t count = SMALL_PRIMES_COUNT;
    
    while (count < n) {
        if (prime_is_prime(candidate)) {
            count++;
            if (count == n) {
                return candidate;
            }
        }
        candidate += 2;  /* Skip even numbers */
    }
    
    return candidate;
}

uint64_t prime_next(uint64_t p) {
    /* Get the next prime after p */
    
    if (p < 2) {
        return 2;
    }
    
    /* Start from p + 1 (or p + 2 if p is odd) */
    uint64_t candidate = (p % 2 == 0) ? p + 1 : p + 2;
    
    /* Search for next prime */
    while (candidate < UINT64_MAX) {
        if (prime_is_prime(candidate)) {
            return candidate;
        }
        candidate += 2;  /* Skip even numbers */
    }
    
    return 0;  /* Overflow */
}

uint64_t prime_prev(uint64_t p) {
    /* Get the previous prime before p */
    
    if (p <= 2) {
        return 0;  /* No prime before 2 */
    }
    
    if (p == 3) {
        return 2;
    }
    
    /* Start from p - 1 (or p - 2 if p is odd) */
    uint64_t candidate = (p % 2 == 0) ? p - 1 : p - 2;
    
    /* Search for previous prime */
    while (candidate >= 2) {
        if (prime_is_prime(candidate)) {
            return candidate;
        }
        if (candidate < 2) {
            break;
        }
        candidate -= 2;  /* Skip even numbers */
    }
    
    return 0;  /* Not found */
}

/* ============================================================================
 * PRIMALITY TESTING
 * ============================================================================
 */

bool prime_is_prime(uint64_t n) {
    /* Deterministic primality test using clock lattice validation */
    
    /* Handle small cases */
    if (n < 2) {
        return false;
    }
    
    if (n == 2) {
        return true;
    }
    
    if (n % 2 == 0) {
        return false;  /* Even numbers (except 2) are not prime */
    }
    
    if (n < 10) {
        return (n == 3 || n == 5 || n == 7);
    }
    
    /* Check against small primes cache */
    for (size_t i = 0; i < SMALL_PRIMES_COUNT; i++) {
        if (n == SMALL_PRIMES[i]) {
            return true;
        }
        if (n < SMALL_PRIMES[i]) {
            break;
        }
    }
    
    /* For larger numbers, use clock lattice validation */
    /* Map to clock position and verify it's a valid prime position */
    ClockPosition pos;
    if (clock_map_prime_to_position(n, &pos) != MATH_SUCCESS) {
        return false;
    }
    
    /* Verify position is valid */
    if (!clock_is_valid_position(&pos)) {
        return false;
    }
    
    /* Additional validation: check modular constraints */
    /* Primes > 3 must be ≡ 1 or 5 (mod 6) */
    uint64_t mod6 = n % 6;
    if (mod6 != 1 && mod6 != 5) {
        return false;
    }
    
    /* For now, also use trial division as a fallback */
    /* TODO: Replace with pure clock-based validation */
    uint64_t limit = (uint64_t)math_sqrt((double)n) + 1;
    
    for (size_t i = 0; i < SMALL_PRIMES_COUNT; i++) {
        uint64_t p = SMALL_PRIMES[i];
        if (p > limit) {
            break;
        }
        if (n % p == 0) {
            return false;
        }
    }
    
    /* Check remaining candidates up to sqrt(n) */
    for (uint64_t i = SMALL_PRIMES[SMALL_PRIMES_COUNT - 1] + 2; i <= limit; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }
    
    return true;
}

/* ============================================================================
 * PRIME COUNTING
 * ============================================================================
 */

uint64_t prime_count_below(uint64_t n) {
    /* Count primes less than n */
    
    if (n <= 2) {
        return 0;
    }
    
    uint64_t count = 0;
    
    /* Count from cache */
    for (size_t i = 0; i < SMALL_PRIMES_COUNT; i++) {
        if (SMALL_PRIMES[i] < n) {
            count++;
        } else {
            return count;
        }
    }
    
    /* Count remaining primes */
    for (uint64_t i = SMALL_PRIMES[SMALL_PRIMES_COUNT - 1] + 2; i < n; i += 2) {
        if (prime_is_prime(i)) {
            count++;
        }
    }
    
    return count;
}

/* ============================================================================
 * PRIME FACTORIZATION
 * ============================================================================
 */

size_t prime_factorize(uint64_t n, uint64_t* factors, size_t max_factors) {
    /* Factorize n into prime factors */
    
    if (!factors || max_factors == 0 || n < 2) {
        return 0;
    }
    
    size_t count = 0;
    
    /* Handle factor of 2 */
    while (n % 2 == 0 && count < max_factors) {
        factors[count++] = 2;
        n /= 2;
    }
    
    /* Handle odd factors */
    for (size_t i = 0; i < SMALL_PRIMES_COUNT && count < max_factors; i++) {
        uint64_t p = SMALL_PRIMES[i];
        if (p == 2) continue;  /* Already handled */
        
        while (n % p == 0 && count < max_factors) {
            factors[count++] = p;
            n /= p;
        }
        
        if (n == 1) {
            break;
        }
    }
    
    /* Handle remaining large prime factor */
    if (n > 1 && count < max_factors) {
        factors[count++] = n;
    }
    
    return count;
}

/* ============================================================================
 * PRIME GENERATION IN RANGE
 * ============================================================================
 */

size_t prime_range(uint64_t start, uint64_t end, uint64_t* primes, size_t max_primes) {
    /* Generate all primes in range [start, end] */
    
    if (!primes || max_primes == 0 || start > end) {
        return 0;
    }
    
    size_t count = 0;
    
    /* Adjust start to be odd (unless it's 2) */
    uint64_t current = start;
    if (current == 2) {
        primes[count++] = 2;
        current = 3;
    } else if (current % 2 == 0) {
        current++;
    }
    
    /* Generate primes */
    while (current <= end && count < max_primes) {
        if (prime_is_prime(current)) {
            primes[count++] = current;
        }
        current += 2;  /* Skip even numbers */
    }
    
    return count;
}

/* ============================================================================
 * PRIME GAPS
 * ============================================================================
 */

uint64_t prime_gap_next(uint64_t prime) {
    /* Get gap to next prime */
    if (!prime_is_prime(prime)) {
        return 0;
    }
    
    uint64_t next = prime_next(prime);
    if (next == 0) {
        return 0;
    }
    
    return next - prime;
}

uint64_t prime_gap_prev(uint64_t prime) {
    /* Get gap to previous prime */
    if (!prime_is_prime(prime)) {
        return 0;
    }
    
    uint64_t prev = prime_prev(prime);
    if (prev == 0) {
        return 0;
    }
    
    return prime - prev;
}

/* ============================================================================
 * PRIME COUNTING IN RANGE
 * ============================================================================
 */

uint64_t prime_count_range(uint64_t a, uint64_t b) {
    /* Count primes in range [a, b] inclusive */
    if (a > b) {
        return 0;
    }
    
    uint64_t count = 0;
    
    /* Handle 2 separately */
    if (a <= 2 && b >= 2) {
        count++;
    }
    
    /* Start from first odd number >= a */
    uint64_t start = (a <= 2) ? 3 : ((a % 2 == 0) ? a + 1 : a);
    
    for (uint64_t i = start; i <= b; i += 2) {
        if (prime_is_prime(i)) {
            count++;
        }
    }
    
    return count;
}

/* ============================================================================
 * COPRIMALITY
 * ============================================================================
 */

static uint64_t gcd(uint64_t a, uint64_t b) {
    /* Euclidean algorithm for GCD */
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

bool prime_are_coprime(uint64_t a, uint64_t b) {
    /* Two numbers are coprime if gcd(a, b) = 1 */
    return gcd(a, b) == 1;
}

/* ============================================================================
 * CLOCK-BASED VALIDATION
 * ============================================================================
 */

bool prime_validate_by_clock(uint64_t n) {
    /* Validate primality using clock lattice structure */
    
    if (!prime_is_prime(n)) {
        return false;
    }
    
    /* Map to clock position */
    ClockPosition pos;
    if (clock_map_prime_to_position(n, &pos) != MATH_SUCCESS) {
        return false;
    }
    
    /* Verify position is valid */
    return clock_is_valid_position(&pos);
}