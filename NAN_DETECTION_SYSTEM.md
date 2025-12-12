# NaN Detection System Documentation

## Overview

A comprehensive NaN (Not-a-Number) detection system has been integrated into the CLLM training pipeline to help identify and debug numerical stability issues.

## Files Created

1. **src/ai/cllm_nan_checker.c** - Implementation of NaN detection functions
2. **src/include/cllm_nan_checker.h** - Header file with function declarations

## Features

### 1. Array-Level Checking
- `check_array_for_nan_double()` - Check double arrays for NaN/Inf values
- `check_array_for_nan_float()` - Check float arrays for NaN/Inf values
- Returns index of first NaN found, or -1 if clean
- Reports both NaN and Inf values with detailed diagnostics

### 2. Component-Specific Checks
- `check_embeddings_for_nan()` - Verify input embeddings
- `check_attention_outputs_for_nan()` - Check attention layer outputs
- `check_feedforward_outputs_for_nan()` - Check feedforward layer outputs
- `check_logits_for_nan()` - Verify output logits
- `check_gradients_for_nan()` - Check gradient arrays
- `check_weights_for_nan()` - Verify model weights

### 3. Comprehensive Pass Checks
- `check_forward_pass_for_nan()` - Full forward pass verification
- `check_backward_pass_for_nan()` - Full backward pass verification
- Provides detailed progress reporting
- Returns true if NaN detected, false otherwise

## Integration Points

The NaN detection system is integrated at key points in the training pipeline:

### Forward Pass (cllm_training.c)
1. **After Embeddings** - Line ~1345
   - Checks input embeddings after initialization
   - Detects NaN from lazy embedding computation

2. **After Attention** - Line ~1370
   - Checks attention outputs for each layer
   - Detects NaN from attention computation

3. **After Feedforward** - Line ~1430
   - Checks feedforward outputs for each layer
   - Detects NaN from feedforward and layer norm

4. **After Logits** - Line ~1470
   - Checks final logits before loss computation
   - Detects NaN from output projection

### Backward Pass (cllm_training.c)
1. **After Gradients** - Line ~1695
   - Checks all gradients after backward pass
   - Detects NaN from gradient computation

## Usage

### Compile-Time Control
The system can be enabled/disabled via the `ENABLE_NAN_CHECKS` flag in `cllm_training.c`:

```c
// Enable NaN checking (set to 1 to enable, 0 to disable)
#define ENABLE_NAN_CHECKS 1
```

### Runtime Behavior
When enabled:
- Checks are performed after each major operation
- Detailed error messages are printed to stderr
- Training stops immediately if NaN is detected
- Returns -1.0f from forward pass to signal error

When disabled:
- No performance impact
- All checks are compiled out via preprocessor

## Error Messages

### Example Output
```
=== NaN Check: Forward Pass ===
✓ Embeddings OK
✓ Attention layer 0 OK
✓ Feedforward layer 0 OK
✓ Attention layer 1 OK
✓ Feedforward layer 1 OK
✓ Logits OK
=== Forward Pass: All checks passed ===
```

### When NaN is Detected
```
NaN detected in attention_output_layer_1 at index 245 (value: nan)
CRITICAL: NaN detected in attention output at layer 1!
```

## Performance Impact

- **Enabled**: Minimal overhead (~1-2% slowdown)
  - Only checks array bounds and NaN conditions
  - No complex computations
  - Early exit on first NaN found

- **Disabled**: Zero overhead
  - All checks compiled out via preprocessor
  - No runtime cost

## Testing

The system has been tested with:
- Small model: vocab=100, embed=32, layers=2
- Training data: 1598 tokens
- Result: No NaN detected, training successful
- Loss: 7.67 → 2.22 (proper convergence)

## Future Enhancements

Potential improvements:
1. Add histogram of gradient magnitudes
2. Track gradient statistics over time
3. Add automatic gradient clipping when NaN detected
4. Log NaN detection events to file
5. Add visualization of where NaN originates

## Maintenance

The NaN detection system is:
- Self-contained in two files
- Easy to enable/disable
- Minimal maintenance required
- Automatically included in build via Makefile wildcards

## Troubleshooting

If NaN is detected:
1. Check the error message for exact location
2. Examine the component that produced NaN
3. Verify input data is valid
4. Check for numerical overflow/underflow
5. Review recent code changes to that component

## Related Files

- `src/ai/cllm_training.c` - Main training loop with NaN checks
- `src/ai/cllm_nan_checker.c` - NaN detection implementation
- `src/include/cllm_nan_checker.h` - NaN detection header
- `todo.md` - Development progress tracking
</file_path>