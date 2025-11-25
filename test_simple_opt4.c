#include "ai/cllm_optimizer.h"
#include <stdio.h>

int main() {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.1f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return 1;
    }
    
    printf("Before SGD step: params[0] = %f\n", params[0]);
    printf("current_lr = %f\n", state->current_lr);
    printf("Calling optimizer_sgd_step...\n");
    
    int result = optimizer_sgd_step(state, gradients);
    printf("Result: %d\n", result);
    
    if (result == 0) {
        printf("After SGD step: params[0] = %f\n", params[0]);
    }
    
    optimizer_free(state);
    return 0;
}
