/**
 * @file loss_functions.h
 * @brief Mathematical loss functions for optimization problems
 * 
 * This library provides pure mathematical implementations of common loss functions
 * used in optimization, machine learning, and statistical modeling. All functions
 * are implemented using the crystalline library for arbitrary precision arithmetic.
 * 
 * These are MATHEMATICAL ALGORITHMS, not AI-specific code. They can be used for:
 * - Neural network training
 * - Statistical regression
 * - Optimization problems
 * - Physics simulations
 * - Financial modeling
 * - Any domain requiring loss/error computation
 */

#ifndef LOSS_FUNCTIONS_H
#define LOSS_FUNCTIONS_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Loss function types
 */
typedef enum {
    LOSS_CROSS_ENTROPY,      /**< Cross-entropy loss for classification */
    LOSS_MSE,                /**< Mean Squared Error for regression */
    LOSS_MAE,                /**< Mean Absolute Error for robust regression */
    LOSS_HUBER,              /**< Huber loss (smooth MAE) */
    LOSS_KL_DIVERGENCE,      /**< Kullback-Leibler divergence */
    LOSS_BINARY_CROSS_ENTROPY /**< Binary cross-entropy for binary classification */
} LossFunctionType;

/**
 * @brief Reduction strategy for loss computation
 */
typedef enum {
    LOSS_REDUCTION_MEAN,     /**< Average loss over all samples */
    LOSS_REDUCTION_SUM,      /**< Sum of losses */
    LOSS_REDUCTION_NONE      /**< No reduction, return per-sample losses */
} LossReductionType;

/**
 * @brief Loss computation configuration
 */
typedef struct {
    LossFunctionType type;           /**< Type of loss function */
    LossReductionType reduction;     /**< How to reduce losses */
    double label_smoothing;          /**< Label smoothing factor (0.0 = none) */
    double huber_delta;              /**< Delta parameter for Huber loss */
    bool clip_gradients;             /**< Whether to clip gradients */
    double gradient_clip_value;      /**< Value for gradient clipping */
    double gradient_clip_norm;       /**< Norm for gradient clipping */
} LossConfig;

/**
 * @brief Loss computation result
 */
typedef struct {
    double loss_value;               /**< Computed loss value */
    double* per_sample_losses;       /**< Per-sample losses (if reduction=NONE) */
    size_t num_samples;              /**< Number of samples */
    bool has_nan;                    /**< Whether NaN was detected */
    bool has_inf;                    /**< Whether Inf was detected */
} LossResult;

/**
 * @brief Gradient information
 */
typedef struct {
    double* gradients;               /**< Gradient values */
    size_t size;                     /**< Number of gradient values */
    double gradient_norm;            /**< L2 norm of gradients */
    bool clipped;                    /**< Whether gradients were clipped */
} GradientInfo;

/* ============================================================================
 * Loss Computation Functions
 * ============================================================================ */

/**
 * @brief Create a loss configuration with default values
 * @param type Loss function type
 * @return Loss configuration
 */
LossConfig loss_config_create(LossFunctionType type);

/**
 * @brief Compute cross-entropy loss
 * @param predictions Predicted probabilities [batch_size x num_classes]
 * @param targets Target class indices or one-hot vectors
 * @param batch_size Number of samples
 * @param num_classes Number of classes
 * @param config Loss configuration
 * @return Loss result
 */
LossResult loss_cross_entropy(
    const double* predictions,
    const double* targets,
    size_t batch_size,
    size_t num_classes,
    const LossConfig* config
);

/**
 * @brief Compute mean squared error loss
 * @param predictions Predicted values
 * @param targets Target values
 * @param size Number of values
 * @param config Loss configuration
 * @return Loss result
 */
LossResult loss_mse(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/**
 * @brief Compute mean absolute error loss
 * @param predictions Predicted values
 * @param targets Target values
 * @param size Number of values
 * @param config Loss configuration
 * @return Loss result
 */
LossResult loss_mae(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/**
 * @brief Compute Huber loss (smooth MAE)
 * @param predictions Predicted values
 * @param targets Target values
 * @param size Number of values
 * @param config Loss configuration (uses huber_delta parameter)
 * @return Loss result
 */
LossResult loss_huber(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/**
 * @brief Compute KL divergence
 * @param predictions Predicted probability distribution
 * @param targets Target probability distribution
 * @param size Number of values
 * @param config Loss configuration
 * @return Loss result
 */
LossResult loss_kl_divergence(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/**
 * @brief Compute binary cross-entropy loss
 * @param predictions Predicted probabilities [0, 1]
 * @param targets Target binary values {0, 1}
 * @param size Number of values
 * @param config Loss configuration
 * @return Loss result
 */
LossResult loss_binary_cross_entropy(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/* ============================================================================
 * Gradient Computation Functions
 * ============================================================================ */

/**
 * @brief Compute gradients for cross-entropy loss
 * @param predictions Predicted probabilities
 * @param targets Target values
 * @param batch_size Number of samples
 * @param num_classes Number of classes
 * @param config Loss configuration
 * @return Gradient information
 */
GradientInfo loss_cross_entropy_gradient(
    const double* predictions,
    const double* targets,
    size_t batch_size,
    size_t num_classes,
    const LossConfig* config
);

/**
 * @brief Compute gradients for MSE loss
 * @param predictions Predicted values
 * @param targets Target values
 * @param size Number of values
 * @param config Loss configuration
 * @return Gradient information
 */
GradientInfo loss_mse_gradient(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/**
 * @brief Compute gradients for MAE loss
 * @param predictions Predicted values
 * @param targets Target values
 * @param size Number of values
 * @param config Loss configuration
 * @return Gradient information
 */
GradientInfo loss_mae_gradient(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/**
 * @brief Compute gradients for Huber loss
 * @param predictions Predicted values
 * @param targets Target values
 * @param size Number of values
 * @param config Loss configuration
 * @return Gradient information
 */
GradientInfo loss_huber_gradient(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Apply label smoothing to target labels
 * @param targets Target labels (one-hot or class indices)
 * @param num_samples Number of samples
 * @param num_classes Number of classes
 * @param smoothing Smoothing factor (0.0 = no smoothing)
 * @return Smoothed labels (caller must free)
 */
double* loss_apply_label_smoothing(
    const double* targets,
    size_t num_samples,
    size_t num_classes,
    double smoothing
);

/**
 * @brief Clip gradients by value
 * @param gradients Gradient values (modified in place)
 * @param size Number of gradients
 * @param clip_value Maximum absolute value
 * @return Whether any gradients were clipped
 */
bool loss_clip_gradients_by_value(
    double* gradients,
    size_t size,
    double clip_value
);

/**
 * @brief Clip gradients by norm
 * @param gradients Gradient values (modified in place)
 * @param size Number of gradients
 * @param max_norm Maximum L2 norm
 * @return Whether gradients were clipped
 */
bool loss_clip_gradients_by_norm(
    double* gradients,
    size_t size,
    double max_norm
);

/**
 * @brief Compute L2 norm of gradients
 * @param gradients Gradient values
 * @param size Number of gradients
 * @return L2 norm
 */
double loss_gradient_norm(const double* gradients, size_t size);

/**
 * @brief Check for NaN or Inf in array
 * @param values Array to check
 * @param size Number of values
 * @param has_nan Output: whether NaN was found
 * @param has_inf Output: whether Inf was found
 */
void loss_check_numerical_issues(
    const double* values,
    size_t size,
    bool* has_nan,
    bool* has_inf
);

/**
 * @brief Free loss result resources
 * @param result Loss result to free
 */
void loss_result_free(LossResult* result);

/**
 * @brief Free gradient info resources
 * @param info Gradient info to free
 */
void gradient_info_free(GradientInfo* info);

#endif /* LOSS_FUNCTIONS_H */