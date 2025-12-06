#include <stdio.h>
#include <string.h>
#include "cllm_training.h"

extern void cllm_update_learning_rate(CLLMTraining* training);

int main() {
    printf("=== Testing Learning Rate Schedule ===\n\n");
    
    CLLMTraining training = {0};
    training.config.initial_learning_rate = 0.0001f;
    training.config.learning_rate = 0.0001f;
    training.config.warmup_steps = 100;
    training.config.min_lr = 1e-6f;
    training.config.max_steps = 1000;
    strcpy(training.config.lr_scheduler, "none");
    
    printf("Config:\n");
    printf("  Initial LR: %.6f\n", training.config.initial_learning_rate);
    printf("  Warmup steps: %d\n", training.config.warmup_steps);
    printf("  Min LR: %.6f\n", training.config.min_lr);
    printf("  Scheduler: %s\n\n", training.config.lr_scheduler);
    
    printf("Learning rate during first 150 steps:\n");
    for (int step = 0; step < 150; step++) {
        training.current_step = step;
        cllm_update_learning_rate(&training);
        
        if (step < 10 || step % 10 == 0 || step == 99 || step == 100) {
            printf("  Step %3d: LR = %.8f\n", step, training.config.learning_rate);
        }
    }
    
    return 0;
}