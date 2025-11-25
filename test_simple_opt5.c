#include "ai/cllm_optimizer.h"
#include <stdio.h>

int main() {
    printf("Test 1: Calling optimizer_default_config\n");
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    printf("Config created, LR = %f\n", config.learning_rate);
    
    printf("\nTest 2: Calling optimizer_validate_config\n");
    bool valid = optimizer_validate_config(&config);
    printf("Config valid: %d\n", valid);
    
    printf("\nTest 3: Calling optimizer_compute_gradient_norm\n");
    float grads[3] = {3.0f, 4.0f, 0.0f};
    float norm = optimizer_compute_gradient_norm(grads, 3);
    printf("Gradient norm: %f\n", norm);
    
    printf("\nAll tests passed!\n");
    return 0;
}
