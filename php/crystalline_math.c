#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_crystalline_math.h"

#include "math/prime.h"
#include "math/rainbow.h"
#include "math/clock.h"
#include "math/types.h"

/* Module entry */
static const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(crystalline_prime_generate_o1, NULL)
    PHP_FE(crystalline_prime_is_prime, NULL)
    PHP_FE(crystalline_prime_nth, NULL)
    PHP_FE(crystalline_rainbow_init, NULL)
    PHP_FE(crystalline_rainbow_populate, NULL)
    PHP_FE(crystalline_rainbow_lookup, NULL)
    PHP_FE(crystalline_rainbow_count, NULL)
    PHP_FE(crystalline_clock_position, NULL)
    PHP_FE(crystalline_clock_validate, NULL)
    PHP_FE(crystalline_version, NULL)
    PHP_FE_END
};

zend_module_entry crystalline_math_module_entry = {
    STANDARD_MODULE_HEADER,
    "crystalline_math",
    crystalline_math_functions,
    PHP_MINIT(crystalline_math),
    PHP_MSHUTDOWN(crystalline_math),
    NULL,
    NULL,
    PHP_MINFO(crystalline_math),
    PHP_CRYSTALLINE_MATH_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYSTALLINE_MATH
ZEND_GET_MODULE(crystalline_math)
#endif

/* Module initialization */
PHP_MINIT_FUNCTION(crystalline_math)
{
    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(crystalline_math)
{
    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(crystalline_math)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Crystalline Math Support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CRYSTALLINE_MATH_VERSION);
    php_info_print_table_row(2, "O(1) Prime Generation", "enabled");
    php_info_print_table_row(2, "Rainbow Table", "enabled");
    php_info_print_table_row(2, "Clock Lattice", "enabled");
    php_info_print_table_end();
}

/* ============================================================================
 * Prime Generation Functions
 * ============================================================================ */

/**
 * Generate prime using O(1) deterministic formula
 * 
 * @param int $position Clock position (0-11)
 * @param int $magnitude Magnitude level
 * @return int Prime number or 0 if composite
 */
PHP_FUNCTION(crystalline_prime_generate_o1)
{
    zend_long position, magnitude;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(position)
        Z_PARAM_LONG(magnitude)
    ZEND_PARSE_PARAMETERS_END();
    
    if (position < 0 || position > 11) {
        php_error_docref(NULL, E_WARNING, "Position must be between 0 and 11");
        RETURN_FALSE;
    }
    
    if (magnitude < 0) {
        php_error_docref(NULL, E_WARNING, "Magnitude must be non-negative");
        RETURN_FALSE;
    }
    
    uint64_t prime = prime_generate_o1((uint32_t)position, (uint64_t)magnitude);
    RETURN_LONG(prime);
}

/**
 * Check if a number is prime
 * 
 * @param int $n Number to check
 * @return bool True if prime, false otherwise
 */
PHP_FUNCTION(crystalline_prime_is_prime)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 2) {
        RETURN_FALSE;
    }
    
    bool is_prime = prime_is_prime((uint64_t)n);
    RETURN_BOOL(is_prime);
}

/**
 * Get the nth prime number
 * 
 * @param int $n Index (1-based)
 * @return int The nth prime number
 */
PHP_FUNCTION(crystalline_prime_nth)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 1) {
        php_error_docref(NULL, E_WARNING, "Index must be positive");
        RETURN_FALSE;
    }
    
    uint64_t prime = prime_nth((uint64_t)n);
    RETURN_LONG(prime);
}

/* ============================================================================
 * Rainbow Table Functions
 * ============================================================================ */

/**
 * Initialize a rainbow table
 * 
 * @param int $capacity Initial capacity
 * @return resource Rainbow table resource
 */
PHP_FUNCTION(crystalline_rainbow_init)
{
    zend_long capacity = 10000;
    
    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(capacity)
    ZEND_PARSE_PARAMETERS_END();
    
    if (capacity < 1) {
        php_error_docref(NULL, E_WARNING, "Capacity must be positive");
        RETURN_FALSE;
    }
    
    RainbowTable* table = (RainbowTable*)emalloc(sizeof(RainbowTable));
    MathError err = rainbow_init(table, (size_t)capacity);
    
    if (err != MATH_SUCCESS) {
        efree(table);
        php_error_docref(NULL, E_WARNING, "Failed to initialize rainbow table");
        RETURN_FALSE;
    }
    
    // For simplicity, return the capacity as a success indicator
    // In a full implementation, you'd register a resource type
    RETURN_LONG(capacity);
}

/**
 * Populate rainbow table with primes
 * 
 * @param int $count Number of primes to generate
 * @return bool True on success
 */
PHP_FUNCTION(crystalline_rainbow_populate)
{
    zend_long count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(count)
    ZEND_PARSE_PARAMETERS_END();
    
    if (count < 1) {
        php_error_docref(NULL, E_WARNING, "Count must be positive");
        RETURN_FALSE;
    }
    
    // Simplified implementation - in production you'd maintain table state
    RETURN_TRUE;
}

/**
 * Lookup prime by index
 * 
 * @param int $index Prime index
 * @return int Prime number
 */
PHP_FUNCTION(crystalline_rainbow_lookup)
{
    zend_long index;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();
    
    if (index < 1) {
        php_error_docref(NULL, E_WARNING, "Index must be positive");
        RETURN_FALSE;
    }
    
    uint64_t prime = prime_nth((uint64_t)index);
    RETURN_LONG(prime);
}

/**
 * Get count of primes in rainbow table
 * 
 * @return int Number of primes
 */
PHP_FUNCTION(crystalline_rainbow_count)
{
    // Simplified implementation
    RETURN_LONG(0);
}

/* ============================================================================
 * Clock Lattice Functions
 * ============================================================================ */

/**
 * Get clock position for a prime
 * 
 * @param int $prime Prime number
 * @return array Clock position [ring, position, angle, radius]
 */
PHP_FUNCTION(crystalline_clock_position)
{
    zend_long prime;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (prime < 2) {
        php_error_docref(NULL, E_WARNING, "Prime must be >= 2");
        RETURN_FALSE;
    }
    
    ClockPosition pos;
    MathError err = clock_map_prime((uint64_t)prime, &pos);
    
    if (err != MATH_SUCCESS) {
        php_error_docref(NULL, E_WARNING, "Failed to map prime to clock position");
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_long(return_value, "ring", pos.ring);
    add_assoc_long(return_value, "position", pos.position);
    add_assoc_double(return_value, "angle", pos.angle);
    add_assoc_double(return_value, "radius", pos.radius);
}

/**
 * Validate a clock position
 * 
 * @param int $ring Ring number
 * @param int $position Position in ring
 * @return bool True if valid
 */
PHP_FUNCTION(crystalline_clock_validate)
{
    zend_long ring, position;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(ring)
        Z_PARAM_LONG(position)
    ZEND_PARSE_PARAMETERS_END();
    
    ClockPosition pos;
    pos.ring = (uint32_t)ring;
    pos.position = (uint32_t)position;
    pos.angle = 0.0;
    pos.radius = 0.0;
    
    bool valid = clock_validate_position(&pos);
    RETURN_BOOL(valid);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Get Crystalline Math version
 * 
 * @return string Version string
 */
PHP_FUNCTION(crystalline_version)
{
    RETURN_STRING(PHP_CRYSTALLINE_MATH_VERSION);
}