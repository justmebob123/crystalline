#include "ai/cllm_optimizer.h"
#include <stdio.h>

int main() {
    printf("Creating config\n");
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.1f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    
    printf("Creating optimizer\n");
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return 1;
    }
    
    printf("Optimizer created\n");
    printf("num_parameters: %zu\n", state->num_parameters);
    printf("parameters[0]: %f\n", state->parameters[0]);
    
    printf("Manually updating parameter\n");
    state->parameters[0] -= 0.1f * 0.1f;  // lr * grad
    printf("parameters[0] after manual update: %f\n", state->parameters[0]);
    printf("params[0] (original array): %f\n", params[0]);
    
    optimizer_free(state);
    return 0;
}
