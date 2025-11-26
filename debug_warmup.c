#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_training.h"

int main() {
    // Create minimal model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    model->vocab_size = 50;
    model->embedding_dim = 64;
    model->num_layers = 1;
    model->embeddings.vocab_size = model->vocab_size;
    model->embeddings.embedding_dim = model->embedding_dim;
    model->embeddings.embeddings = (float*)calloc(
        model->vocab_size * model->embedding_dim, sizeof(float));
    model->attention_layers = (AttentionLayer*)calloc(1, sizeof(AttentionLayer));
    model->ff_layers = (FeedForwardLayer*)calloc(1, sizeof(FeedForwardLayer));
    model->layer_norms = (CLLMLayerNorm*)calloc(1, sizeof(CLLMLayerNorm));
    
    // Create training config
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 100,
        .warmup_steps = 10,
        .min_lr = 0.0f,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    if (training) {
        printf("Warmup phase debugging:\n");
        printf("Base LR: %.6f\n", config.learning_rate);
        printf("Warmup steps: %d\n", config.warmup_steps);
        printf("Min LR: %.6f\n\n", config.min_lr);
        
        float prev_lr = 0.0f;
        for (int step = 0; step < config.warmup_steps; step++) {
            training->current_step = step;
            cllm_update_learning_rate(training);
            float current_lr = training->config.learning_rate;
            
            printf("Step %2d: current_lr=%.6f, prev_lr=%.6f", step, current_lr, prev_lr);
            if (step > 0 && current_lr <= prev_lr) {
                printf(" ❌ NOT INCREASING!\n");
            } else {
                printf(" ✓\n");
            }
            prev_lr = current_lr;
        }
        
        cllm_training_cleanup(training);
    }
    
    // Cleanup
    free(model->embeddings.embeddings);
    free(model->attention_layers);
    free(model->ff_layers);
    free(model->layer_norms);
    free(model);
    
    return 0;
}