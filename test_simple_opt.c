#include "ai/cllm_optimizer.h"
#include <stdio.h>

int main() {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.1f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    printf("Creating optimizer...\n");
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return 1;
    }
    
    printf("Initial params: %f, %f, %f\n", params[0], params[1], params[2]);
    printf("Initial LR: %f\n", state->current_lr);
    printf("Initial step: %d\n", state->step);
    
    printf("Calling optimizer_step...\n");
    int result = optimizer_step(state, gradients);
    printf("Result: %d\n", result);
    
    if (result == 0) {
        printf("After step params: %f, %f, %f\n", params[0], params[1], params[2]);
        printf("After step LR: %f\n", state->current_lr);
        printf("After step: %d\n", state->step);
    } else {
        printf("ERROR: optimizer_step failed\n");
    }
    
    optimizer_free(state);
    return 0;
}
