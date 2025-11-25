#include "ai/cllm_optimizer.h"
#include <stdio.h>

int main() {
    printf("Creating config\n");
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.1f;
    
    printf("Creating parameter array\n");
    float params[3] = {1.0f, 2.0f, 3.0f};
    
    printf("Calling optimizer_create\n");
    fflush(stdout);
    OptimizerState* state = optimizer_create(&config, params, 3);
    
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return 1;
    }
    
    printf("Optimizer created successfully\n");
    printf("current_lr: %f\n", state->current_lr);
    printf("num_parameters: %zu\n", state->num_parameters);
    
    printf("Calling optimizer_free\n");
    fflush(stdout);
    optimizer_free(state);
    
    printf("Done\n");
    return 0;
}
