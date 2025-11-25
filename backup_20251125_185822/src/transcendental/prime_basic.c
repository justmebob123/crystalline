#define _GNU_SOURCE
#include "../include/prime_math_custom.h"
#include "../include/prime_math.h"

/*
 * CRITICAL: This file maintains mathematical independence.
 * NO math.h dependency - uses custom implementations.
 */

// Custom absolute value without math.h
static inline double custom_fabs(double x) {
    return (x < 0.0) ? -x : x;
}

// Custom NaN check without math.h
// IEEE 754: NaN has exponent = 0x7FF and non-zero mantissa
static inline int custom_isnan(double x) {
    union { double d; uint64_t u; } val;
    val.d = x;
    return ((val.u & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL) &&
           ((val.u & 0x000FFFFFFFFFFFFFULL) != 0);
}

// Custom infinity check without math.h
// IEEE 754: infinity has exponent = 0x7FF and zero mantissa
static inline int custom_isinf(double x) {
    union { double d; uint64_t u; } val;
    val.d = x;
    return ((val.u & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL) &&
           ((val.u & 0x000FFFFFFFFFFFFFULL) == 0);
}

// Basic utility functions
double prime_fabs(double x) {
    return custom_fabs(x);
}

// Float absolute value function - moved to prime_float_math.c

int prime_isnan(double x) {
    return custom_isnan(x);
}

int prime_isinf(double x) {
    return custom_isinf(x);
}

// Fixed-point conversion functions
// FIXED_POINT_SCALE defined in prime_types.h

uint64_t double_to_fixed(double x) {
    return (uint64_t)(custom_fabs(x) * FIXED_POINT_SCALE);
}

double fixed_to_double(uint64_t x) {
    return (double)x / FIXED_POINT_SCALE;
}

double fixed_to_double_signed(int64_t x) {
    return (double)x / FIXED_POINT_SCALE;
}

// Basic prime arithmetic functions
double prime_add(double a, double b) {
    return a + b;
}

double prime_subtract(double a, double b) {
    return a - b;
}

double prime_multiply(double a, double b) {
    return a * b;
}

double prime_divide(double a, double b) {
    if (b == 0) return 0; // Handle division by zero
    return a / b;
}

double prime_abs(double x) {
    return prime_fabs(x);
}

// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_pow(double base, double exponent) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_pow(base, exponent);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_fmod(double a, double b) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     if (b == 0) return 0;
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_fmod(a, b);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: // Transcendental functions
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_sqrt(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     if (x < 0) return 0;
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_sqrt(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_exp(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_exp(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_log(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     if (x <= 0) return 0;
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_log(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_log10(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     if (x <= 0) return 0;
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_log10(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_sinh(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_sinh(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_cosh(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_cosh(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_tanh(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_tanh(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: // Trigonometric functions
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_sin(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_sin(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_cos(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_cos(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_tan(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_tan(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_asin(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     if (x < -1 || x > 1) return 0;
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_asin(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_acos(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     if (x < -1 || x > 1) return 0;
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_acos(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: 
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: double prime_atan(double x) {
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c:     return prime_atan(x);
// REMOVED: // COMMENTED OUT - Implemented in prime_math_custom.c: }

// Special functions (minimal implementations)
double shannon_entropy(const double* data, int n) {
    if (!data || n <= 0) return 0.0;
    
    double entropy = 0.0;
    for (int i = 0; i < n; i++) {
        if (data[i] > 0) {
            entropy -= data[i] * prime_log2(data[i]);
        }
    }
    return entropy;
}

double relative_entropy(const double* p, const double* q, int n) {
    if (!p || !q || n <= 0) return 0.0;
    
    double kl_div = 0.0;
    for (int i = 0; i < n; i++) {
        if (p[i] > 0 && q[i] > 0) {
            kl_div += p[i] * prime_log2(p[i] / q[i]);
        }
    }
    return kl_div;
}

double wave_simulation(double amplitude, double frequency, int harmonics, double phase, int overtones, double damping) {
    (void)overtones; // Suppress unused parameter warning
    double result = 0.0;
    for (int h = 1; h <= harmonics; h++) {
        double harmonic_freq = frequency * h;
        double harmonic_amp = amplitude / h;
        double damping_factor = prime_exp(-damping * h);
        result += harmonic_amp * damping_factor * prime_sin(2.0 * PRIME_PI * harmonic_freq + phase);
    }
    return result;
}

double tetration_log_approx(double x, int height) {
    if (x <= 0 || height <= 0) return 0.0;
    double result = prime_log(x);
    for (int i = 1; i < height; i++) {
        if (result > 0) {
            result = prime_log(result);
            if (prime_isnan(result) || prime_isinf(result)) break;
        } else {
            break;
        }
    }
    return (prime_isnan(result) || prime_isinf(result)) ? 0.0 : result;
}

double super_logarithm(double x) {
    if (x <= 0) return 0.0;
    if (x > 1.0) {
        return prime_log(prime_log(x));
    } else {
        return prime_log(x);
    }
}

double fibonacci_prime(int n) {
    if (n <= 0) return 0.0;
    double a = 0.0, b = 1.0;
    double result = 0.0;
    int count = 0;
    
    while (count < n) {
        double next = a + b;
        a = b;
        b = next;
        
        if (b > 1) {
            int is_prime = 1;
            int int_b = (int)prime_round(b);
            
            if (int_b > 2 && int_b % 2 == 0) is_prime = 0;
            
            for (int d = 3; d * d <= int_b && is_prime; d += 2) {
                if (int_b % d == 0) is_prime = 0;
            }
            
            if (is_prime) {
                result = b;
                count++;
            }
        }
        
        if (b > 1000000) break;
    }
    return result;
}

double golden_stabilizer(double x, int iterations, int depth) {
    if (iterations <= 0) return x;
    
    double result = x;
    for (int i = 0; i < iterations; i++) {
        result = result / PHI + PHI * (depth % 3 == 0 ? 0.1 : 0.05);
        if (i > 0) {
            int is_prime_iteration = 1;
            
            for (int d = 2; d * d <= i + 1; d++) {
                if ((i + 1) % d == 0) {
                    is_prime_iteration = 0;
                    break;
                }
            }
            
            if (is_prime_iteration) {
                result *= (1.0 + 0.01 / (i + 1));
            }
        }
    }
    return result;
}

double resonance_frequency(double base_freq, double modifier) {
    if (base_freq <= 0) return 0.0;
    double resonance = base_freq;
    int primes[] = {2, 3, 5, 7, 11, 13};
    int num_primes = sizeof(primes) / sizeof(primes[0]);
    
    for (int i = 0; i < num_primes; i++) {
        resonance += base_freq * modifier / (primes[i] * primes[i]);
    }
    return resonance;
}

double harmonic_mean(const double* values, int n) {
    if (!values || n <= 0) return 0.0;
    
    double sum_reciprocal = 0.0;
    for (int i = 0; i < n; i++) {
        if (values[i] != 0) {
            sum_reciprocal += 1.0 / values[i];
        }
    }
    
    return (sum_reciprocal > 0) ? (n / sum_reciprocal) : 0.0;
}

double check_self_similarity(const double* pattern, int size, int scale) {
    if (!pattern || size <= 0 || scale <= 0 || scale >= size) {
        return 0.0;
    }
    
    int num_segments = size / scale;
    if (num_segments < 2) return 0.0;
    
    double similarity = 0.0;
    int comparisons = 0;
    
    for (int i = 1; i < num_segments; i++) {
        double segment_similarity = 0.0;
        
        for (int j = 0; j < scale; j++) {
            double diff = prime_fabs(pattern[j] - pattern[i * scale + j]);
            double max_val = prime_fmax(prime_fabs(pattern[j]), 1.0);
            segment_similarity += 1.0 - (diff / max_val);
        }
        
        similarity += segment_similarity / scale;
        comparisons++;
    }
    
    return (comparisons > 0) ? (similarity / comparisons) : 0.0;
}

double fractal_dimension(const double* data, int size) {
    if (!data || size < 4) return 0.0;
    
    double total_variation = 0.0;
    for (int i = 1; i < size; i++) {
        total_variation += prime_fabs(data[i] - data[i-1]);
    }
    
    double range = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] > range) range = data[i];
    }
    
    if (range <= 0) return 0.0;
    
    double roughness = total_variation / (size * range);
    return 1.0 + roughness;
}