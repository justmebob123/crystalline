#include <stdio.h>
#include "ai/cllm_optimizer.h"

int main() {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    config.scheduler = LR_SCHEDULER_WARMUP;
    config.warmup_steps = 100;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    OptimizerState* state = optimizer_create(&config, params, 3);
    
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    // Step 0
    optimizer_step(state, gradients);
    printf("After step 0: step=%d, LR=%f\n", state->step, state->current_lr);
    
    // Steps 1-49
    for (int i = 1; i < 50; i++) {
        optimizer_step(state, gradients);
    }
    printf("After 50 steps total: step=%d, LR=%f\n", state->step, state->current_lr);
    printf("Expected: 0.001 * (50/100) = %f\n", 0.001f * (50.0f/100.0f));
    
    optimizer_free(state);
    return 0;
}
