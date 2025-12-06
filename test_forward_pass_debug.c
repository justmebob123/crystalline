/**
 * Debug forward pass to find where zeros/NaN originate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_inference.h"
#include "include/prime_float_math.h"

void print_stats(const char* name, double* arr, size_t size) {
    double sum = 0.0, max_val = 0.0, min_val = arr[0];
    int nan_count = 0, inf_count = 0, zero_count = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (isnan(arr[i])) nan_count++;
        else if (isinf(arr[i])) inf_count++;
        else if (fabs(arr[i]) < 1e-10) zero_count++;
        else {
            sum += fabs(arr[i]);
            if (fabs(arr[i]) > max_val) max_val = fabs(arr[i]);
            if (fabs(arr[i]) < min_val) min_val = fabs(arr[i]);
        }
    }
    
    printf("%s: size=%zu, avg=%.6f, max=%.6f, min=%.6f, zeros=%d, nan=%d, inf=%d\n",
           name, size, sum/size, max_val, min_val, zero_count, nan_count, inf_count);
}

int main() {
    printf("\n=== Forward Pass Debug ===\n\n");
    
    // Create tiny model
    CLLMConfig* config = (CLLMConfig*)calloc(1, sizeof(CLLMConfig));
    config->vocab_size = 50;
    config->embedding_dim = 16;
    config->num_layers = 2;
    config->num_heads = 2;
    config->ff_dim = 32;
    config->max_seq_len = 8;
    
    printf("Creating model...\n");
    extern CLLMModel* cllm_create_model(const CLLMConfig* config);
    CLLMModel* model = cllm_create_model(config);
    free(config);
    
    if (!model) {
        printf("ERROR: Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Check initial weights
    printf("Checking initial weights:\n");
    print_stats("  Embeddings", model->embeddings.embeddings, model->vocab_size * model->embedding_dim);
    print_stats("  Attention Q", model->attention_layers[0].query_lattice, model->embedding_dim * model->embedding_dim);
    print_stats("  FF W1", model->ff_layers[0].w1_lattice, model->embedding_dim * model->ff_layers[0].hidden_dim);
    printf("\n");
    
    // Create training
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 4,
        .save_every = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("ERROR: Failed to init training\n");
        return 1;
    }
    printf("✓ Training initialized\n\n");
    
    // Create simple input
    uint32_t input_tokens[] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    printf("Running forward pass...\n");
    cllm_forward_training(training, input_tokens);
    
    printf("\nChecking forward pass outputs:\n");
    print_stats("  Input embeddings", training->input_embeddings, 2 * 4 * 16);
    print_stats("  Layer 0 attention out", training->attention_outputs[0], 2 * 4 * 16);
    print_stats("  Layer 0 FF out", training->ff_outputs[0], 2 * 4 * 16);
    print_stats("  Layer 0 final out", training->layer_outputs[0], 2 * 4 * 16);
    print_stats("  Final hidden", training->final_hidden, 2 * 4 * 16);
    print_stats("  Logits", training->logits, 2 * 4 * 50);
    
    printf("\n✓ Forward pass complete\n");
    
    // Cleanup
    cllm_training_cleanup(training);
    extern void cllm_free_model(CLLMModel* model);
    cllm_free_model(model);
    
    return 0;
}