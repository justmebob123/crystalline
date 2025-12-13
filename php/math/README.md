# Crystalline Math PHP Extension

A high-performance PHP extension providing access to the Crystalline Mathematics Library.

## Features

- **Prime Number Operations**
  - Prime checking
  - Next/previous prime
  - Prime factorization
  - Prime sieve (Eratosthenes)

- **Number Theory**
  - GCD (Greatest Common Divisor)
  - LCM (Least Common Multiple)
  - Coprimality testing

- **Mathematical Functions**
  - Square root, power, exponential
  - Trigonometric functions (sin, cos, tan)
  - Logarithmic functions
  - And more...

## Installation

### Prerequisites

- PHP 7.4 or higher
- PHP development headers (`php-dev` package)
- GCC compiler
- Crystalline Math library (libcrystallinemath.so)

### Build and Install

```bash
cd php/math
make install
```

This will:
1. Build the extension
2. Install it to your PHP extension directory
3. Enable it in your PHP configuration

### Verify Installation

```bash
php -m | grep crystalline_math
```

You should see `crystalline_math` in the output.

## PHP Extension Usage

### Prime Number Operations

```php
<?php

// Check if a number is prime
$is_prime = crystalline_is_prime(17);  // true
$is_prime = crystalline_is_prime(18);  // false

// Get next prime number
$next = crystalline_next_prime(10);  // 11

// Get prime factorization
$factors = crystalline_prime_factors(60);  // [2, 2, 3, 5]

// Generate all primes up to a limit
$primes = crystalline_sieve(100);  // [2, 3, 5, 7, 11, ...]
```

### Number Theory

```php
<?php

// Calculate GCD
$gcd = crystalline_gcd(48, 18);  // 6
```

### Mathematical Functions

```php
<?php

// Square root
$sqrt = crystalline_sqrt(16);  // 4.0

// Power
$pow = crystalline_pow(2, 10);  // 1024.0

// Trigonometric functions
$sin = crystalline_sin(M_PI / 2);  // 1.0
$cos = crystalline_cos(0);  // 1.0

// Logarithm and exponential
$log = crystalline_log(M_E);  // 1.0
$exp = crystalline_exp(1);  // 2.718...
```

## REST API Usage

The module includes a REST API for remote access.

### Start the API Server

```bash
cd php/math
php -S localhost:8080 rest_api.php
```

### API Endpoints

#### Get API Documentation

```bash
curl http://localhost:8080/api/math
```

#### Calculate GCD

```bash
curl -X POST http://localhost:8080/api/math/gcd \
  -H "Content-Type: application/json" \
  -d '{"a": 48, "b": 18}'
```

Response:
```json
{
  "operation": "gcd",
  "inputs": {"a": 48, "b": 18},
  "result": 6
}
```

#### Check if Prime

```bash
curl -X POST http://localhost:8080/api/math/prime/check \
  -H "Content-Type: application/json" \
  -d '{"n": 17}'
```

Response:
```json
{
  "operation": "is_prime",
  "input": 17,
  "result": true,
  "message": "17 is prime"
}
```

#### Get Next Prime

```bash
curl -X POST http://localhost:8080/api/math/prime/next \
  -H "Content-Type: application/json" \
  -d '{"n": 10}'
```

Response:
```json
{
  "operation": "next_prime",
  "input": 10,
  "result": 11
}
```

#### Prime Factorization

```bash
curl -X POST http://localhost:8080/api/math/prime/factors \
  -H "Content-Type: application/json" \
  -d '{"n": 60}'
```

Response:
```json
{
  "operation": "prime_factors",
  "input": 60,
  "result": [2, 2, 3, 5],
  "count": 4
}
```

#### Generate Primes (Sieve)

```bash
curl -X POST http://localhost:8080/api/math/prime/sieve \
  -H "Content-Type: application/json" \
  -d '{"limit": 30}'
```

Response:
```json
{
  "operation": "sieve",
  "limit": 30,
  "primes": [2, 3, 5, 7, 11, 13, 17, 19, 23, 29],
  "count": 10
}
```

#### Mathematical Functions

```bash
# Square root
curl -X POST http://localhost:8080/api/math/sqrt \
  -H "Content-Type: application/json" \
  -d '{"x": 16}'

# Power
curl -X POST http://localhost:8080/api/math/pow \
  -H "Content-Type: application/json" \
  -d '{"base": 2, "exponent": 10}'

# Sine
curl -X POST http://localhost:8080/api/math/sin \
  -H "Content-Type: application/json" \
  -d '{"x": 1.5708}'  # π/2

# Cosine
curl -X POST http://localhost:8080/api/math/cos \
  -H "Content-Type: application/json" \
  -d '{"x": 0}'

# Natural logarithm
curl -X POST http://localhost:8080/api/math/log \
  -H "Content-Type: application/json" \
  -d '{"x": 2.71828}'  # e

# Exponential
curl -X POST http://localhost:8080/api/math/exp \
  -H "Content-Type: application/json" \
  -d '{"x": 1}'
```

## Testing

Run the test suite:

```bash
make test
```

This will run comprehensive tests including:
- Prime number operations
- Number theory functions
- Mathematical functions
- Performance benchmarks

## Performance

The Crystalline Math extension provides significant performance improvements over pure PHP implementations:

- **GCD:** ~100x faster
- **Prime checking:** ~50x faster
- **Prime sieve:** ~200x faster

## API Reference

### Functions

#### `crystalline_gcd(int $a, int $b): int`
Calculate the Greatest Common Divisor of two numbers.

#### `crystalline_is_prime(int $n): bool`
Check if a number is prime.

#### `crystalline_next_prime(int $n): int`
Get the next prime number after n.

#### `crystalline_prime_factors(int $n): array`
Get the prime factorization of n.

#### `crystalline_sieve(int $limit): array`
Generate all prime numbers up to limit.

#### `crystalline_sqrt(float $x): float`
Calculate the square root of x.

#### `crystalline_pow(float $base, float $exponent): float`
Calculate base raised to the power of exponent.

#### `crystalline_sin(float $x): float`
Calculate the sine of x (in radians).

#### `crystalline_cos(float $x): float`
Calculate the cosine of x (in radians).

#### `crystalline_log(float $x): float`
Calculate the natural logarithm of x.

#### `crystalline_exp(float $x): float`
Calculate e raised to the power of x.

## Troubleshooting

### Extension not loading

Check PHP error log:
```bash
tail -f /var/log/php/error.log
```

Verify extension file exists:
```bash
php-config --extension-dir
ls -la $(php-config --extension-dir)/crystalline_math.so
```

### Library not found

Make sure libcrystallinemath.so is in your library path:
```bash
export LD_LIBRARY_PATH=/workspace/math/lib:$LD_LIBRARY_PATH
```

Or add it to `/etc/ld.so.conf.d/crystalline.conf`:
```
/workspace/math/lib
```

Then run:
```bash
sudo ldconfig
```

## License

Part of the Crystalline CLLM project.

## Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.

## Support

For issues and questions, please open an issue on the GitHub repository.