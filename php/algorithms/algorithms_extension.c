/*
  Algorithms PHP Extension
  
  Provides PHP bindings for the Algorithms Library
  Focus: Numerical algorithms, optimization, statistics
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_algorithms.h"
#include <math.h>

// Include the algorithms library headers
#include "../../algorithms/include/numerical.h"
#include "../../algorithms/include/statistics.h"
#include "../../algorithms/include/loss_functions.h"

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(algorithms)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(algorithms)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(algorithms)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Algorithms Support", "enabled");
    php_info_print_table_row(2, "Version", "1.0.0");
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto array numerical_softmax(array $data)
   Apply softmax function to array */
PHP_FUNCTION(numerical_softmax)
{
    zval *data;
    HashTable *data_hash;
    zval *entry;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    data_hash = Z_ARRVAL_P(data);
    count = zend_hash_num_elements(data_hash);
    
    if (count == 0) {
        RETURN_EMPTY_ARRAY();
    }
    
    double *input = (double *)emalloc(count * sizeof(double));
    double *output = (double *)emalloc(count * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(data_hash, entry) {
        input[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    numerical_softmax(input, output, count);
    
    array_init(return_value);
    for (i = 0; i < count; i++) {
        add_next_index_double(return_value, output[i]);
    }
    
    efree(input);
    efree(output);
}
/* }}} */

/* {{{ proto double cross_entropy_loss(array $predictions, array $targets)
   Calculate cross entropy loss */
PHP_FUNCTION(cross_entropy_loss)
{
    zval *pred_arr, *target_arr;
    HashTable *pred_hash, *target_hash;
    zval *entry;
    size_t pred_count, target_count;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(pred_arr)
        Z_PARAM_ARRAY(target_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    pred_hash = Z_ARRVAL_P(pred_arr);
    target_hash = Z_ARRVAL_P(target_arr);
    pred_count = zend_hash_num_elements(pred_hash);
    target_count = zend_hash_num_elements(target_hash);
    
    if (pred_count != target_count) {
        php_error_docref(NULL, E_WARNING, "Arrays must have same length");
        RETURN_FALSE;
    }
    
    if (pred_count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *predictions = (double *)emalloc(pred_count * sizeof(double));
    double *targets = (double *)emalloc(target_count * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(pred_hash, entry) {
        predictions[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    i = 0;
    ZEND_HASH_FOREACH_VAL(target_hash, entry) {
        targets[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double loss = 0.0;
    for (i = 0; i < pred_count; i++) {
        if (targets[i] > 0.0 && predictions[i] > 0.0) {
            loss -= targets[i] * log(predictions[i]);
        }
    }
    
    efree(predictions);
    efree(targets);
    
    RETURN_DOUBLE(loss);
}
/* }}} */

/* {{{ proto double stats_mean(array $data)
   Calculate mean of array */
PHP_FUNCTION(stats_mean)
{
    zval *data;
    HashTable *data_hash;
    zval *entry;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data)
    ZEND_PARSE_PARAMETERS_END();
    
    data_hash = Z_ARRVAL_P(data);
    count = zend_hash_num_elements(data_hash);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(data_hash, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = stats_mean(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_variance(array $data, bool $sample_variance)
   Calculate variance of array */
PHP_FUNCTION(stats_variance)
{
    zval *data;
    zend_bool sample_variance = 1;
    HashTable *data_hash;
    zval *entry;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ARRAY(data)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(sample_variance)
    ZEND_PARSE_PARAMETERS_END();
    
    data_hash = Z_ARRVAL_P(data);
    count = zend_hash_num_elements(data_hash);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(data_hash, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = stats_variance(values, count, sample_variance);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double stats_std_dev(array $data, bool $sample_variance)
   Calculate standard deviation of array */
PHP_FUNCTION(stats_std_dev)
{
    zval *data;
    zend_bool sample_variance = 1;
    HashTable *data_hash;
    zval *entry;
    size_t count;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ARRAY(data)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(sample_variance)
    ZEND_PARSE_PARAMETERS_END();
    
    data_hash = Z_ARRVAL_P(data);
    count = zend_hash_num_elements(data_hash);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(data_hash, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = stats_std_dev(values, count, sample_variance);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double numerical_dot_product(array $a, array $b)
   Calculate dot product of two arrays */
PHP_FUNCTION(numerical_dot_product)
{
    zval *arr_a, *arr_b;
    HashTable *hash_a, *hash_b;
    zval *entry;
    size_t count_a, count_b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(arr_a)
        Z_PARAM_ARRAY(arr_b)
    ZEND_PARSE_PARAMETERS_END();
    
    hash_a = Z_ARRVAL_P(arr_a);
    hash_b = Z_ARRVAL_P(arr_b);
    count_a = zend_hash_num_elements(hash_a);
    count_b = zend_hash_num_elements(hash_b);
    
    if (count_a != count_b) {
        php_error_docref(NULL, E_WARNING, "Arrays must have same length");
        RETURN_FALSE;
    }
    
    if (count_a == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values_a = (double *)emalloc(count_a * sizeof(double));
    double *values_b = (double *)emalloc(count_b * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(hash_a, entry) {
        values_a[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    i = 0;
    ZEND_HASH_FOREACH_VAL(hash_b, entry) {
        values_b[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = numerical_dot_product(values_a, values_b, count_a);
    
    efree(values_a);
    efree(values_b);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto double numerical_cosine_similarity(array $a, array $b)
   Calculate cosine similarity between two arrays */
PHP_FUNCTION(numerical_cosine_similarity)
{
    zval *arr_a, *arr_b;
    HashTable *hash_a, *hash_b;
    zval *entry;
    size_t count_a, count_b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(arr_a)
        Z_PARAM_ARRAY(arr_b)
    ZEND_PARSE_PARAMETERS_END();
    
    hash_a = Z_ARRVAL_P(arr_a);
    hash_b = Z_ARRVAL_P(arr_b);
    count_a = zend_hash_num_elements(hash_a);
    count_b = zend_hash_num_elements(hash_b);
    
    if (count_a != count_b) {
        php_error_docref(NULL, E_WARNING, "Arrays must have same length");
        RETURN_FALSE;
    }
    
    if (count_a == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values_a = (double *)emalloc(count_a * sizeof(double));
    double *values_b = (double *)emalloc(count_b * sizeof(double));
    size_t i = 0;
    
    ZEND_HASH_FOREACH_VAL(hash_a, entry) {
        values_a[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    i = 0;
    ZEND_HASH_FOREACH_VAL(hash_b, entry) {
        values_b[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = numerical_cosine_similarity(values_a, values_b, count_a);
    
    efree(values_a);
    efree(values_b);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_numerical_softmax, 0)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_cross_entropy_loss, 0)
    ZEND_ARG_INFO(0, predictions)
    ZEND_ARG_INFO(0, targets)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stats_mean, 0)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stats_variance, 0)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, sample_variance)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stats_std_dev, 0)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, sample_variance)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_numerical_dot_product, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_numerical_cosine_similarity, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ algorithms_functions[]
 */
const zend_function_entry algorithms_functions[] = {
    PHP_FE(numerical_softmax, arginfo_numerical_softmax)
    PHP_FE(cross_entropy_loss, arginfo_cross_entropy_loss)
    PHP_FE(stats_mean, arginfo_stats_mean)
    PHP_FE(stats_variance, arginfo_stats_variance)
    PHP_FE(stats_std_dev, arginfo_stats_std_dev)
    PHP_FE(numerical_dot_product, arginfo_numerical_dot_product)
    PHP_FE(numerical_cosine_similarity, arginfo_numerical_cosine_similarity)
    PHP_FE_END
};
/* }}} */

/* {{{ algorithms_module_entry
 */
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