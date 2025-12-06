#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_data_loader.h"
#include "cllm_tokenizer.h"

extern CLLMModel* cllm_create_model(CLLMConfig* config);
extern void cllm_free_model(CLLMModel* model);
extern float cllm_train_epoch(CLLMTraining* training);
extern void cllm_precompute_all_embeddings(CLLMModel* model);
extern float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
extern void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer);
extern float cllm_compute_loss(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens);
extern void cllm_optimizer_step_adam(CLLMTraining* training);
extern int cllm_get_batch(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens);

int main() {
    printf("=== Weight Monitoring Test ===\n\n");
    
    // Create model
    CLLMConfig model_config = {
        .vocab_size = 50,
        .embedding_dim = 32,
        .num_layers = 2,  // 2 layers to test multi-layer
        .num_heads = 2,
        .ff_dim = 64,
        .max_seq_len = 16,
        .dropout = 0.0f
    };
    
    CLLMModel* model = cllm_create_model(&model_config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    
    cllm_precompute_all_embeddings(model);
    
    // Training data
    const char* text = "The sky is blue. The grass is green. The sun is yellow. Water is wet. Fire is hot. Ice is cold.";
    
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(50);
    uint32_t num_tokens_u32 = 0;
    uint32_t* tokens = cllm_tokenizer_encode(tokenizer, text, &num_tokens_u32);
    size_t num_tokens = num_tokens_u32;
    
    printf("Tokenized %zu tokens\n\n", num_tokens);
    
    // Training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 3,
        .warmup_steps = 0,
        .gradient_accumulation_steps = 1
    };
    strcpy(train_config.lr_scheduler, "none");
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to init training\n");
        cllm_free_model(model);
        free(tokens);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    
    // Monitor weights through 3 batches
    printf("Training 3 batches and monitoring weight changes:\n\n");
    
    uint32_t* input_tokens = (uint32_t*)malloc(2 * 8 * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(2 * 8 * sizeof(uint32_t));
    
    for (int batch = 0; batch < 3; batch++) {
        // Save weights before
        double weight_before[5];
        for (int i = 0; i < 5; i++) {
            weight_before[i] = model->embeddings.embeddings[i];
        }
        
        // Get batch
        training->current_batch_offset = 0;
        int num_batch_tokens = cllm_get_batch(training, input_tokens, target_tokens);
        if (num_batch_tokens == 0) break;
        
        // Forward
        cllm_forward_training(training, input_tokens);
        
        // Compute loss
        float loss = cllm_compute_loss(training, input_tokens, target_tokens, num_batch_tokens);
        
        // Backward
        cllm_backward_training(training, target_tokens, NULL);
        
        // Check gradients
        double max_grad = 0.0;
        for (int i = 0; i < 5; i++) {
            double g = fabs(training->gradients[i]);
            if (g > max_grad) max_grad = g;
        }
        
        // Optimizer step
        cllm_optimizer_step_adam(training);
        training->current_step++;
        
        // Check weights after
        double weight_after[5];
        double max_delta = 0.0;
        for (int i = 0; i < 5; i++) {
            weight_after[i] = model->embeddings.embeddings[i];
            double delta = fabs(weight_after[i] - weight_before[i]);
            if (delta > max_delta) max_delta = delta;
        }
        
        printf("Batch %d:\n", batch + 1);
        printf("  Loss: %.6f\n", loss);
        printf("  Max gradient: %.6f\n", max_grad);
        printf("  Max weight change: %.6f\n", max_delta);
        printf("  Weight[0]: %.6f -> %.6f (delta: %+.6f)\n", 
               weight_before[0], weight_after[0], weight_after[0] - weight_before[0]);
        printf("\n");
    }
    
    free(input_tokens);
    free(target_tokens);
    cllm_training_free(training);
    cllm_free_model(model);
    free(tokens);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}