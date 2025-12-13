/*
  Algorithms PHP Extension
  
  Provides PHP bindings for the Algorithms Library
  Focus: Numerical algorithms, optimization, statistics
  
  IMPORTANT: This uses CUSTOM arbitrary precision math library
  NO standard math.h functions except where absolutely required by PHP
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_algorithms.h"

// Include the algorithms library headers
#include "../../algorithms/include/numerical.h"
#include "../../algorithms/include/statistics.h"
#include "../../algorithms/include/loss_functions.h"

/* Helper function to convert PHP array to C double array */
static double* php_array_to_double(zval *arr, size_t *count) {
    HashTable *hash = Z_ARRVAL_P(arr);
    *count = zend_hash_num_elements(hash);
    
    if (*count == 0) {
        return NULL;
    }
    
    double *values = (double *)emalloc(*count * sizeof(double));
    size_t i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(hash, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    return values;
}

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(algorithms)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(algorithms)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(algorithms)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Algorithms Support", "enabled");
    php_info_print_table_row(2, "Version", "1.0.0");
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto array numerical_softmax(array $data) */
PHP_FUNCTION(numerical_softmax)
{
    zval *data;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    double *input = php_array_to_double(data, &count);
    if (!input) {
        RETURN_EMPTY_ARRAY();
    }
    
    double *output = (double *)emalloc(count * sizeof(double));
    numerical_softmax(input, output, count);
    
    array_init(return_value);
    for (size_t i = 0; i < count; i++) {
        add_next_index_double(return_value, output[i]);
    }
    
    efree(input);
    efree(output);
}
/* }}} */

/* {{{ proto array numerical_log_softmax(array $data) */
PHP_FUNCTION(numerical_log_softmax)
{
    zval *data;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    double *input = php_array_to_double(data, &count);
    if (!input) {
        RETURN_EMPTY_ARRAY();
    }
    
    double *output = (double *)emalloc(count * sizeof(double));
    numerical_log_softmax(input, output, count);
    
    array_init(return_value);
    for (size_t i = 0; i < count; i++) {
        add_next_index_double(return_value, output[i]);
    }
    
    efree(input);
    efree(output);
}
/* }}} */

/* {{{ proto double numerical_safe_log(double $x, double $epsilon) */
PHP_FUNCTION(numerical_safe_log)
{
    double x, epsilon;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(x)
        Z_PARAM_DOUBLE(epsilon)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(numerical_safe_log(x, epsilon));
}
/* }}} */

/* {{{ proto double numerical_safe_divide(double $a, double $b, double $default) */
PHP_FUNCTION(numerical_safe_divide)
{
    double a, b, default_val;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
        Z_PARAM_DOUBLE(default_val)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(numerical_safe_divide(a, b, default_val));
}
/* }}} */

/* {{{ proto double numerical_clip(double $x, double $min, double $max) */
PHP_FUNCTION(numerical_clip)
{
    double x, min_val, max_val;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(x)
        Z_PARAM_DOUBLE(min_val)
        Z_PARAM_DOUBLE(max_val)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_DOUBLE(numerical_clip(x, min_val, max_val));
}
/* }}} */

/* {{{ proto double stats_mean(array $data) */
PHP_FUNCTION(stats_mean)
{
    zval *data;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_mean(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_variance(array $data, double $mean) */
PHP_FUNCTION(stats_variance)
{
    zval *data;
    double mean;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(data)
        Z_PARAM_DOUBLE(mean)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_variance(values, count, true);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_std_dev(array $data, double $mean) */
PHP_FUNCTION(stats_std_dev)
{
    zval *data;
    double mean;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(data)
        Z_PARAM_DOUBLE(mean)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_std_dev(values, count, true);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_median(array $data) */
PHP_FUNCTION(stats_median)
{
    zval *data;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_median(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_percentile(array $data, double $percentile) */
PHP_FUNCTION(stats_percentile)
{
    zval *data;
    double percentile;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(data)
        Z_PARAM_DOUBLE(percentile)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_percentile(values, count, percentile);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_min(array $data) */
PHP_FUNCTION(stats_min)
{
    zval *data;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_min(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_max(array $data) */
PHP_FUNCTION(stats_max)
{
    zval *data;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    double *values = php_array_to_double(data, &count);
    if (!values) {
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_max(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_correlation(array $x, array $y) */
PHP_FUNCTION(stats_correlation)
{
    zval *x_arr, *y_arr;
    size_t x_count, y_count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(x_arr)
        Z_PARAM_ARRAY(y_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    double *x = php_array_to_double(x_arr, &x_count);
    double *y = php_array_to_double(y_arr, &y_count);
    
    if (!x || !y || x_count != y_count) {
        if (x) efree(x);
        if (y) efree(y);
        RETURN_DOUBLE(0.0);
    }
    
    double result = stats_correlation(x, y, x_count);
    
    efree(x);
    efree(y);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double cross_entropy_loss(array $predictions, array $targets) */
PHP_FUNCTION(cross_entropy_loss)
{
    zval *pred_arr, *target_arr;
    size_t pred_count, target_count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(pred_arr)
        Z_PARAM_ARRAY(target_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    double *predictions = php_array_to_double(pred_arr, &pred_count);
    double *targets = php_array_to_double(target_arr, &target_count);
    
    if (!predictions || !targets || pred_count != target_count) {
        if (predictions) efree(predictions);
        if (targets) efree(targets);
        RETURN_DOUBLE(0.0);
    }
    
    // Use custom implementation to avoid math.h
    double loss = 0.0;
    for (size_t i = 0; i < pred_count; i++) {
        if (targets[i] > 0.0 && predictions[i] > 0.0) {
            loss -= targets[i] * numerical_safe_log(predictions[i], 1e-10);
        }
    }
    
    efree(predictions);
    efree(targets);
    
    RETURN_DOUBLE(loss);
}
/* }}} */

/* {{{ proto double numerical_dot_product(array $a, array $b) */
PHP_FUNCTION(numerical_dot_product)
{
    zval *a_arr, *b_arr;
    size_t a_count, b_count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(a_arr)
        Z_PARAM_ARRAY(b_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    double *a = php_array_to_double(a_arr, &a_count);
    double *b = php_array_to_double(b_arr, &b_count);
    
    if (!a || !b || a_count != b_count) {
        if (a) efree(a);
        if (b) efree(b);
        RETURN_DOUBLE(0.0);
    }
    
    double result = numerical_dot_product(a, b, a_count);
    
    efree(a);
    efree(b);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double numerical_cosine_similarity(array $a, array $b) */
PHP_FUNCTION(numerical_cosine_similarity)
{
    zval *a_arr, *b_arr;
    size_t a_count, b_count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(a_arr)
        Z_PARAM_ARRAY(b_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    double *a = php_array_to_double(a_arr, &a_count);
    double *b = php_array_to_double(b_arr, &b_count);
    
    if (!a || !b || a_count != b_count) {
        if (a) efree(a);
        if (b) efree(b);
        RETURN_DOUBLE(0.0);
    }
    
    double result = numerical_cosine_similarity(a, b, a_count);
    
    efree(a);
    efree(b);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_array, 0)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_double, 0)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_two_arrays, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_two_doubles, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_three_doubles, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
    ZEND_ARG_INFO(0, c)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ algorithms_functions[] */
const zend_function_entry algorithms_functions[] = {
    PHP_FE(numerical_softmax, arginfo_array)
    PHP_FE(numerical_log_softmax, arginfo_array)
    PHP_FE(numerical_safe_log, arginfo_two_doubles)
    PHP_FE(numerical_safe_divide, arginfo_three_doubles)
    PHP_FE(numerical_clip, arginfo_three_doubles)
    PHP_FE(stats_mean, arginfo_array)
    PHP_FE(stats_variance, arginfo_array_double)
    PHP_FE(stats_std_dev, arginfo_array_double)
    PHP_FE(stats_median, arginfo_array)
    PHP_FE(stats_percentile, arginfo_array_double)
    PHP_FE(stats_min, arginfo_array)
    PHP_FE(stats_max, arginfo_array)
    PHP_FE(stats_correlation, arginfo_two_arrays)
    PHP_FE(cross_entropy_loss, arginfo_two_arrays)
    PHP_FE(numerical_dot_product, arginfo_two_arrays)
    PHP_FE(numerical_cosine_similarity, arginfo_two_arrays)
    PHP_FE_END
};
/* }}} */

/* {{{ algorithms_module_entry */
zend_module_entry algorithms_module_entry = {
    STANDARD_MODULE_HEADER,
    "algorithms",
    algorithms_functions,
    PHP_MINIT(algorithms),
    PHP_MSHUTDOWN(algorithms),
    NULL,
    NULL,
    PHP_MINFO(algorithms),
    "1.0.0",
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ALGORITHMS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(algorithms)
#endif