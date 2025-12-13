/*
  Crystalline Math PHP Extension
  
  Provides PHP bindings for the Crystalline Mathematics Library
  Focus: Arithmetic operations, prime numbers, mathematical functions
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
#include "../../math/include/math/arithmetic.h"
#include "../../math/include/math/prime.h"
#include "../../math/include/math/transcendental.h"

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
    php_info_print_table_header(2, "crystalline_math support", "enabled");
    php_info_print_table_row(2, "Version", "1.0.0");
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto double math_add(double a, double b)
   Add two numbers */
PHP_FUNCTION(math_add)
{
    double a, b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_add(a, b));
}
/* }}} */

/* {{{ proto double math_sub(double a, double b)
   Subtract two numbers */
PHP_FUNCTION(math_sub)
{
    double a, b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_sub(a, b));
}
/* }}} */

/* {{{ proto double math_mul(double a, double b)
   Multiply two numbers */
PHP_FUNCTION(math_mul)
{
    double a, b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_mul(a, b));
}
/* }}} */

/* {{{ proto double math_div(double a, double b)
   Divide two numbers */
PHP_FUNCTION(math_div)
{
    double a, b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_div(a, b));
}
/* }}} */

/* {{{ proto double math_sqrt(double x)
   Square root */
PHP_FUNCTION(math_sqrt)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_sqrt(x));
}
/* }}} */

/* {{{ proto double math_pow(double base, double exp)
   Power function */
PHP_FUNCTION(math_pow)
{
    double base, exp;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(base)
        Z_PARAM_DOUBLE(exp)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_pow(base, exp));
}
/* }}} */

/* {{{ proto double math_sin(double x)
   Sine function */
PHP_FUNCTION(math_sin)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_sin(x));
}
/* }}} */

/* {{{ proto double math_cos(double x)
   Cosine function */
PHP_FUNCTION(math_cos)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(math_cos(x));
}
/* }}} */

/* {{{ proto bool is_prime(int n)
   Check if number is prime */
PHP_FUNCTION(is_prime)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_BOOL(prime_is_prime((uint64_t)n));
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_math_add, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_sub, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_mul, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_div, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_sqrt, 0)
    ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_pow, 0)
    ZEND_ARG_INFO(0, base)
    ZEND_ARG_INFO(0, exp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_sin, 0)
    ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_cos, 0)
    ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_prime, 0)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ crystalline_math_functions[]
 */
const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(math_add, arginfo_math_add)
    PHP_FE(math_sub, arginfo_math_sub)
    PHP_FE(math_mul, arginfo_math_mul)
    PHP_FE(math_div, arginfo_math_div)
    PHP_FE(math_sqrt, arginfo_math_sqrt)
    PHP_FE(math_pow, arginfo_math_pow)
    PHP_FE(math_sin, arginfo_math_sin)
    PHP_FE(math_cos, arginfo_math_cos)
    PHP_FE(is_prime, arginfo_is_prime)
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
    "1.0.0",
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CRYSTALLINE_MATH
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(crystalline_math)
#endif