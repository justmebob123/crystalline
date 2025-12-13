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

// Include the algorithms library headers
#include "../../algorithms/include/numerical.h"
#include "../../algorithms/include/statistics.h"
#include "../../algorithms/include/optimizers.h"

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
    php_info_print_table_row(2, "Version", PHP_ALGORITHMS_VERSION);
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto array algo_statistics(array $data)
   Calculate statistics for an array of numbers */
PHP_FUNCTION(algo_statistics)
{
    zval *data_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *data_ht = Z_ARRVAL_P(data_array);
    int count = zend_hash_num_elements(data_ht);
    
    if (count == 0) {
        send_error("Empty data array");
    }
    
    // Convert PHP array to C array
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(data_ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    // Calculate statistics
    double mean = calculate_mean(values, count);
    double variance = calculate_variance(values, count);
    double std_dev = calculate_std_dev(values, count);
    double min_val = values[0];
    double max_val = values[0];
    
    for (int j = 1; j < count; j++) {
        if (values[j] < min_val) min_val = values[j];
        if (values[j] > max_val) max_val = values[j];
    }
    
    efree(values);
    
    // Return statistics as associative array
    array_init(return_value);
    add_assoc_long(return_value, "count", count);
    add_assoc_double(return_value, "mean", mean);
    add_assoc_double(return_value, "variance", variance);
    add_assoc_double(return_value, "std_dev", std_dev);
    add_assoc_double(return_value, "min", min_val);
    add_assoc_double(return_value, "max", max_val);
}
/* }}} */

/* {{{ proto float algo_mean(array $data)
   Calculate mean of an array */
PHP_FUNCTION(algo_mean)
{
    zval *data_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *data_ht = Z_ARRVAL_P(data_array);
    int count = zend_hash_num_elements(data_ht);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(data_ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = calculate_mean(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float algo_variance(array $data)
   Calculate variance of an array */
PHP_FUNCTION(algo_variance)
{
    zval *data_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *data_ht = Z_ARRVAL_P(data_array);
    int count = zend_hash_num_elements(data_ht);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(data_ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = calculate_variance(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float algo_std_dev(array $data)
   Calculate standard deviation of an array */
PHP_FUNCTION(algo_std_dev)
{
    zval *data_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(data_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *data_ht = Z_ARRVAL_P(data_array);
    int count = zend_hash_num_elements(data_ht);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(data_ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = calculate_std_dev(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float algo_dot_product(array $a, array $b)
   Calculate dot product of two vectors */
PHP_FUNCTION(algo_dot_product)
{
    zval *array_a, *array_b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(array_a)
        Z_PARAM_ARRAY(array_b)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *ht_a = Z_ARRVAL_P(array_a);
    HashTable *ht_b = Z_ARRVAL_P(array_b);
    
    int count_a = zend_hash_num_elements(ht_a);
    int count_b = zend_hash_num_elements(ht_b);
    
    if (count_a != count_b) {
        php_error_docref(NULL, E_WARNING, "Arrays must have the same length");
        RETURN_FALSE;
    }
    
    if (count_a == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values_a = (double *)emalloc(count_a * sizeof(double));
    double *values_b = (double *)emalloc(count_b * sizeof(double));
    
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(ht_a, entry) {
        values_a[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    i = 0;
    ZEND_HASH_FOREACH_VAL(ht_b, entry) {
        values_b[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = dot_product(values_a, values_b, count_a);
    
    efree(values_a);
    efree(values_b);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float algo_vector_norm(array $vector)
   Calculate L2 norm of a vector */
PHP_FUNCTION(algo_vector_norm)
{
    zval *vector_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(vector_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *ht = Z_ARRVAL_P(vector_array);
    int count = zend_hash_num_elements(ht);
    
    if (count == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = vector_norm(values, count);
    efree(values);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto array algo_normalize_vector(array $vector)
   Normalize a vector to unit length */
PHP_FUNCTION(algo_normalize_vector)
{
    zval *vector_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(vector_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *ht = Z_ARRVAL_P(vector_array);
    int count = zend_hash_num_elements(ht);
    
    if (count == 0) {
        array_init(return_value);
        return;
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    normalize_vector(values, count);
    
    array_init(return_value);
    for (int j = 0; j < count; j++) {
        add_next_index_double(return_value, values[j]);
    }
    
    efree(values);
}
/* }}} */

/* {{{ proto float algo_cosine_similarity(array $a, array $b)
   Calculate cosine similarity between two vectors */
PHP_FUNCTION(algo_cosine_similarity)
{
    zval *array_a, *array_b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(array_a)
        Z_PARAM_ARRAY(array_b)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *ht_a = Z_ARRVAL_P(array_a);
    HashTable *ht_b = Z_ARRVAL_P(array_b);
    
    int count_a = zend_hash_num_elements(ht_a);
    int count_b = zend_hash_num_elements(ht_b);
    
    if (count_a != count_b) {
        php_error_docref(NULL, E_WARNING, "Arrays must have the same length");
        RETURN_FALSE;
    }
    
    if (count_a == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *values_a = (double *)emalloc(count_a * sizeof(double));
    double *values_b = (double *)emalloc(count_b * sizeof(double));
    
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(ht_a, entry) {
        values_a[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    i = 0;
    ZEND_HASH_FOREACH_VAL(ht_b, entry) {
        values_b[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = cosine_similarity(values_a, values_b, count_a);
    
    efree(values_a);
    efree(values_b);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto array algo_softmax(array $logits)
   Apply softmax function to logits */
PHP_FUNCTION(algo_softmax)
{
    zval *logits_array;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(logits_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *ht = Z_ARRVAL_P(logits_array);
    int count = zend_hash_num_elements(ht);
    
    if (count == 0) {
        array_init(return_value);
        return;
    }
    
    double *values = (double *)emalloc(count * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(ht, entry) {
        values[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    softmax(values, count);
    
    array_init(return_value);
    for (int j = 0; j < count; j++) {
        add_next_index_double(return_value, values[j]);
    }
    
    efree(values);
}
/* }}} */

/* {{{ proto float algo_cross_entropy(array $predictions, array $targets)
   Calculate cross-entropy loss */
PHP_FUNCTION(algo_cross_entropy)
{
    zval *pred_array, *target_array;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(pred_array)
        Z_PARAM_ARRAY(target_array)
    ZEND_PARSE_PARAMETERS_END();
    
    HashTable *pred_ht = Z_ARRVAL_P(pred_array);
    HashTable *target_ht = Z_ARRVAL_P(target_array);
    
    int count_pred = zend_hash_num_elements(pred_ht);
    int count_target = zend_hash_num_elements(target_ht);
    
    if (count_pred != count_target) {
        php_error_docref(NULL, E_WARNING, "Arrays must have the same length");
        RETURN_FALSE;
    }
    
    if (count_pred == 0) {
        RETURN_DOUBLE(0.0);
    }
    
    double *predictions = (double *)emalloc(count_pred * sizeof(double));
    double *targets = (double *)emalloc(count_target * sizeof(double));
    
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(pred_ht, entry) {
        predictions[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    i = 0;
    ZEND_HASH_FOREACH_VAL(target_ht, entry) {
        targets[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    double result = cross_entropy_loss(predictions, targets, count_pred);
    
    efree(predictions);
    efree(targets);
    
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto array algo_gradient_descent(array $initial, callable $gradient_fn, array $options)
   Perform gradient descent optimization */
PHP_FUNCTION(algo_gradient_descent)
{
    zval *initial_array, *gradient_fn, *options_array = NULL;
    
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_ARRAY(initial_array)
        Z_PARAM_ZVAL(gradient_fn)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_OR_NULL(options_array)
    ZEND_PARSE_PARAMETERS_END();
    
    // Get initial values
    HashTable *ht = Z_ARRVAL_P(initial_array);
    int dim = zend_hash_num_elements(ht);
    
    if (dim == 0) {
        array_init(return_value);
        return;
    }
    
    double *x = (double *)emalloc(dim * sizeof(double));
    int i = 0;
    zval *entry;
    
    ZEND_HASH_FOREACH_VAL(ht, entry) {
        x[i++] = zval_get_double(entry);
    } ZEND_HASH_FOREACH_END();
    
    // Get options
    double learning_rate = 0.01;
    int max_iterations = 1000;
    double tolerance = 1e-6;
    
    if (options_array) {
        HashTable *opt_ht = Z_ARRVAL_P(options_array);
        zval *lr = zend_hash_str_find(opt_ht, "learning_rate", sizeof("learning_rate") - 1);
        if (lr) learning_rate = zval_get_double(lr);
        
        zval *max_iter = zend_hash_str_find(opt_ht, "max_iterations", sizeof("max_iterations") - 1);
        if (max_iter) max_iterations = zval_get_long(max_iter);
        
        zval *tol = zend_hash_str_find(opt_ht, "tolerance", sizeof("tolerance") - 1);
        if (tol) tolerance = zval_get_double(tol);
    }
    
    // Simple gradient descent implementation
    for (int iter = 0; iter < max_iterations; iter++) {
        // Call gradient function
        zval params[1];
        zval retval;
        
        array_init(&params[0]);
        for (int j = 0; j < dim; j++) {
            add_next_index_double(&params[0], x[j]);
        }
        
        if (call_user_function(EG(function_table), NULL, gradient_fn, &retval, 1, params) != SUCCESS) {
            zval_ptr_dtor(&params[0]);
            efree(x);
            php_error_docref(NULL, E_WARNING, "Failed to call gradient function");
            RETURN_FALSE;
        }
        
        // Get gradient values
        if (Z_TYPE(retval) != IS_ARRAY) {
            zval_ptr_dtor(&params[0]);
            zval_ptr_dtor(&retval);
            efree(x);
            php_error_docref(NULL, E_WARNING, "Gradient function must return an array");
            RETURN_FALSE;
        }
        
        HashTable *grad_ht = Z_ARRVAL(retval);
        if (zend_hash_num_elements(grad_ht) != dim) {
            zval_ptr_dtor(&params[0]);
            zval_ptr_dtor(&retval);
            efree(x);
            php_error_docref(NULL, E_WARNING, "Gradient dimension mismatch");
            RETURN_FALSE;
        }
        
        // Update x
        double max_grad = 0.0;
        i = 0;
        ZEND_HASH_FOREACH_VAL(grad_ht, entry) {
            double grad = zval_get_double(entry);
            x[i] -= learning_rate * grad;
            if (fabs(grad) > max_grad) max_grad = fabs(grad);
            i++;
        } ZEND_HASH_FOREACH_END();
        
        zval_ptr_dtor(&params[0]);
        zval_ptr_dtor(&retval);
        
        // Check convergence
        if (max_grad < tolerance) {
            break;
        }
    }
    
    // Return optimized values
    array_init(return_value);
    for (int j = 0; j < dim; j++) {
        add_next_index_double(return_value, x[j]);
    }
    
    efree(x);
}
/* }}} */

/* {{{ algorithms_functions[]
 */
const zend_function_entry algorithms_functions[] = {
    PHP_FE(algo_statistics,         NULL)
    PHP_FE(algo_mean,                NULL)
    PHP_FE(algo_variance,            NULL)
    PHP_FE(algo_std_dev,             NULL)
    PHP_FE(algo_dot_product,         NULL)
    PHP_FE(algo_vector_norm,         NULL)
    PHP_FE(algo_normalize_vector,    NULL)
    PHP_FE(algo_cosine_similarity,   NULL)
    PHP_FE(algo_softmax,             NULL)
    PHP_FE(algo_cross_entropy,       NULL)
    PHP_FE(algo_gradient_descent,    NULL)
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
    PHP_ALGORITHMS_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ALGORITHMS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(algorithms)
#endif