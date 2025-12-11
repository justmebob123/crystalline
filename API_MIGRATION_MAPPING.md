# API Migration Mapping: OLD crystalline → NEW math

## Overview
This document maps the OLD crystalline library API to the NEW math library API to facilitate migration.

---

## 1. BASIC ARITHMETIC

| OLD API (crystalline) | NEW API (math) | Notes |
|----------------------|----------------|-------|
| `prime_add(a, b)` | `math_add(a, b)` | Direct replacement |
| `prime_subtract(a, b)` | `math_sub(a, b)` | Direct replacement |
| `prime_multiply(a, b)` | `math_mul(a, b)` | Direct replacement |
| `prime_divide(a, b)` | `math_div(a, b)` | Direct replacement |
| `prime_fabs(x)` | `math_abs(x)` | Direct replacement |
| `prime_fmin(a, b)` | `math_min(a, b)` | Direct replacement |
| `prime_fmax(a, b)` | `math_max(a, b)` | Direct replacement |
| `prime_fmod(x, y)` | `math_fmod(x, y)` | Direct replacement |
| `prime_remainder(x, y)` | `math_remainder(x, y)` | Direct replacement |

---

## 2. ROUNDING FUNCTIONS

| OLD API (crystalline) | NEW API (math) | Notes |
|----------------------|----------------|-------|
| `prime_floor(x)` | `math_floor(x)` | Direct replacement |
| `prime_ceil(x)` | `math_ceil(x)` | Direct replacement |
| `prime_round(x)` | `math_round(x)` | Direct replacement |
| `prime_trunc(x)` | `math_trunc(x)` | Direct replacement |

---

## 3. TRANSCENDENTAL FUNCTIONS

| OLD API (crystalline) | NEW API (math) | Notes |
|----------------------|----------------|-------|
| `prime_sqrt(x)` | `math_sqrt(x)` | Direct replacement |
| `prime_pow(x, y)` | `math_pow(x, y)` | Direct replacement |
| `prime_exp(x)` | `math_exp(x)` | Direct replacement |
| `prime_log(x)` | `math_log(x)` | Direct replacement |
| `prime_sin(x)` | `math_sin(x)` | Direct replacement |
| `prime_cos(x)` | `math_cos(x)` | Direct replacement |
| `prime_tan(x)` | `math_tan(x)` | Direct replacement |
| `prime_asin(x)` | `math_asin(x)` | Direct replacement |
| `prime_acos(x)` | `math_acos(x)` | Direct replacement |
| `prime_atan(x)` | `math_atan(x)` | Direct replacement |
| `prime_sinh(x)` | `math_sinh(x)` | Direct replacement |
| `prime_cosh(x)` | `math_cosh(x)` | Direct replacement |
| `prime_tanh(x)` | `math_tanh(x)` | Direct replacement |

---

## 4. SPECIAL CHECKS

| OLD API (crystalline) | NEW API (math) | Notes |
|----------------------|----------------|-------|
| `prime_isnan(x)` | `math_is_nan(x)` | Returns bool instead of int |
| `prime_isinf(x)` | `math_is_inf(x)` | Returns bool instead of int |
| N/A | `math_is_finite(x)` | New function |

---

## 5. FLOAT VARIANTS

| OLD API (crystalline) | NEW API (math) | Notes |
|----------------------|----------------|-------|
| `prime_fabsf(x)` | `math_abs((double)x)` | Cast to double |
| `prime_sqrtf(x)` | `math_sqrt((double)x)` | Cast to double |
| `prime_sinf(x)` | `math_sin((double)x)` | Cast to double |
| `prime_cosf(x)` | `math_cos((double)x)` | Cast to double |
| `prime_tanf(x)` | `math_tan((double)x)` | Cast to double |
| `prime_expf(x)` | `math_exp((double)x)` | Cast to double |
| `prime_logf(x)` | `math_log((double)x)` | Cast to double |
| `prime_powf(x, y)` | `math_pow((double)x, (double)y)` | Cast to double |
| `prime_atanf(x)` | `math_atan((double)x)` | Cast to double |
| `prime_tanhf(x)` | `math_tanh((double)x)` | Cast to double |
| `prime_fmodf(x, y)` | `math_fmod((double)x, (double)y)` | Cast to double |
| `prime_fminf(x, y)` | `math_min((double)x, (double)y)` | Cast to double |
| `prime_fmaxf(x, y)` | `math_max((double)x, (double)y)` | Cast to double |
| `prime_isnanf(x)` | `math_is_nan((double)x)` | Cast to double |
| `prime_isinff(x)` | `math_is_inf((double)x)` | Cast to double |

---

## 6. NEW FUNCTIONS (Not in OLD API)

| NEW API (math) | Description |
|----------------|-------------|
| `math_cbrt(x)` | Cube root |
| `math_powi(x, n)` | Integer power (optimized) |
| `math_expm1(x)` | exp(x) - 1 (accurate for small x) |
| `math_log1p(x)` | log(1 + x) (accurate for small x) |
| `math_log10(x)` | Base-10 logarithm |
| `math_log2(x)` | Base-2 logarithm |
| `math_sincos(x, &sin, &cos)` | Compute both sin and cos |
| `math_atan2(y, x)` | Two-argument arctangent |
| `math_asinh(x)` | Inverse hyperbolic sine |
| `math_acosh(x)` | Inverse hyperbolic cosine |
| `math_atanh(x)` | Inverse hyperbolic tangent |
| `math_clamp(x, min, max)` | Clamp value to range |
| `math_sign(x)` | Sign of x (-1, 0, 1) |
| `math_approx_equal(a, b, eps)` | Approximate equality |

---

## 7. INITIALIZATION

| OLD API (crystalline) | NEW API (math) | Notes |
|----------------------|----------------|-------|
| `prime_math_init()` | N/A | Not needed in new API |
| `prime_math_init_depth(d)` | N/A | Not needed in new API |
| `prime_math_cleanup()` | N/A | Not needed in new API |
| `prime_math_is_initialized()` | N/A | Not needed in new API |
| `prime_math_get_depth()` | N/A | Not needed in new API |
| `prime_math_extend_cache(n)` | N/A | Not needed in new API |

**Note**: The new math library does not require explicit initialization. Functions work immediately.

---

## 8. MIGRATION STRATEGY

### Step 1: Include Header
```c
// OLD
#include "prime_math.h"
#include "prime_float_math.h"

// NEW
#include "math.h"  // or #include "math/arithmetic.h" + "math/transcendental.h"
```

### Step 2: Replace Function Calls
Use find-and-replace with the mappings above:
```bash
# Example: Replace prime_sqrt with math_sqrt
sed -i 's/prime_sqrt(/math_sqrt(/g' file.c
```

### Step 3: Remove Initialization
```c
// OLD
prime_math_init();
// ... code ...
prime_math_cleanup();

// NEW
// No initialization needed!
// ... code ...
```

### Step 4: Handle Float Variants
```c
// OLD
float x = 2.0f;
float y = prime_sqrtf(x);

// NEW
float x = 2.0f;
float y = (float)math_sqrt((double)x);
```

---

## 9. COMPATIBILITY LAYER (Optional)

For gradual migration, create a compatibility header:

```c
// prime_math_compat.h
#ifndef PRIME_MATH_COMPAT_H
#define PRIME_MATH_COMPAT_H

#include "math.h"

// Map old names to new names
#define prime_add(a, b) math_add(a, b)
#define prime_subtract(a, b) math_sub(a, b)
#define prime_multiply(a, b) math_mul(a, b)
#define prime_divide(a, b) math_div(a, b)
#define prime_fabs(x) math_abs(x)
#define prime_sqrt(x) math_sqrt(x)
#define prime_pow(x, y) math_pow(x, y)
#define prime_exp(x) math_exp(x)
#define prime_log(x) math_log(x)
#define prime_sin(x) math_sin(x)
#define prime_cos(x) math_cos(x)
#define prime_tan(x) math_tan(x)
#define prime_floor(x) math_floor(x)
#define prime_ceil(x) math_ceil(x)
#define prime_round(x) math_round(x)
// ... etc

// No-op initialization
#define prime_math_init() ((void)0)
#define prime_math_cleanup() ((void)0)

#endif
```

---

## 10. TESTING STRATEGY

1. **Unit Tests**: Verify each function produces identical results
2. **Integration Tests**: Test full algorithms with both APIs
3. **Performance Tests**: Ensure no regression
4. **Regression Tests**: Run existing test suite

---

## 11. MIGRATION CHECKLIST

- [ ] Create API mapping document (this file)
- [ ] Fix duplicate definitions in new math library
- [ ] Build new math library (static + shared)
- [ ] Create compatibility layer (optional)
- [ ] Migrate algorithms library (82 files)
- [ ] Update Makefiles and include paths
- [ ] Run full test suite
- [ ] Performance benchmarks
- [ ] Update documentation
- [ ] Remove old crystalline library

---

## NOTES

- The new API is cleaner (math_* instead of prime_*)
- No initialization required (simpler usage)
- Better naming conventions (math_sub vs prime_subtract)
- Additional functions not in old API
- Float variants handled by casting (simpler API)

