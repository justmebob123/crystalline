# OLD to NEW Math Library API Mapping

## Overview

This document maps the OLD math library API (include/bigint_core.h, include/bigfixed_core.h) to the NEW math library API (math/include/math/).

**CRITICAL:** The NEW math library is called `libcrystallinemath` and is located in the `math/` directory.

---

## Library Structure

### OLD Library (DEPRECATED)
```
include/
├── bigint_core.h       - BigInt arbitrary precision integers
├── bigfixed_core.h     - BigFixed fixed-point numbers
├── prime_lowlevel.h    - Low-level prime functions
└── prime_math_custom.h - Custom prime math functions
```

### NEW Library (CURRENT)
```
math/include/math/
├── types.h             - Core types (MathError, ClockContext, etc.)
├── abacus.h            - Crystalline Abacus (replaces BigInt/BigFixed)
├── clock.h             - Clock lattice functions
├── prime.h             - Prime generation (O(1) deterministic)
├── rainbow.h           - Rainbow table for primes
├── ntt.h               - Number Theoretic Transform
├── transcendental.h    - Transcendental functions (NO math.h)
├── arithmetic.h        - Basic arithmetic operations
├── compact_vector.h    - Compact vector system (NEW!)
├── platonic_generator.h - Platonic solid generators
├── polytope.h          - Polytope structures
└── visualization.h     - Visualization helpers
```

---

## Type Mappings

### OLD Types → NEW Types

| OLD Type | NEW Type | Notes |
|----------|----------|-------|
| `BigInt` | `CrystallineAbacus` | Universal arbitrary precision |
| `BigFixed` | `CrystallineAbacus` | Same type, uses weight_exponent for fractions |
| `BigInt*` | `CrystallineAbacus*` | Pointer type |
| `BigFixed*` | `CrystallineAbacus*` | Pointer type |
| `BigInt**` | `CrystallineAbacus**` | Array of pointers |
| `BigFixed**` | `CrystallineAbacus**` | Array of pointers |

### NEW Core Types

```c
// Error handling
typedef enum {
    MATH_SUCCESS = 0,
    MATH_ERROR_NULL_POINTER,
    MATH_ERROR_INVALID_ARGUMENT,
    MATH_ERROR_OUT_OF_MEMORY,
    MATH_ERROR_DIVISION_BY_ZERO,
    MATH_ERROR_OVERFLOW,
    MATH_ERROR_UNDERFLOW,
    MATH_ERROR_NOT_IMPLEMENTED
} MathError;

// Clock context for prime generation
typedef struct {
    uint64_t *prime_cache;
    uint32_t cache_size;
    uint32_t cache_capacity;
} ClockContext;

// Crystalline Abacus (replaces BigInt and BigFixed)
typedef struct {
    AbacusBead* beads;       // Array of beads
    size_t num_beads;        // Number of beads
    size_t capacity;         // Allocated capacity
    uint32_t base;           // Number base (12, 60, 100, or any >= 2)
    bool negative;           // Sign
    int32_t min_exponent;    // For fractional precision
} CrystallineAbacus;
```

---

## Function Mappings

### Initialization & Cleanup

| OLD Function | NEW Function | Notes |
|--------------|--------------|-------|
| `bigint_create(value)` | `abacus_create_from_uint64(value, base, ctx)` | Requires base and context |
| `bigint_init(bi)` | `abacus_init(abacus, base, ctx)` | Requires base and context |
| `bigint_free(bi)` | `abacus_free(abacus)` | Same concept |
| `bigfixed_create(value, precision)` | `abacus_create_from_uint64(value, base, ctx)` | Use min_exponent for precision |
| `bigfixed_init(bf, precision)` | `abacus_init(abacus, base, ctx)` | Set min_exponent separately |
| `bigfixed_free(bf)` | `abacus_free(abacus)` | Same concept |

**Example Conversion:**
```c
// OLD
BigInt* a = bigint_create(157);
bigint_free(a);

// NEW
ClockContext ctx;
clock_init(&ctx);
CrystallineAbacus* a = abacus_create_from_uint64(157, 12, &ctx);
abacus_free(a);
clock_cleanup(&ctx);
```

### Arithmetic Operations

| OLD Function | NEW Function | Notes |
|--------------|--------------|-------|
| `bigint_add(result, a, b)` | `abacus_add(result, a, b)` | Returns MathError |
| `bigint_sub(result, a, b)` | `abacus_subtract(result, a, b)` | Returns MathError |
| `bigint_mul(result, a, b)` | `abacus_multiply(result, a, b)` | Returns MathError |
| `bigint_div(result, a, b)` | `abacus_divide(result, a, b)` | Returns MathError |
| `bigint_mod(result, a, b)` | `abacus_mod(result, a, b)` | Returns MathError |
| `bigfixed_add(result, a, b)` | `abacus_add(result, a, b)` | Same function! |
| `bigfixed_sub(result, a, b)` | `abacus_subtract(result, a, b)` | Same function! |
| `bigfixed_mul(result, a, b)` | `abacus_multiply(result, a, b)` | Same function! |
| `bigfixed_div(result, a, b)` | `abacus_divide(result, a, b)` | Same function! |

**Example Conversion:**
```c
// OLD
BigInt* a = bigint_create(5);
BigInt* b = bigint_create(7);
BigInt* result = bigint_create(0);
bigint_add(result, a, b);  // result = 12

// NEW
ClockContext ctx;
clock_init(&ctx);
CrystallineAbacus* a = abacus_create_from_uint64(5, 12, &ctx);
CrystallineAbacus* b = abacus_create_from_uint64(7, 12, &ctx);
CrystallineAbacus* result = abacus_create_from_uint64(0, 12, &ctx);
MathError err = abacus_add(result, a, b);  // result = 12
if (err != MATH_SUCCESS) {
    // Handle error
}
```

### Comparison Operations

| OLD Function | NEW Function | Notes |
|--------------|--------------|-------|
| `bigint_compare(a, b)` | `abacus_compare(a, b)` | Returns -1, 0, or 1 |
| `bigint_equals(a, b)` | `abacus_compare(a, b) == 0` | Use compare |
| `bigint_less_than(a, b)` | `abacus_compare(a, b) < 0` | Use compare |
| `bigint_greater_than(a, b)` | `abacus_compare(a, b) > 0` | Use compare |
| `bigfixed_compare(a, b)` | `abacus_compare(a, b)` | Same function! |

### Conversion Operations

| OLD Function | NEW Function | Notes |
|--------------|--------------|-------|
| `bigint_to_uint64(bi)` | `abacus_to_uint64(abacus)` | May lose precision |
| `bigint_to_double(bi)` | `abacus_to_double(abacus)` | May lose precision |
| `bigint_from_string(str)` | `abacus_from_string(str, base, ctx)` | Requires base |
| `bigint_to_string(bi)` | `abacus_to_string(abacus)` | Returns allocated string |
| `bigfixed_to_double(bf)` | `abacus_to_double(abacus)` | Same function! |
| `bigfixed_from_double(d, precision)` | `abacus_from_double(d, base, precision, ctx)` | Requires base |

### Modular Arithmetic

| OLD Function | NEW Function | Notes |
|--------------|--------------|-------|
| `bigint_mod_add(result, a, b, mod)` | `abacus_mod_add(result, a, b, mod)` | Returns MathError |
| `bigint_mod_sub(result, a, b, mod)` | `abacus_mod_subtract(result, a, b, mod)` | Returns MathError |
| `bigint_mod_mul(result, a, b, mod)` | `abacus_mod_multiply(result, a, b, mod)` | Returns MathError |
| `bigint_mod_exp(result, base, exp, mod)` | `abacus_mod_exp(result, base, exp, mod)` | Returns MathError |
| `bigint_mod_inverse(result, a, mod)` | `abacus_mod_inverse(result, a, mod)` | Returns MathError |

### Prime Functions

| OLD Function | NEW Function | Notes |
|--------------|--------------|-------|
| `is_prime(n)` | `prime_is_prime(n)` | O(1) deterministic! |
| `next_prime(n)` | `prime_next(n, ctx)` | Uses O(1) formula |
| `prime_at_index(idx)` | `prime_at_index(idx, ctx)` | O(1) generation |
| `generate_primes(n)` | `rainbow_populate(table, max, ctx)` | Uses rainbow table |

---

## Include Path Changes

### OLD Includes (REMOVE)
```c
#include "../../include/bigint_core.h"
#include "../../include/bigfixed_core.h"
#include "../../include/prime_lowlevel.h"
#include "bigint_core.h"
#include "bigfixed_core.h"
```

### NEW Includes (USE)
```c
#include "math/types.h"
#include "math/abacus.h"
#include "math/clock.h"
#include "math/prime.h"
#include "math/rainbow.h"
#include "math/ntt.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
```

---

## Compilation Changes

### OLD Compilation
```makefile
CFLAGS = -I./include
LDFLAGS = -L. -lcrystalline
```

### NEW Compilation
```makefile
CFLAGS = -I./include -I./math/include
LDFLAGS = -L. -L./math/lib -lcrystalline -lcrystallinemath
```

---

## Error Handling Changes

### OLD Error Handling
```c
// OLD: No error handling
bigint_add(result, a, b);
```

### NEW Error Handling
```c
// NEW: Explicit error handling
MathError err = abacus_add(result, a, b);
if (err != MATH_SUCCESS) {
    fprintf(stderr, "Error: %d\n", err);
    return err;
}
```

---

## Memory Management Changes

### OLD Memory Management
```c
// OLD: Manual allocation
BigInt* a = (BigInt*)malloc(sizeof(BigInt));
bigint_init(a);
// ... use a ...
bigint_free(a);
free(a);
```

### NEW Memory Management
```c
// NEW: Integrated allocation
ClockContext ctx;
clock_init(&ctx);
CrystallineAbacus* a = abacus_create_from_uint64(0, 12, &ctx);
// ... use a ...
abacus_free(a);  // Frees everything
clock_cleanup(&ctx);
```

---

## Array Utilities

### OLD Array Functions
```c
BigFixed** bigfixed_array_create(size_t size, int precision);
void bigfixed_array_free(BigFixed** array, size_t size);
void bigfixed_array_zero(BigFixed** array, size_t size);
void bigfixed_array_copy(BigFixed** dest, BigFixed** src, size_t size);
void bigfixed_array_from_float(BigFixed** dest, const float* src, size_t size);
void bigfixed_array_to_float(float* dest, BigFixed** src, size_t size);
```

### NEW Array Functions
```c
// Create array
CrystallineAbacus** array = (CrystallineAbacus**)malloc(size * sizeof(CrystallineAbacus*));
for (size_t i = 0; i < size; i++) {
    array[i] = abacus_create_from_uint64(0, base, ctx);
}

// Free array
for (size_t i = 0; i < size; i++) {
    abacus_free(array[i]);
}
free(array);

// Zero array
for (size_t i = 0; i < size; i++) {
    abacus_set_zero(array[i]);
}

// Copy array
for (size_t i = 0; i < size; i++) {
    abacus_copy(dest[i], src[i]);
}

// From float array
for (size_t i = 0; i < size; i++) {
    abacus_from_double(src[i], base, precision, ctx, dest[i]);
}

// To float array
for (size_t i = 0; i < size; i++) {
    dest[i] = (float)abacus_to_double(src[i]);
}
```

---

## Migration Checklist

For each file being migrated:

- [ ] Update includes to use NEW math library
- [ ] Replace `BigInt` with `CrystallineAbacus`
- [ ] Replace `BigFixed` with `CrystallineAbacus`
- [ ] Add `ClockContext` initialization
- [ ] Update function calls to NEW API
- [ ] Add error handling for all operations
- [ ] Update memory management
- [ ] Update array utilities
- [ ] Test compilation
- [ ] Test functionality
- [ ] Verify no performance regression

---

## Common Pitfalls

1. **Forgetting ClockContext:** The NEW API requires a ClockContext for many operations
2. **Ignoring Error Codes:** The NEW API returns MathError - always check!
3. **Base Parameter:** The NEW API requires specifying a base (12, 60, 100, or any >= 2)
4. **Memory Management:** The NEW API integrates allocation - don't double-free!
5. **Include Paths:** Use `math/` prefix for NEW includes

---

## Benefits of NEW API

1. **O(1) Prime Generation:** Revolutionary deterministic formula
2. **Universal Base Support:** Works with ANY base >= 2
3. **Unified Type:** One type for integers and fixed-point
4. **Better Error Handling:** Explicit error codes
5. **Geometric Operations:** Leverages clock lattice structure
6. **No External Dependencies:** Pure crystalline mathematics
7. **Compact Vectors:** Optional memory-efficient representation
8. **Self-Similar Structure:** Recursive arithmetic support

---

**Last Updated:** 2024-12-19
**Status:** Complete API Mapping