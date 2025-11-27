#include <stdio.h>
#include <stdlib.h>
#include "ai/cllm_loss.h"
#include "ai/cllm_batch.h"

int main() {
    printf("Testing loss functions...\n\n");
    
    // Test 1: Create tensor
    printf("Test 1: Creating tensor...\n");
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    if (!predictions) {
        printf("FAILED: Could not create predictions tensor\n");
        return 1;
    }
    printf("SUCCESS: Tensor created (batch_size=2, num_classes=3)\n");
    printf("  shape: [%u, %u]\n", predictions->shape[0], predictions->shape[1]);
    printf("  total_size: %zu\n", predictions->total_size);
    
    // Test 2: Set values
    printf("\nTest 2: Setting tensor values...\n");
    tensor_set(predictions, (uint32_t[]){0, 0}, 2.0f);
    tensor_set(predictions, (uint32_t[]){0, 1}, 1.0f);
    tensor_set(predictions, (uint32_t[]){0, 2}, 0.1f);
    tensor_set(predictions, (uint32_t[]){1, 0}, 0.5f);
    tensor_set(predictions, (uint32_t[]){1, 1}, 2.5f);
    tensor_set(predictions, (uint32_t[]){1, 2}, 0.2f);
    printf("SUCCESS: Values set\n");
    
    // Verify values
    printf("  predictions[0,0] = %f\n", tensor_get(predictions, (uint32_t[]){0, 0}));
    printf("  predictions[0,1] = %f\n", tensor_get(predictions, (uint32_t[]){0, 1}));
    printf("  predictions[1,1] = %f\n", tensor_get(predictions, (uint32_t[]){1, 1}));
    
    // Test 3: Create targets
    printf("\nTest 3: Creating targets tensor...\n");
    Tensor* targets = tensor_create((uint32_t[]){2}, 1);
    if (!targets) {
        printf("FAILED: Could not create targets tensor\n");
        tensor_free(predictions);
        return 1;
    }
    tensor_set(targets, (uint32_t[]){0}, 0.0f);
    tensor_set(targets, (uint32_t[]){1}, 1.0f);
    printf("SUCCESS: Targets created\n");
    printf("  targets[0] = %f\n", tensor_get(targets, (uint32_t[]){0}));
    printf("  targets[1] = %f\n", tensor_get(targets, (uint32_t[]){1}));
    
    // Test 4: Compute cross-entropy loss
    printf("\nTest 4: Computing cross-entropy loss...\n");
    float loss = loss_cross_entropy_forward(
        predictions, targets, NULL,
        LOSS_REDUCTION_MEAN, 0.0f, 1e-7f, NULL
    );
    
    printf("  loss = %f\n", loss);
    
    if (isnan(loss)) {
        printf("FAILED: Loss is NaN\n");
    } else if (isinf(loss)) {
        printf("FAILED: Loss is Inf\n");
    } else if (loss < 0.0f) {
        printf("FAILED: Loss is negative\n");
    } else {
        printf("SUCCESS: Loss computed correctly\n");
    }
    
    // Cleanup
    tensor_free(predictions);
    tensor_free(targets);
    
    printf("\nAll tests completed!\n");
    return 0;
}
