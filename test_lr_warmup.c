#include <stdio.h>
#include <string.h>
#include "cllm_training.h"

extern void cllm_update_learning_rate(CLLMTraining* training);

int main() {
    printf("=== Testing Learning Rate with Warmup ===\n\n");
    
    CLLMTraining training = {0};
    training.config.initial_learning_rate = 0.0001f;
    training.config.learning_rate = 0.0001f;
    training.config.warmup_steps = 100;
    training.config.min_lr = 1e-6f;
    training.config.max_steps = 1000;
    strcpy(training.config.lr_scheduler, "none");
    
    printf("Simulating 68 batches/epoch for 3 epochs (204 steps total)\n");
    printf("Warmup steps: 100\n");
    printf("Target LR: 0.0001\n\n");
    
    for (int step = 0; step < 204; step++) {
        training.current_step = step;
        cllm_update_learning_rate(&training);
        
        int epoch = step / 68 + 1;
        int batch = step % 68 + 1;
        
        if (step < 5 || step == 67 || step == 68 || step == 99 || step == 100 || step == 135 || step == 203) {
            printf("Epoch %d, Batch %2d (step %3d): LR = %.8f\n", 
                   epoch, batch, step, training.config.learning_rate);
        }
    }
    
    return 0;
}