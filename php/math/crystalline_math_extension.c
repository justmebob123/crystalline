/*
  Crystalline Math PHP Extension
  
  Provides PHP bindings for the Crystalline Mathematics Library
  Focus: Arithmetic operations, prime numbers, mathematical functions
  
  IMPORTANT: This uses CUSTOM arbitrary precision math library
  NO standard math.h functions are used in the core library
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
#include "../../math/include/math/validation.h"

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

/* {{{ proto double math_add(double a, double b) */
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

/* {{{ proto double math_sub(double a, double b) */
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

/* {{{ proto double math_mul(double a, double b) */
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

/* {{{ proto double math_div(double a, double b) */
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

/* {{{ proto double math_mod(double a, double b) */
PHP_FUNCTION(math_mod)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_mod(a, b));
}
/* }}} */

/* {{{ proto double math_abs(double x) */
PHP_FUNCTION(math_abs)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_abs(x));
}
/* }}} */

/* {{{ proto double math_min(double a, double b) */
PHP_FUNCTION(math_min)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_min(a, b));
}
/* }}} */

/* {{{ proto double math_max(double a, double b) */
PHP_FUNCTION(math_max)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_max(a, b));
}
/* }}} */

/* {{{ proto double math_clamp(double x, double min, double max) */
PHP_FUNCTION(math_clamp)
{
    double x, min_val, max_val;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(x)
        Z_PARAM_DOUBLE(min_val)
        Z_PARAM_DOUBLE(max_val)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_clamp(x, min_val, max_val));
}
/* }}} */

/* {{{ proto int math_sign(double x) */
PHP_FUNCTION(math_sign)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(math_sign(x));
}
/* }}} */

/* {{{ proto double math_floor(double x) */
PHP_FUNCTION(math_floor)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_floor(x));
}
/* }}} */

/* {{{ proto double math_ceil(double x) */
PHP_FUNCTION(math_ceil)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_ceil(x));
}
/* }}} */

/* {{{ proto double math_round(double x) */
PHP_FUNCTION(math_round)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_round(x));
}
/* }}} */

/* {{{ proto double math_trunc(double x) */
PHP_FUNCTION(math_trunc)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_trunc(x));
}
/* }}} */

/* {{{ proto double math_sqrt(double x) */
PHP_FUNCTION(math_sqrt)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sqrt(x));
}
/* }}} */

/* {{{ proto double math_pow(double base, double exp) */
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

/* {{{ proto double math_sin(double x) */
PHP_FUNCTION(math_sin)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sin(x));
}
/* }}} */

/* {{{ proto double math_cos(double x) */
PHP_FUNCTION(math_cos)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_cos(x));
}
/* }}} */

/* {{{ proto double math_tan(double x) */
PHP_FUNCTION(math_tan)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_tan(x));
}
/* }}} */

/* {{{ proto double math_asin(double x) */
PHP_FUNCTION(math_asin)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_asin(x));
}
/* }}} */

/* {{{ proto double math_acos(double x) */
PHP_FUNCTION(math_acos)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_acos(x));
}
/* }}} */

/* {{{ proto double math_atan(double x) */
PHP_FUNCTION(math_atan)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_atan(x));
}
/* }}} */

/* {{{ proto double math_sinh(double x) */
PHP_FUNCTION(math_sinh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sinh(x));
}
/* }}} */

/* {{{ proto double math_cosh(double x) */
PHP_FUNCTION(math_cosh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_cosh(x));
}
/* }}} */

/* {{{ proto double math_tanh(double x) */
PHP_FUNCTION(math_tanh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_tanh(x));
}
/* }}} */

/* {{{ proto double math_cbrt(double x) */
PHP_FUNCTION(math_cbrt)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_cbrt(x));
}
/* }}} */

/* {{{ proto double math_exp(double x) */
PHP_FUNCTION(math_exp)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_exp(x));
}
/* }}} */

/* {{{ proto double math_log(double x) */
PHP_FUNCTION(math_log)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log(x));
}
/* }}} */

/* {{{ proto double math_log10(double x) */
PHP_FUNCTION(math_log10)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log10(x));
}
/* }}} */

/* {{{ proto double math_log2(double x) */
PHP_FUNCTION(math_log2)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log2(x));
}
/* }}} */

/* {{{ proto bool math_is_nan(double x) */
PHP_FUNCTION(math_is_nan)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_is_nan(x));
}
/* }}} */

/* {{{ proto bool math_is_inf(double x) */
PHP_FUNCTION(math_is_inf)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_is_inf(x));
}
/* }}} */

/* {{{ proto bool math_is_finite(double x) */
PHP_FUNCTION(math_is_finite)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_is_finite(x));
}
/* }}} */

/* {{{ proto bool math_approx_equal(double a, double b, double epsilon) */
PHP_FUNCTION(math_approx_equal)
{
    double a, b, epsilon;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
        Z_PARAM_DOUBLE(epsilon)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_approx_equal(a, b, epsilon));
}
/* }}} */

/* {{{ proto bool is_prime(int n) */
PHP_FUNCTION(is_prime)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(prime_is_prime((uint64_t)n));
}
/* }}} */

/* {{{ proto int crystalline_prime_generate_o1(int position, int magnitude) */
PHP_FUNCTION(crystalline_prime_generate_o1)
{
    zend_long position, magnitude;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(position)
        Z_PARAM_LONG(magnitude)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_generate_o1((uint32_t)position, (uint64_t)magnitude));
}
/* }}} */

/* {{{ proto int prime_nth(int n) */
PHP_FUNCTION(prime_nth)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_nth((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_next(int n) */
PHP_FUNCTION(prime_next)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_next((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_prev(int n) */
PHP_FUNCTION(prime_prev)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_prev((uint64_t)n));
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_math_binary, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_unary, 0)
    ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_clamp, 0)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, min)
    ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_prime, 0)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_prime_generate, 0)
    ZEND_ARG_INFO(0, position)
    ZEND_ARG_INFO(0, magnitude)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ crystalline_math_functions[] */
const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(math_add, arginfo_math_binary)
    PHP_FE(math_sub, arginfo_math_binary)
    PHP_FE(math_mul, arginfo_math_binary)
    PHP_FE(math_div, arginfo_math_binary)
    PHP_FE(math_mod, arginfo_math_binary)
    PHP_FE(math_abs, arginfo_math_unary)
    PHP_FE(math_min, arginfo_math_binary)
    PHP_FE(math_max, arginfo_math_binary)
    PHP_FE(math_clamp, arginfo_math_clamp)
    PHP_FE(math_sign, arginfo_math_unary)
    PHP_FE(math_floor, arginfo_math_unary)
    PHP_FE(math_ceil, arginfo_math_unary)
    PHP_FE(math_round, arginfo_math_unary)
    PHP_FE(math_trunc, arginfo_math_unary)
    PHP_FE(math_sqrt, arginfo_math_unary)
    PHP_FE(math_cbrt, arginfo_math_unary)
    PHP_FE(math_pow, arginfo_math_binary)
    PHP_FE(math_exp, arginfo_math_unary)
    PHP_FE(math_log, arginfo_math_unary)
    PHP_FE(math_log10, arginfo_math_unary)
    PHP_FE(math_log2, arginfo_math_unary)
    PHP_FE(math_sin, arginfo_math_unary)
    PHP_FE(math_cos, arginfo_math_unary)
    PHP_FE(math_tan, arginfo_math_unary)
    PHP_FE(math_asin, arginfo_math_unary)
    PHP_FE(math_acos, arginfo_math_unary)
    PHP_FE(math_atan, arginfo_math_unary)
    PHP_FE(math_sinh, arginfo_math_unary)
    PHP_FE(math_cosh, arginfo_math_unary)
    PHP_FE(math_tanh, arginfo_math_unary)
    PHP_FE(math_is_nan, arginfo_math_unary)
    PHP_FE(math_is_inf, arginfo_math_unary)
    PHP_FE(math_is_finite, arginfo_math_unary)
    PHP_FE(math_approx_equal, arginfo_math_clamp)
    PHP_FE(is_prime, arginfo_is_prime)
    PHP_FE(crystalline_prime_generate_o1, arginfo_prime_generate)
    PHP_FE(prime_nth, arginfo_is_prime)
    PHP_FE(prime_next, arginfo_is_prime)
    PHP_FE(prime_prev, arginfo_is_prime)
    PHP_FE_END
};
/* }}} */

/* {{{ crystalline_math_module_entry */
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