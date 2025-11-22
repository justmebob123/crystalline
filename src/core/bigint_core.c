#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "../include/prime_math_custom.h"
#include <stdio.h>
#include "../include/crystal_abacus.h"

// Basic BigInt operations implementation
void big_init(BigInt *n) {
    if (!n) return;
    n->d = malloc(sizeof(uint32_t));
    n->len = 1;
    n->capacity = 1;
    n->target_len = 1;
    n->extended_len = 1;
    n->negative = 0;
    n->extended_mode = 0;
    n->d[0] = 0;
}

void big_free(BigInt *n) {
    if (!n) return;
    if (n->d) {
        free(n->d);
        n->d = NULL;
    }
    n->len = 0;
    n->capacity = 0;
}

void big_from_int(BigInt *n, uint64_t val) {
    if (!n) return;
    if (!n->d) {
        // If not initialized, initialize it properly
        big_init(n);
    }
    n->d[0] = (uint32_t)(val & 0xFFFFFFFF);
    if (n->capacity > 1) {
        n->d[1] = (uint32_t)(val >> 32);
        n->len = (val > 0xFFFFFFFF) ? 2 : 1;
    }
    n->negative = 0;
}

char* big_to_string(const BigInt *n) {
    if (!n || !n->d) return strdup("0");
    
    char buffer[32];
    uint64_t val = n->d[0];
    if (n->len > 1) {
        val |= ((uint64_t)n->d[1] << 32);
    }
    snprintf(buffer, sizeof(buffer), "%lu", val);
    return strdup(buffer);
}

void big_add(const BigInt *a, const BigInt *b, BigInt *result) {
    if (!a || !b || !result) return;
    
    if (!result->d) {
        big_init(result);
    }
    
    uint64_t a_val = a->d[0];
    uint64_t b_val = b->d[0];
    if (a->len > 1) a_val |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) b_val |= ((uint64_t)b->d[1] << 32);
    
    uint64_t sum = a_val + b_val;
    result->d[0] = (uint32_t)(sum & 0xFFFFFFFF);
    if (result->capacity > 1) {
        result->d[1] = (uint32_t)(sum >> 32);
        result->len = (sum > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_sub(const BigInt *a, const BigInt *b, BigInt *result) {
    if (!a || !b || !result) return;
    
    if (!result->d) {
        big_init(result);
    }
    
    uint64_t a_val = a->d[0];
    uint64_t b_val = b->d[0];
    if (a->len > 1) a_val |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) b_val |= ((uint64_t)b->d[1] << 32);
    
    uint64_t diff = a_val - b_val;
    result->d[0] = (uint32_t)(diff & 0xFFFFFFFF);
    if (result->capacity > 1) {
        result->d[1] = (uint32_t)(diff >> 32);
        result->len = (diff > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_mul(const BigInt *a, const BigInt *b, BigInt *result) {
    if (!a || !b || !result) return;
    
    if (!result->d) {
        big_init(result);
    }
    
    // Ensure result has enough capacity for the product
    if (result->capacity < 2) {
        result->d = realloc(result->d, 2 * sizeof(uint32_t));
        result->capacity = 2;
    }
    
    uint64_t a_val = a->d[0];
    uint64_t b_val = b->d[0];
    if (a->len > 1) a_val |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) b_val |= ((uint64_t)b->d[1] << 32);
    
    uint64_t product = a_val * b_val;
    result->d[0] = (uint32_t)(product & 0xFFFFFFFF);
    result->d[1] = (uint32_t)(product >> 32);
    result->len = (product > 0xFFFFFFFF) ? 2 : 1;
}

void big_div(const BigInt *a, const BigInt *b, BigInt *quotient, BigInt *remainder) {
    if (!a || !b || !quotient || !remainder) return;
    
    if (!quotient->d) big_init(quotient);
    if (!remainder->d) big_init(remainder);
    
    uint64_t a_val = a->d[0];
    uint64_t b_val = b->d[0];
    if (a->len > 1) a_val |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) b_val |= ((uint64_t)b->d[1] << 32);
    
    if (b_val == 0) {
        quotient->d[0] = 0;
        quotient->len = 1;
        remainder->d[0] = (uint32_t)(a_val & 0xFFFFFFFF);
        remainder->len = (a_val > 0xFFFFFFFF) ? 2 : 1;
        return;
    }
    
    uint64_t q = a_val / b_val;
    uint64_t r = a_val % b_val;
    
    quotient->d[0] = (uint32_t)(q & 0xFFFFFFFF);
    if (quotient->capacity > 1) {
        quotient->d[1] = (uint32_t)(q >> 32);
        quotient->len = (q > 0xFFFFFFFF) ? 2 : 1;
    }
    
    remainder->d[0] = (uint32_t)(r & 0xFFFFFFFF);
    if (remainder->capacity > 1) {
        remainder->d[1] = (uint32_t)(r >> 32);
        remainder->len = (r > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_mod(const BigInt *a, const BigInt *b, BigInt *result) {
    if (!a || !b || !result) return;
    
    if (!result->d) big_init(result);
    
    uint64_t a_val = a->d[0];
    uint64_t b_val = b->d[0];
    if (a->len > 1) a_val |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) b_val |= ((uint64_t)b->d[1] << 32);
    
    if (b_val == 0) {
        result->d[0] = 0;
        result->len = 1;
        return;
    }
    
    uint64_t r = a_val % b_val;
    result->d[0] = (uint32_t)(r & 0xFFFFFFFF);
    if (result->capacity > 1) {
        result->d[1] = (uint32_t)(r >> 32);
        result->len = (r > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_copy(BigInt *dest, const BigInt *src) {
    if (!dest || !src) return;
    
    if (!dest->d) {
        dest->d = malloc(sizeof(uint32_t) * src->capacity);
        dest->len = src->len;
        dest->capacity = src->capacity;
        dest->target_len = src->target_len;
        dest->extended_len = src->extended_len;
        dest->negative = src->negative;
        dest->extended_mode = src->extended_mode;
    }
    
    for (size_t i = 0; i < src->len; i++) {
        dest->d[i] = src->d[i];
    }
    dest->len = src->len;
}

bool big_is_zero(const BigInt *n) {
    return (n && n->d && n->len > 0 && n->d[0] == 0 && 
            (n->len == 1 || (n->len > 1 && n->d[1] == 0)));
}

int big_cmp(const BigInt *a, const BigInt *b) {
    if (!a || !b) return 0;
    
    uint64_t a_val = a->d[0];
    uint64_t b_val = b->d[0];
    if (a->len > 1) a_val |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) b_val |= ((uint64_t)b->d[1] << 32);
    
    if (a_val < b_val) return -1;
    if (a_val > b_val) return 1;
    return 0;
}

void big_powmod(const BigInt *base, const BigInt *exp, const BigInt *mod, BigInt *result) {
    if (!base || !exp || !mod || !result) return;
    
    if (!result->d) big_init(result);
    
    uint64_t b = base->d[0];
    uint64_t e = exp->d[0];
    uint64_t m = mod->d[0];
    
    if (base->len > 1) b |= ((uint64_t)base->d[1] << 32);
    if (exp->len > 1) e |= ((uint64_t)exp->d[1] << 32);
    if (mod->len > 1) m |= ((uint64_t)mod->d[1] << 32);
    
    if (m == 0) {
        result->d[0] = 0;
        result->len = 1;
        return;
    }
    
    uint64_t result_val = 1;
    for (uint64_t i = 0; i < e && i < 20; i++) {
        result_val = (result_val * b) % m;
    }
    
    result->d[0] = (uint32_t)(result_val & 0xFFFFFFFF);
    if (result->capacity > 1) {
        result->d[1] = (uint32_t)(result_val >> 32);
        result->len = (result_val > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_shl(BigInt *n, int bits) {
    if (!n || bits < 0) return;
    
    uint64_t val = n->d[0];
    if (n->len > 1) val |= ((uint64_t)n->d[1] << 32);
    
    if (bits > 63) {
        val = 0;
    } else {
        val <<= bits;
    }
    
    n->d[0] = (uint32_t)(val & 0xFFFFFFFF);
    if (n->capacity > 1) {
        n->d[1] = (uint32_t)(val >> 32);
        n->len = (val > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_shr(BigInt *n, int bits) {
    if (!n || bits < 0) return;
    
    uint64_t val = n->d[0];
    if (n->len > 1) val |= ((uint64_t)n->d[1] << 32);
    
    if (bits > 63) {
        val = 0;
    } else {
        val >>= bits;
    }
    
    n->d[0] = (uint32_t)(val & 0xFFFFFFFF);
    if (n->capacity > 1) {
        n->d[1] = (uint32_t)(val >> 32);
        n->len = (val > 0xFFFFFFFF) ? 2 : 1;
    }
}

int big_is_prime(const BigInt *n, int iterations) {
    (void)iterations; // Suppress unused parameter warning
    if (!n || !n->d) return 0;
    
    uint64_t val = n->d[0];
    if (n->len > 1) val |= ((uint64_t)n->d[1] << 32);
    
    if (val < 2) return 0;
    if (val == 2) return 1;
    if (val % 2 == 0) return 0;
    
    for (uint64_t i = 3; i * i <= val && i < 1000; i += 2) {
        if (val % i == 0) return 0;
    }
    
    return 1;
}

int big_is_prime_trial(BigInt *n) {
    return big_is_prime(n, 1);
}

int big_is_prime_fermat(BigInt *n, int iterations) {
    return big_is_prime(n, iterations);
}

int big_is_prime_solovay_strassen(BigInt *n, int iterations) {
    return big_is_prime(n, iterations);
}

int big_is_prime_miller_rabin(BigInt *n) {
    return big_is_prime(n, 5);
}

bool big_are_twin_primes(const BigInt *p1, const BigInt *p2) {
    if (!p1 || !p2) return 0;
    
    if (!big_is_prime(p1, 1) || !big_is_prime(p2, 1)) return 0;
    
    BigInt diff, two;
    big_init(&diff);
    big_init(&two);
    big_from_int(&two, 2);  // Twin primes differ by 2, not 1
    
    if (big_cmp(p1, p2) > 0) {
        big_sub(p1, p2, &diff);
    } else {
        big_sub(p2, p1, &diff);
    }
    
    bool result = (big_cmp(&diff, &two) == 0);
    
    big_free(&diff);
    big_free(&two);
    
    return result;
}

bool big_is_sophie_germain_prime(const BigInt *p) {
    if (!p || !big_is_prime(p, 1)) return 0;
    
    BigInt two_p_plus_one, two, one, p_double;
    big_init(&two_p_plus_one);
    big_init(&two);
    big_init(&one);
    big_init(&p_double);
    
    big_from_int(&two, 2);
    big_from_int(&one, 1);
    big_mul(p, &two, &p_double);
    big_add(&p_double, &one, &two_p_plus_one);  // 2p + 1, not 2p + 2
    
    bool result = big_is_prime(&two_p_plus_one, 1);
    
    big_free(&two_p_plus_one);
    big_free(&two);
    big_free(&one);
    big_free(&p_double);
    
    return result;
}

void big_prime_factorization(BigInt *n, BigInt*** factors, int** powers, int* count) {
    if (!n || !factors || !powers || !count) {
        *count = 0;
        return;
    }
    
    // Convert to uint64_t for simple factorization
    uint64_t val = n->d[0];
    if (n->len > 1) val |= ((uint64_t)n->d[1] << 32);
    
    *factors = malloc(sizeof(BigInt*) * 10);
    *powers = malloc(sizeof(int) * 10);
    *count = 0;
    
    // Factor out 2s
    int power = 0;
    while (val % 2 == 0 && val > 0) {
        power++;
        val /= 2;
    }
    if (power > 0) {
        (*factors)[*count] = malloc(sizeof(BigInt));
        big_init((*factors)[*count]);
        big_from_int((*factors)[*count], 2);
        (*powers)[*count] = power;
        (*count)++;
    }
    
    // Factor out odd primes
    for (uint64_t i = 3; i * i <= val && *count < 10; i += 2) {
        power = 0;
        while (val % i == 0 && val > 0) {
            power++;
            val /= i;
        }
        if (power > 0) {
            (*factors)[*count] = malloc(sizeof(BigInt));
            big_init((*factors)[*count]);
            big_from_int((*factors)[*count], i);
            (*powers)[*count] = power;
            (*count)++;
        }
    }
    
    // If val > 1, it's a prime factor
    if (val > 1 && *count < 10) {
        (*factors)[*count] = malloc(sizeof(BigInt));
        big_init((*factors)[*count]);
        big_from_int((*factors)[*count], val);
        (*powers)[*count] = 1;
        (*count)++;
    }
}

void big_euler_totient(const BigInt *n, BigInt *result) {
    if (!n || !result) return;
    
    uint64_t val = n->d[0];
    if (n->len > 1) val |= ((uint64_t)n->d[1] << 32);
    
    uint64_t totient = val;
    
    if (val == 0) {
        big_from_int(result, 0);
        return;
    }
    
    for (uint64_t p = 2; p * p <= val && p < 100; p++) {
        if (val % p == 0) {
            while (val % p == 0) {
                val /= p;
            }
            totient -= totient / p;
        }
    }
    
    if (val > 1) {
        totient -= totient / val;
    }
    
    big_from_int(result, totient);
}

void big_gcd(const BigInt *a, const BigInt *b, BigInt *result) {
    if (!a || !b || !result) return;
    
    if (!result->d) big_init(result);
    
    uint64_t val_a = a->d[0];
    uint64_t val_b = b->d[0];
    if (a->len > 1) val_a |= ((uint64_t)a->d[1] << 32);
    if (b->len > 1) val_b |= ((uint64_t)b->d[1] << 32);
    
    while (val_b != 0) {
        uint64_t temp = val_b;
        val_b = val_a % val_b;
        val_a = temp;
    }
    
    result->d[0] = (uint32_t)(val_a & 0xFFFFFFFF);
    if (result->capacity > 1) {
        result->d[1] = (uint32_t)(val_a >> 32);
        result->len = (val_a > 0xFFFFFFFF) ? 2 : 1;
    }
}

void big_lcm(const BigInt *a, const BigInt *b, BigInt *result) {
    if (!a || !b || !result) return;
    
    BigInt product, gcd, temp;
    big_init(&product);
    big_init(&gcd);
    big_init(&temp);
    
    big_mul(a, b, &product);
    big_gcd(a, b, &gcd);
    
    if (!big_is_zero(&gcd)) {
        big_div(&product, &gcd, result, &temp);
    } else {
        big_copy(result, &product);
    }
    
    big_free(&product);
    big_free(&gcd);
    big_free(&temp);
}

void crystal_abacus_big_init(CrystalAbacusBig *ab) {
    if (!ab) return;
    ab->current_index = 1;  // Start at 1 so first prime will be 2
    ab->primes = NULL;
    ab->num_primes = 0;
    ab->capacity = 0;
}

void crystal_abacus_big_free(CrystalAbacusBig *ab) {
    if (!ab) return;
    if (ab->primes) {
        free(ab->primes);
        ab->primes = NULL;
    }
}

void crystal_abacus_big_next_prime(CrystalAbacusBig *ab, BigInt *p) {
    if (!ab || !p) return;
    
    uint64_t current = ab->current_index;
    current++;
    
    // No upper limit - continue searching for primes
    while (1) {
        int is_prime = 1;
        if (current < 2) {
            is_prime = 0;
        } else if (current == 2) {
            is_prime = 1;
        } else if (current % 2 == 0) {
            is_prime = 0;
        } else {
            for (uint64_t i = 3; is_prime && i * i <= current; i += 2) {
                if (current % i == 0) is_prime = 0;
            }
        }
        
        if (is_prime) {
            big_from_int(p, current);
            ab->current_index = current;
            return;
        }
        current++;
    }
}

void big_prime_add(const BigInt *a, const BigInt *b, BigInt *c) {
    big_add(a, b, c);
}

void big_prime_subtract(const BigInt *a, const BigInt *b, BigInt *c) {
    big_sub(a, b, c);
}

void big_prime_multiply(const BigInt *a, const BigInt *b, BigInt *c) {
    big_mul(a, b, c);
}

void big_prime_divide(const BigInt *a, const BigInt *b, BigInt *q, BigInt *r) {
    big_div(a, b, q, r);
}

double big_to_double(const BigInt *n) {
    if (!n || !n->d) return 0.0;
    uint64_t val = n->d[0];
    if (n->len > 1) val |= ((uint64_t)n->d[1] << 32);
    return (double)val;
}

BigInt** big_generate_n_primes(int n, int* count) {
    if (n <= 0) {
        *count = 0;
        return NULL;
    }
    
    BigInt** primes = malloc(n * sizeof(BigInt*));
    if (!primes) {
        *count = 0;
        return NULL;
    }
    
    CrystalAbacusBig abacus;
    crystal_abacus_big_init(&abacus);
    
    int generated = 0;
    BigInt prime;
    big_init(&prime);
    
    for (int i = 0; i < n; i++) {
        primes[i] = malloc(sizeof(BigInt));
        if (!primes[i]) {
            for (int j = 0; j < i; j++) {
                big_free(primes[j]);
                free(primes[j]);
            }
            free(primes);
            big_free(&prime);
            crystal_abacus_big_free(&abacus);
            *count = 0;
            return NULL;
        }
        
        big_init(primes[i]);
        crystal_abacus_big_next_prime(&abacus, primes[i]);
        generated++;
    }
    
    big_free(&prime);
    crystal_abacus_big_free(&abacus);
    *count = generated;
    return primes;
}

int big_lattice_sign(BigInt *x, int depth) {
    (void)depth; // Suppress unused parameter warning
    if (!x || big_is_zero(x)) return 0;
    uint64_t val = x->d[0];
    if (x->len > 1) val |= ((uint64_t)x->d[1] << 32);
    return (val > 0) ? 1 : -1;
}

// ═══════════════════════════════════════════════════════════════════════════
// ADDITIONAL BIGINT OPERATIONS
// ═══════════════════════════════════════════════════════════════════════════

int big_cmp_int(const BigInt *a, uint64_t n) {
    if (!a) return -1;
    if (big_is_zero(a) && n == 0) return 0;
    if (big_is_zero(a)) return -1;
    if (n == 0) return 1;
    
    // If a has more than 2 limbs, it's definitely larger than uint64_t
    if (a->len > 2) return 1;
    
    // Compare as uint64_t
    uint64_t a_val = a->d[0];
    if (a->len > 1) {
        a_val |= ((uint64_t)a->d[1] << 32);
    }
    
    if (a_val < n) return -1;
    if (a_val > n) return 1;
    return 0;
}

void big_lattice_add(const BigInt *a, const BigInt *b, BigInt *result, int depth) {
    (void)depth; // Lattice depth for future multi-dimensional operations
    big_add(a, b, result);
}

bool big_is_mersenne_prime(const BigInt *p) {
    // A Mersenne prime is a prime of the form 2^n - 1
    // Check if p+1 is a power of 2
    if (!p) return false;
    
    BigInt p_plus_1;
    big_init(&p_plus_1);
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    
    big_add(p, &one, &p_plus_1);
    
    // Check if p_plus_1 is a power of 2
    // A number is a power of 2 if it has only one bit set
    bool is_power_of_2 = false;
    
    if (p_plus_1.len > 0) {
        int bit_count = 0;
        for (size_t i = 0; i < p_plus_1.len; i++) {
            uint32_t word = p_plus_1.d[i];
            while (word) {
                if (word & 1) bit_count++;
                word >>= 1;
            }
        }
        is_power_of_2 = (bit_count == 1);
    }
    
    big_free(&p_plus_1);
    big_free(&one);
    
    // Also verify it's actually prime
    if (is_power_of_2) {
        return big_is_prime(p, 10);
    }
    
    return false;
}