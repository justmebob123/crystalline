#include "ai/cllm_optimizer.h"
#include <stdio.h>

int main() {
    printf("Test 1: Creating config\n");
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.1f;
    printf("Config created, LR = %f\n", config.learning_rate);
    
    printf("\nTest 2: Validating config\n");
    bool valid = optimizer_validate_config(&config);
    printf("Config valid: %d\n", valid);
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    
    printf("\nTest 3: Creating optimizer\n");
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return 1;
    }
    printf("Optimizer created successfully\n");
    printf("State pointer: %p\n", (void*)state);
    printf("Parameters pointer: %p\n", (void*)state->parameters);
    printf("Gradients pointer: %p\n", (void*)state->gradients);
    
    printf("\nTest 4: Calling SGD step directly\n");
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    int result = optimizer_sgd_step(state, gradients);
    printf("SGD step result: %d\n", result);
    printf("Params after SGD: %f, %f, %f\n", params[0], params[1], params[2]);
    
    optimizer_free(state);
    return 0;
}
