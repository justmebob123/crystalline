# PHP Module Redesign Plan

**Date:** December 13, 2024  
**Priority:** Mathematics libraries FIRST, CLLM secondary

---

## Objective

Create individual PHP modules and REST APIs for each library, with **hyper focus on mathematics libraries**.

---

## Libraries Priority Order

### PRIMARY FOCUS (Mathematics)
1. **libcrystallinemath.so** - Core mathematics library
   - Prime number operations
   - GCD calculations
   - Mathematical functions
   - Crystalline mathematics

2. **libalgorithms.so** - Algorithms library
   - Numerical algorithms
   - Optimization algorithms
   - Statistical functions
   - Mathematical algorithms

### SECONDARY FOCUS
3. **libcllm.so** - CLLM AI library
   - Model training
   - Inference
   - Vocabulary management

### OPTIONAL
4. **libcrawler.so** - Web crawler
5. **libdocproc.so** - Document processing

---

## Module Structure

Each library will have:
1. **PHP Extension** - Native C extension for direct library access
2. **REST API** - HTTP API for remote access
3. **Documentation** - API documentation with examples
4. **Tests** - Comprehensive test suite

---

## Implementation Plan

### Phase 1: Crystalline Math Module (PRIMARY)

**PHP Extension:**
- `php_crystalline_math.so`
- Functions:
  - `crystalline_gcd($a, $b)` - GCD calculation
  - `crystalline_is_prime($n)` - Prime check
  - `crystalline_next_prime($n)` - Next prime
  - `crystalline_prime_factors($n)` - Prime factorization
  - `crystalline_sieve($limit)` - Prime sieve
  - All math functions from the library

**REST API:**
- Endpoint: `/api/math/*`
- Routes:
  - `POST /api/math/gcd` - Calculate GCD
  - `POST /api/math/prime/check` - Check if prime
  - `POST /api/math/prime/next` - Get next prime
  - `POST /api/math/prime/factors` - Get prime factors
  - `POST /api/math/prime/sieve` - Generate primes up to limit

**Files:**
- `php/math/crystalline_math_extension.c`
- `php/math/php_crystalline_math.h`
- `php/math/config.m4`
- `php/math/rest_api.php`
- `php/math/test.php`
- `php/math/README.md`

### Phase 2: Algorithms Module (PRIMARY)

**PHP Extension:**
- `php_algorithms.so`
- Functions:
  - `algo_optimize($function, $params)` - Optimization
  - `algo_statistics($data)` - Statistical analysis
  - `algo_numerical($operation, $data)` - Numerical operations
  - All algorithm functions

**REST API:**
- Endpoint: `/api/algo/*`
- Routes:
  - `POST /api/algo/optimize` - Run optimization
  - `POST /api/algo/stats` - Calculate statistics
  - `POST /api/algo/numerical` - Numerical operations

**Files:**
- `php/algorithms/algorithms_extension.c`
- `php/algorithms/php_algorithms.h`
- `php/algorithms/config.m4`
- `php/algorithms/rest_api.php`
- `php/algorithms/test.php`
- `php/algorithms/README.md`

### Phase 3: CLLM Module (SECONDARY)

**PHP Extension:**
- `php_cllm.so`
- Functions:
  - `cllm_create_model($config)` - Create model
  - `cllm_train($model, $data)` - Train model
  - `cllm_infer($model, $prompt)` - Run inference
  - `cllm_save($model, $path)` - Save model
  - `cllm_load($path)` - Load model

**REST API:**
- Endpoint: `/api/cllm/*`
- Routes:
  - `POST /api/cllm/create` - Create model
  - `POST /api/cllm/train` - Train model
  - `POST /api/cllm/infer` - Run inference
  - `POST /api/cllm/save` - Save model
  - `POST /api/cllm/load` - Load model

**Files:**
- `php/cllm/cllm_extension.c`
- `php/cllm/php_cllm.h`
- `php/cllm/config.m4`
- `php/cllm/rest_api.php`
- `php/cllm/test.php`
- `php/cllm/README.md`

---

## Directory Structure

```
php/
├── math/                          # PRIMARY FOCUS
│   ├── crystalline_math_extension.c
│   ├── php_crystalline_math.h
│   ├── config.m4
│   ├── rest_api.php
│   ├── test.php
│   ├── README.md
│   └── Makefile
├── algorithms/                    # PRIMARY FOCUS
│   ├── algorithms_extension.c
│   ├── php_algorithms.h
│   ├── config.m4
│   ├── rest_api.php
│   ├── test.php
│   ├── README.md
│   └── Makefile
├── cllm/                         # SECONDARY
│   ├── cllm_extension.c
│   ├── php_cllm.h
│   ├── config.m4
│   ├── rest_api.php
│   ├── test.php
│   ├── README.md
│   └── Makefile
└── README.md                     # Main documentation
```

---

## Build System

Each module will have its own Makefile:

```makefile
# Example: php/math/Makefile
PHPIZE = phpize
PHP_CONFIG = php-config
EXTENSION_DIR = $(shell $(PHP_CONFIG) --extension-dir)

all: build

build:
	$(PHPIZE)
	./configure --with-php-config=$(PHP_CONFIG)
	make
	
install:
	make install
	echo "extension=crystalline_math.so" > /etc/php/conf.d/crystalline_math.ini

clean:
	make clean
	$(PHPIZE) --clean

test:
	php test.php
```

---

## API Documentation Format

Each REST API will have OpenAPI/Swagger documentation:

```yaml
openapi: 3.0.0
info:
  title: Crystalline Math API
  version: 1.0.0
paths:
  /api/math/gcd:
    post:
      summary: Calculate GCD
      requestBody:
        content:
          application/json:
            schema:
              type: object
              properties:
                a: { type: integer }
                b: { type: integer }
      responses:
        200:
          description: GCD result
          content:
            application/json:
              schema:
                type: object
                properties:
                  result: { type: integer }
```

---

## Testing Strategy

1. **Unit Tests** - Test each function individually
2. **Integration Tests** - Test REST API endpoints
3. **Performance Tests** - Benchmark operations
4. **Load Tests** - Test under high load

---

## Timeline

1. **Phase 1 (Math Module):** 2-3 hours
2. **Phase 2 (Algorithms Module):** 2-3 hours
3. **Phase 3 (CLLM Module):** 2-3 hours

**Total:** 6-9 hours for complete implementation

---

## Success Criteria

- ✅ Each library has a working PHP extension
- ✅ Each library has a REST API
- ✅ All functions are documented
- ✅ All tests pass
- ✅ Performance is acceptable
- ✅ Code is maintainable

---

## Next Steps

1. Start with Crystalline Math module (PRIMARY FOCUS)
2. Implement PHP extension
3. Create REST API
4. Write tests
5. Document everything
6. Move to Algorithms module
7. Finally implement CLLM module (SECONDARY)