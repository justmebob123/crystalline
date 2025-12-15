# Convergence Detection Component - Detailed Specification

## Overview

Convergence detection is a **critical missing component** that should be used by ALL phases of the geometric recovery pipeline. It provides consistent, configurable stopping criteria and prevents infinite loops.

## Why It's Critical

### Current Problems

1. **No Unified Stopping Criteria** - Each component implements its own convergence logic
2. **Hardcoded Iterations** - Many components use fixed iteration counts
3. **No Early Stopping** - Can't stop when convergence is achieved early
4. **Inconsistent Thresholds** - Different thresholds across components
5. **No Convergence History** - Can't analyze convergence patterns

### Benefits of Standalone Component

1. **Consistent Behavior** - All phases use same convergence logic
2. **Configurable** - Easy to adjust thresholds per phase
3. **Reusable** - Single implementation used everywhere
4. **Analyzable** - Convergence history for debugging
5. **Adaptive** - Can adjust thresholds based on performance

## Mathematical Foundation

### Convergence Criteria

A system converges when:

1. **Absolute Error Criterion:**
   ```
   |error(t) - error(t-1)| < ε_abs
   ```

2. **Relative Error Criterion:**
   ```
   |error(t) - error(t-1)| / |error(t-1)| < ε_rel
   ```

3. **Moving Average Criterion:**
   ```
   variance(error[t-w:t]) < ε_var
   ```
   where w = window size

4. **Gradient Criterion:**
   ```
   |d(error)/dt| < ε_grad
   ```

5. **Stability Criterion:**
   ```
   error remains below threshold for n consecutive iterations
   ```

### Multi-Metric Convergence

For robust convergence, combine multiple metrics:

```
converged = (absolute_converged AND relative_converged) OR
            (moving_avg_converged AND stable_for_n_iterations)
```

## API Design

### Core Structures

```c
/**
 * Convergence criteria configuration
 */
typedef struct {
    // Absolute error threshold
    double abs_threshold;
    
    // Relative error threshold (percentage)
    double rel_threshold;
    
    // Variance threshold for moving average
    double var_threshold;
    
    // Gradient threshold
    double grad_threshold;
    
    // Stability requirements
    uint32_t stability_window;      // Must be stable for this many iterations
    
    // Moving average window size
    uint32_t moving_avg_window;
    
    // Maximum iterations (safety limit)
    uint32_t max_iterations;
    
    // Minimum iterations (prevent premature convergence)
    uint32_t min_iterations;
    
} ConvergenceCriteria;

/**
 * Convergence detector state
 */
typedef struct {
    // Configuration
    ConvergenceCriteria criteria;
    
    // State
    uint32_t current_iteration;
    double current_error;
    double previous_error;
    
    // History
    double* error_history;          // [max_iterations]
    uint32_t history_size;
    uint32_t history_capacity;
    
    // Convergence tracking
    bool has_converged;
    uint32_t convergence_iteration; // When it converged
    uint32_t iterations_stable;     // How long stable
    
    // Multi-metric tracking
    bool abs_converged;
    bool rel_converged;
    bool var_converged;
    bool grad_converged;
    bool stable_converged;
    
    // Statistics
    double min_error;
    double max_error;
    double avg_error;
    double convergence_rate;        // How fast it converged
    
} ConvergenceDetector;

/**
 * Convergence result
 */
typedef struct {
    bool converged;
    uint32_t iterations;
    double final_error;
    double convergence_rate;
    
    // Which criteria were met
    bool abs_criterion_met;
    bool rel_criterion_met;
    bool var_criterion_met;
    bool grad_criterion_met;
    bool stable_criterion_met;
    
    // Confidence in convergence
    double confidence;
    
} ConvergenceResult;
```

### Core API Functions

```c
/**
 * Create convergence detector with default criteria
 */
ConvergenceDetector* convergence_detector_create(
    uint32_t max_iterations
);

/**
 * Create convergence detector with custom criteria
 */
ConvergenceDetector* convergence_detector_create_custom(
    ConvergenceCriteria* criteria
);

/**
 * Update detector with new error value
 * Returns true if converged
 */
bool convergence_detector_update(
    ConvergenceDetector* detector,
    double error
);

/**
 * Check if converged (without updating)
 */
bool convergence_detector_check(
    const ConvergenceDetector* detector
);

/**
 * Get convergence result
 */
ConvergenceResult convergence_detector_get_result(
    const ConvergenceDetector* detector
);

/**
 * Reset detector for new run
 */
void convergence_detector_reset(
    ConvergenceDetector* detector
);

/**
 * Free detector
 */
void convergence_detector_free(
    ConvergenceDetector* detector
);

/**
 * Get convergence confidence (0.0 to 1.0)
 */
double convergence_detector_get_confidence(
    const ConvergenceDetector* detector
);

/**
 * Get error history for analysis
 */
const double* convergence_detector_get_history(
    const ConvergenceDetector* detector,
    uint32_t* size_out
);
```

### Advanced API Functions

```c
/**
 * Multi-metric convergence detection
 * Checks convergence across multiple metrics simultaneously
 */
bool convergence_detector_update_multi(
    ConvergenceDetector* detector,
    double* metrics,
    uint32_t num_metrics
);

/**
 * Adaptive threshold adjustment
 * Automatically adjusts thresholds based on convergence behavior
 */
void convergence_detector_adapt_thresholds(
    ConvergenceDetector* detector,
    double adaptation_rate
);

/**
 * Get convergence diagnostics
 */
typedef struct {
    double convergence_rate;
    double oscillation_amplitude;
    double trend;                   // Positive = improving, negative = degrading
    bool is_oscillating;
    bool is_diverging;
    uint32_t estimated_iterations_remaining;
} ConvergenceDiagnostics;

ConvergenceDiagnostics convergence_detector_get_diagnostics(
    const ConvergenceDetector* detector
);
```

## Implementation Details

### Absolute Error Convergence

```c
static bool check_absolute_convergence(
    ConvergenceDetector* detector
) {
    if (detector->current_iteration < detector->criteria.min_iterations) {
        return false;
    }
    
    double abs_change = fabs(detector->current_error - detector->previous_error);
    return abs_change < detector->criteria.abs_threshold;
}
```

### Relative Error Convergence

```c
static bool check_relative_convergence(
    ConvergenceDetector* detector
) {
    if (detector->current_iteration < detector->criteria.min_iterations) {
        return false;
    }
    
    if (fabs(detector->previous_error) < 1e-10) {
        return false;  // Avoid division by zero
    }
    
    double rel_change = fabs(
        (detector->current_error - detector->previous_error) / 
        detector->previous_error
    );
    
    return rel_change < detector->criteria.rel_threshold;
}
```

### Moving Average Variance Convergence

```c
static bool check_variance_convergence(
    ConvergenceDetector* detector
) {
    uint32_t window = detector->criteria.moving_avg_window;
    
    if (detector->history_size < window) {
        return false;
    }
    
    // Compute variance over window
    double sum = 0.0;
    double sum_sq = 0.0;
    
    for (uint32_t i = detector->history_size - window; 
         i < detector->history_size; i++) {
        double val = detector->error_history[i];
        sum += val;
        sum_sq += val * val;
    }
    
    double mean = sum / window;
    double variance = (sum_sq / window) - (mean * mean);
    
    return variance < detector->criteria.var_threshold;
}
```

### Gradient Convergence

```c
static bool check_gradient_convergence(
    ConvergenceDetector* detector
) {
    if (detector->history_size < 2) {
        return false;
    }
    
    // Compute gradient (derivative)
    double gradient = detector->current_error - detector->previous_error;
    
    return fabs(gradient) < detector->criteria.grad_threshold;
}
```

### Stability Convergence

```c
static bool check_stability_convergence(
    ConvergenceDetector* detector
) {
    uint32_t window = detector->criteria.stability_window;
    
    if (detector->iterations_stable >= window) {
        return true;
    }
    
    // Check if error is below threshold
    if (detector->current_error < detector->criteria.abs_threshold) {
        detector->iterations_stable++;
    } else {
        detector->iterations_stable = 0;
    }
    
    return detector->iterations_stable >= window;
}
```

### Main Update Function

```c
bool convergence_detector_update(
    ConvergenceDetector* detector,
    double error
) {
    // Update state
    detector->previous_error = detector->current_error;
    detector->current_error = error;
    detector->current_iteration++;
    
    // Add to history
    if (detector->history_size < detector->history_capacity) {
        detector->error_history[detector->history_size++] = error;
    }
    
    // Update statistics
    if (error < detector->min_error) detector->min_error = error;
    if (error > detector->max_error) detector->max_error = error;
    
    // Check maximum iterations
    if (detector->current_iteration >= detector->criteria.max_iterations) {
        detector->has_converged = false;
        return false;
    }
    
    // Check minimum iterations
    if (detector->current_iteration < detector->criteria.min_iterations) {
        return false;
    }
    
    // Check all convergence criteria
    detector->abs_converged = check_absolute_convergence(detector);
    detector->rel_converged = check_relative_convergence(detector);
    detector->var_converged = check_variance_convergence(detector);
    detector->grad_converged = check_gradient_convergence(detector);
    detector->stable_converged = check_stability_convergence(detector);
    
    // Combined convergence decision
    bool converged = (
        (detector->abs_converged && detector->rel_converged) ||
        (detector->var_converged && detector->stable_converged)
    );
    
    if (converged && !detector->has_converged) {
        detector->has_converged = true;
        detector->convergence_iteration = detector->current_iteration;
        
        // Compute convergence rate
        detector->convergence_rate = 
            (detector->error_history[0] - detector->current_error) / 
            detector->current_iteration;
    }
    
    return detector->has_converged;
}
```

## Usage Examples

### Example 1: Basic Usage

```c
// Create detector
ConvergenceDetector* detector = convergence_detector_create(1000);

// Training loop
for (int iter = 0; iter < 1000; iter++) {
    // Compute error
    double error = compute_training_error(model, data);
    
    // Update detector
    bool converged = convergence_detector_update(detector, error);
    
    if (converged) {
        printf("Converged at iteration %d\n", iter);
        break;
    }
    
    // Continue training
    train_step(model, data);
}

// Get result
ConvergenceResult result = convergence_detector_get_result(detector);
printf("Final error: %.6f\n", result.final_error);
printf("Convergence rate: %.6f\n", result.convergence_rate);
printf("Confidence: %.2f%%\n", result.confidence * 100.0);

// Cleanup
convergence_detector_free(detector);
```

### Example 2: Custom Criteria

```c
// Create custom criteria
ConvergenceCriteria criteria = {
    .abs_threshold = 1e-6,
    .rel_threshold = 1e-4,
    .var_threshold = 1e-8,
    .grad_threshold = 1e-5,
    .stability_window = 10,
    .moving_avg_window = 20,
    .max_iterations = 5000,
    .min_iterations = 100
};

ConvergenceDetector* detector = convergence_detector_create_custom(&criteria);

// Use detector...
```

### Example 3: Multi-Metric Convergence

```c
ConvergenceDetector* detector = convergence_detector_create(1000);

for (int iter = 0; iter < 1000; iter++) {
    // Compute multiple metrics
    double metrics[3] = {
        compute_training_error(model, data),
        compute_validation_error(model, val_data),
        compute_regularization_term(model)
    };
    
    // Update with multiple metrics
    bool converged = convergence_detector_update_multi(
        detector, metrics, 3
    );
    
    if (converged) {
        printf("All metrics converged at iteration %d\n", iter);
        break;
    }
    
    train_step(model, data);
}
```

### Example 4: Adaptive Thresholds

```c
ConvergenceDetector* detector = convergence_detector_create(1000);

for (int iter = 0; iter < 1000; iter++) {
    double error = compute_error(model, data);
    
    // Update detector
    convergence_detector_update(detector, error);
    
    // Adapt thresholds every 100 iterations
    if (iter % 100 == 0) {
        convergence_detector_adapt_thresholds(detector, 0.1);
    }
    
    // Check convergence
    if (convergence_detector_check(detector)) {
        break;
    }
    
    train_step(model, data);
}
```

## Integration with Existing Components

### tetration_attractors.c

```c
// Before
int tetration_system_compute(TetrationSystem* sys) {
    for (int iter = 0; iter < 1000; iter++) {  // Hardcoded
        compute_attractors(sys);
        if (check_convergence_internal(sys)) {  // Custom logic
            break;
        }
    }
    return 0;
}

// After
int tetration_system_compute(TetrationSystem* sys) {
    ConvergenceDetector* detector = convergence_detector_create(1000);
    
    for (int iter = 0; iter < 1000; iter++) {
        compute_attractors(sys);
        
        double error = compute_attractor_error(sys);
        if (convergence_detector_update(detector, error)) {
            break;
        }
    }
    
    sys->convergence_result = convergence_detector_get_result(detector);
    convergence_detector_free(detector);
    
    return 0;
}
```

### micro_model.c

```c
// Before
int micro_model_train(MicroModel* model, TrainingSample* samples, int count) {
    for (int epoch = 0; epoch < 100; epoch++) {  // Hardcoded
        train_epoch(model, samples, count);
        // No convergence check
    }
    return 0;
}

// After
int micro_model_train(MicroModel* model, TrainingSample* samples, int count) {
    ConvergenceDetector* detector = convergence_detector_create(1000);
    
    for (int epoch = 0; epoch < 1000; epoch++) {
        double error = train_epoch(model, samples, count);
        
        if (convergence_detector_update(detector, error)) {
            printf("Training converged at epoch %d\n", epoch);
            break;
        }
    }
    
    model->convergence_result = convergence_detector_get_result(detector);
    convergence_detector_free(detector);
    
    return 0;
}
```

## Testing Strategy

### Unit Tests

1. **Test Absolute Convergence**
   - Error decreases below threshold
   - Converges correctly

2. **Test Relative Convergence**
   - Relative change below threshold
   - Handles zero previous error

3. **Test Variance Convergence**
   - Variance decreases over window
   - Requires sufficient history

4. **Test Gradient Convergence**
   - Gradient approaches zero
   - Detects flat regions

5. **Test Stability Convergence**
   - Remains stable for window
   - Resets on instability

6. **Test Multi-Metric Convergence**
   - All metrics must converge
   - Handles different scales

### Integration Tests

1. **Test with tetration_attractors.c**
2. **Test with micro_model.c**
3. **Test with recursive_recovery.c**
4. **Test with all phases**

### Performance Tests

1. **Memory usage** - Should be O(max_iterations)
2. **CPU usage** - Should be O(1) per update
3. **Convergence speed** - Should detect quickly

## File Structure

```
algorithms/
├── include/
│   └── geometric_recovery/
│       └── convergence_detection.h
└── src/
    └── geometric_recovery/
        └── convergence_detection.c
```

## Implementation Priority

**Priority: CRITICAL - Implement Immediately**

This component is foundational and should be implemented before expanding the orchestrator to 10 phases.

## Next Steps

1. Implement convergence_detection.c
2. Add to Makefile
3. Write unit tests
4. Integrate with existing components
5. Update orchestrator to use convergence detection
6. Document usage patterns