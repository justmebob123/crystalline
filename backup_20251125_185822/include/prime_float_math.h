/*
 * Prime Float Math - Header
 * 
 * Float-precision mathematical functions WITHOUT external dependencies.
 * Provides replacements for standard math.h functions.
 * 
 * CRITICAL: Maintains mathematical independence!
 */

#ifndef PRIME_FLOAT_MATH_H
#define PRIME_FLOAT_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mathematical Constants
 */
#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

/*
 * Basic Mathematical Functions
 */

// Square root
float prime_sqrtf(float x);

// Exponential (e^x)
float prime_expf(float x);

// Natural logarithm
float prime_logf(float x);

// Power (x^y)
float prime_powf(float x, float y);

/*
 * Trigonometric Functions
 */

// Sine
float prime_sinf(float x);

// Cosine
float prime_cosf(float x);

// Tangent
float prime_tanf(float x);

/*
 * Hyperbolic Functions
 */

// Hyperbolic tangent
float prime_tanhf(float x);

/*
 * Utility Functions
 */

// Absolute value
float prime_fabsf(float x);

// Double precision math functions (for compatibility)
double prime_exp(double x);
double prime_sqrt(double x);
double prime_log(double x);
double prime_cos(double x);
double prime_sin(double x);
double prime_tan(double x);
double prime_tanh(double x);
double prime_pow(double x, double y);
double prime_atan(double x);
double prime_atan2(double y, double x);

// Float precision math functions
float prime_expf(float x);
float prime_sqrtf(float x);
float prime_logf(float x);
float prime_cosf(float x);
float prime_sinf(float x);
float prime_tanf(float x);
float prime_tanhf(float x);
float prime_powf(float x, float y);
float prime_atanf(float x);
float prime_atan2f(float y, float x);

// Additional math functions
double prime_floor(double x);
double prime_ceil(double x);
double prime_round(double x);
double prime_fmod(double x, double y);
double prime_acos(double x);
double prime_log2(double x);

// Maximum of two values
float prime_fmaxf(float x, float y);
double prime_fmax(double x, double y);

// Minimum of two values
float prime_fminf(float x, float y);
double prime_fmin(double x, double y);

// Check if NaN
int prime_isnanf(float x);

// Check if infinite
int prime_isinff(float x);

#ifdef __cplusplus
}
#endif

#endif /* PRIME_FLOAT_MATH_H */