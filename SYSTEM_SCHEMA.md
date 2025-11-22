# CLLM System Architecture Schema

## CRITICAL DESIGN PRINCIPLES

### 1. BIGINT IS THE DOMINANT TYPE SYSTEM
- **ALL calculations use BigInt for arbitrary precision**
- **Floating point is ONLY for:**
  - Type conversion to/from user input
  - Final output display
  - Interfacing with external libraries (SDL2, etc.)
- **NEVER use float/double for internal calculations**

## TYPE HIERARCHY

### Core Types (from prime_types.h)

#### BigInt - Primary Calculation Type
```c
typedef struct {
    uint32_t *d;              // Array of 32-bit digits
    size_t len;               // Current number of digits
    size_t capacity;          // Allocated capacity
    int negative;             // Sign: 0 = positive, 1 = negative
} BigInt;
```

**Operations:**
- `big_add()`, `big_sub()`, `big_mul()`, `big_div()` - Core arithmetic
- `big_from_int()` - Convert from uint64_t
- `big_to_string()` - Convert to string for display
- `big_cmp()` - Comparison
- `big_copy()` - Deep copy

#### BigFixed - Fixed-Point Arithmetic
```c
typedef struct {
    BigInt *integer_part;     // Integer portion
    BigInt *fractional_part;  // Fractional portion
    int negative;             // Sign
    size_t scale_bits;        // Fractional precision
} BigFixed;
```

#### CrystalAbacus - Prime Generator
```c
typedef struct {
    int *primes;              // Array of primes (regular int for small primes)
    size_t num_primes;
    size_t capacity;
    int candidate;
} CrystalAbacus;
```

#### CrystalAbacusBig - BigInt Prime Generator
```c
typedef struct {
    BigInt *primes;           // Array of BigInt primes
    size_t num_primes;
    size_t capacity;
    BigInt candidate;         // BigInt candidate
} CrystalAbacusBig;
```

## CLLM TYPE SYSTEM

### CLLMToken
```c
typedef struct {
    uint32_t token_id;           // Token ID (uint32_t - OK for vocab size)
    uint64_t prime_encoding;     // Prime encoding (uint64_t - OK for encoding)
    float lattice_coords[3];     // 3D coordinates (float - OK for visualization)
    float angle;                 // Angular position (float)
    float radius;                // Radial distance (float)
    char token_str[64];          // Token string
    float frequency;             // Token frequency
    uint32_t symmetry_group;
    float spiral_angle;
    float radial_distance;
} CLLMToken;
```

### CLLMLatticePoint
```c
typedef struct {
    uint32_t point_id;           // Point ID
    uint64_t prime;              // Associated prime (uint64_t - OK for reasonable primes)
    uint64_t prime_factor;       // Prime factorization
    float coords[3];             // 3D coordinates (float - for visualization)
    float angle;                 // Angular position
    float radius;                // Radial distance
    float resonance;             // Resonance value
    uint32_t neighbors[MAX_NEIGHBORS];
    uint32_t neighbor_ids[MAX_NEIGHBORS];
    uint32_t num_neighbors;
    uint32_t neighbor_count;
    float embedding[512];        // Embedding vector
    uint32_t symmetry_group;
} CLLMLatticePoint;
```

**IMPORTANT:** CLLMLatticePoint uses:
- `coords` NOT `coordinates`
- `prime` NOT `prime_value`
- `float` types for visualization (this is correct)
- `uint64_t` for prime values (sufficient for practical primes)

### CLLMModel
```c
typedef struct {
    CLLMHeader header;
    CLLMLatticePoint* lattice_points;
    uint64_t vocab_size;         // uint64_t
    uint64_t num_lattice_points; // uint64_t
    uint64_t embedding_dim;      // uint64_t
    float* weights;              // Neural network weights (float for NN)
    uint64_t num_weights;
    // ... embeddings, attention, feedforward layers
    uint32_t num_layers;
} CLLMModel;
```

## NAMING CONVENTIONS

### Function Naming Patterns

#### BigInt Operations (bigint_core.h)
- `big_*()` - Core BigInt operations
  - `big_add()`, `big_sub()`, `big_mul()`, `big_div()`
  - `big_init()`, `big_free()`, `big_copy()`
  - `big_from_int()`, `big_to_string()`
  - `big_cmp()`, `big_is_zero()`

#### Prime Operations (bigint_core.h)
- `big_prime_*()` - Prime-specific BigInt operations
  - `big_prime_add()`, `big_prime_subtract()`, `big_prime_multiply()`
  - These are WRAPPERS around big_add, big_sub, big_mul

#### Standard Math (prime_basic.c)
- `prime_*()` - Standard math on doubles (for compatibility)
  - `prime_add()`, `prime_subtract()`, `prime_multiply()`, `prime_divide()`
  - These operate on DOUBLE, not BigInt
  - Used for simple calculations and compatibility

#### CLLM Operations
- `cllm_*()` - CLLM model operations
  - `cllm_init()`, `cllm_free()`
  - `cllm_forward()`, `cllm_backward()`
  - `cllm_train()`, `cllm_inference()`

## CRITICAL ERRORS IN MY PREVIOUS FIXES

### ❌ WRONG: What I Did
1. Created `bigint_to_double()` conversions - **WRONG APPROACH**
2. Used these conversions in UI code - **BREAKS PRECISION**
3. Treated CLLMLatticePoint.coords as BigInt - **WRONG TYPE**
4. Created wrapper functions with confusing names - **NAMING COLLISION**

### ✅ CORRECT: What Should Be Done
1. CLLMLatticePoint.coords is ALREADY float - **USE DIRECTLY**
2. CLLMLatticePoint.prime is ALREADY uint64_t - **USE DIRECTLY**
3. No conversion needed - **TYPES ARE CORRECT**
4. The original code was mostly correct - **I BROKE IT**

## DATA FLOW

### Calculation Flow
```
Input (user) 
  → Convert to BigInt (big_from_int)
  → Calculate with BigInt (big_add, big_mul, etc.)
  → Convert to display (big_to_string or extract for float display)
  → Output (user)
```

### CLLM Flow
```
Tokens (uint32_t IDs)
  → Embeddings (float arrays for NN)
  → Lattice Points (float coords, uint64_t primes)
  → Attention/FF (float calculations for NN)
  → Output logits (float)
  → Sample token (uint32_t ID)
```

## WHERE BIGINT IS USED

### Core Math Library
- ✅ All prime calculations
- ✅ Arbitrary precision arithmetic
- ✅ Lattice geometry calculations (internal)
- ✅ Coordinate transformations (internal)

### Where BigInt is NOT Used
- ❌ Neural network weights (float for GPU compatibility)
- ❌ Visualization coordinates (float for SDL2)
- ❌ Token IDs (uint32_t sufficient)
- ❌ Embedding vectors (float for NN)

## CORRECT TYPE USAGE

### Calculator (app/calculator.c)
```c
// Calculator uses DOUBLE for user interface
double calc_value1;  // User input
double calc_value2;  // User input
double calc_result;  // Display result

// Operations use prime_add (double version)
result = prime_add(value1, value2);  // CORRECT - operates on double
```

### Lattice Visualization (app/ui/tabs/tab_llm.c)
```c
CLLMLatticePoint* pt = &model->lattice_points[i];

// coords is ALREADY float - use directly
int x = cx + (int)(pt->coords[0] * 250);  // CORRECT
int y = cy + (int)(pt->coords[1] * 250);  // CORRECT

// prime is ALREADY uint64_t - use directly
uint8_t r = (uint8_t)((pt->prime * 137) % 200 + 55);  // CORRECT
```

## FUNCTION REFERENCE GUIDE

### When to use big_* functions
- Internal calculations requiring arbitrary precision
- Prime number operations
- Lattice geometry (internal calculations)

### When to use prime_* (double) functions
- User interface calculations
- Simple arithmetic for display
- Compatibility with existing code

### When to use float directly
- Neural network operations
- Visualization coordinates
- SDL2 rendering
- Embedding vectors

## NEXT STEPS TO FIX MY MISTAKES

1. **REVERT** bigint_conversions.c - NOT NEEDED
2. **REVERT** changes to tab_llm.c - coords are already float
3. **VERIFY** all type usage matches this schema
4. **DOCUMENT** any remaining issues with proper context