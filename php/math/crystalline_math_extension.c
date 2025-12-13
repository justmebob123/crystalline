/*
  Crystalline Math PHP Extension
  
  Provides PHP bindings for the Crystalline Mathematics Library
  Focus: Prime numbers, GCD, mathematical operations
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_crystalline_math.h"

// Include the crystalline math library headers
#include "../../math/include/math/types.h"
#include "../../math/include/math/basic.h"
#include "../../math/include/math/prime.h"
#include "../../math/include/math/gcd.h"

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(crystalline_math)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(crystalline_math)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(crystalline_math)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Crystalline Math Support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CRYSTALLINE_MATH_VERSION);
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto int crystalline_gcd(int a, int b)
   Calculate GCD of two numbers */
PHP_FUNCTION(crystalline_gcd)
{
    zend_long a, b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(a)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();
    
    uint64_t result = math_gcd((uint64_t)a, (uint64_t)b);
    RETURN_LONG((zend_long)result);
}
/* }}} */

/* {{{ proto bool crystalline_is_prime(int n)
   Check if a number is prime */
PHP_FUNCTION(crystalline_is_prime)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 2) {
        RETURN_FALSE;
    }
    
    bool result = math_is_prime((uint64_t)n);
    RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto int crystalline_next_prime(int n)
   Get the next prime number after n */
PHP_FUNCTION(crystalline_next_prime)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    uint64_t result = math_next_prime((uint64_t)n);
    RETURN_LONG((zend_long)result);
}
/* }}} */

/* {{{ proto array crystalline_prime_factors(int n)
   Get prime factorization of n */
PHP_FUNCTION(crystalline_prime_factors)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 2) {
        array_init(return_value);
        return;
    }
    
    array_init(return_value);
    
    uint64_t num = (uint64_t)n;
    
    // Factor out 2s
    while (num % 2 == 0) {
        add_next_index_long(return_value, 2);
        num /= 2;
    }
    
    // Factor out odd primes
    for (uint64_t i = 3; i * i <= num; i += 2) {
        while (num % i == 0) {
            add_next_index_long(return_value, (zend_long)i);
            num /= i;
        }
    }
    
    // If num is still > 1, it's a prime factor
    if (num > 1) {
        add_next_index_long(return_value, (zend_long)num);
    }
}
/* }}} */

/* {{{ proto array crystalline_sieve(int limit)
   Generate all primes up to limit using Sieve of Eratosthenes */
PHP_FUNCTION(crystalline_sieve)
{
    zend_long limit;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(limit)
    ZEND_PARSE_PARAMETERS_END();
    
    if (limit < 2) {
        array_init(return_value);
        return;
    }
    
    array_init(return_value);
    
    // Simple sieve implementation
    bool *is_prime = (bool *)ecalloc(limit + 1, sizeof(bool));
    for (zend_long i = 2; i <= limit; i++) {
        is_prime[i] = true;
    }
    
    for (zend_long i = 2; i * i <= limit; i++) {
        if (is_prime[i]) {
            for (zend_long j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    
    for (zend_long i = 2; i <= limit; i++) {
        if (is_prime[i]) {
            add_next_index_long(return_value, i);
        }
    }
    
    efree(is_prime);
}
/* }}} */

/* {{{ proto float crystalline_sqrt(float x)
   Calculate square root */
PHP_FUNCTION(crystalline_sqrt)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    if (x < 0) {
        php_error_docref(NULL, E_WARNING, "Cannot calculate square root of negative number");
        RETURN_FALSE;
    }
    
    double result = math_sqrt(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float crystalline_pow(float base, float exp)
   Calculate power */
PHP_FUNCTION(crystalline_pow)
{
    double base, exp;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(base)
        Z_PARAM_DOUBLE(exp)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_pow(base, exp);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float crystalline_sin(float x)
   Calculate sine */
PHP_FUNCTION(crystalline_sin)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_sin(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float crystalline_cos(float x)
   Calculate cosine */
PHP_FUNCTION(crystalline_cos)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_cos(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float crystalline_log(float x)
   Calculate natural logarithm */
PHP_FUNCTION(crystalline_log)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    if (x <= 0) {
        php_error_docref(NULL, E_WARNING, "Cannot calculate logarithm of non-positive number");
        RETURN_FALSE;
    }
    
    double result = math_log(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float crystalline_exp(float x)
   Calculate exponential */
PHP_FUNCTION(crystalline_exp)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_exp(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ crystalline_math_functions[]
 */
const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(crystalline_gcd,           NULL)
    PHP_FE(crystalline_is_prime,      NULL)
    PHP_FE(crystalline_next_prime,    NULL)
    PHP_FE(crystalline_prime_factors, NULL)
    PHP_FE(crystalline_sieve,         NULL)
    PHP_FE(crystalline_sqrt,          NULL)
    PHP_FE(crystalline_pow,           NULL)
    PHP_FE(crystalline_sin,           NULL)
    PHP_FE(crystalline_cos,           NULL)
    PHP_FE(crystalline_log,           NULL)
    PHP_FE(crystalline_exp,           NULL)
    PHP_FE_END
};
/* }}} */

/* {{{ crystalline_math_module_entry
 */
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
/* }}} */

#ifdef COMPILE_DL_CRYSTALLINE_MATH
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(crystalline_math)
#endif