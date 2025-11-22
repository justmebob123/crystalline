#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing memset sizes in zero_all_gradients...\n\n");
    
    srand(time(NULL));
    
    // Setup
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(500);
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    cllm_data_loader_load_directory(loader, "./data/raw");
    cllm_data_loader_build_vocab(loader);
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    cllm_data_loader_free(loader);
    
    CLLMConfig model_config = {
        .vocab_size = tokenizer->vocab_size,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&model_config);
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    
    printf("Checking memset sizes:\n\n");
    
    // Main gradients
    printf("Main gradients:\n");
    printf("  total_params: %lu\n", model->header.total_params);
    printf("  size to zero: %lu bytes\n", model->header.total_params * sizeof(float));
    
    // Attention gradients
    printf("\nAttention gradients:\n");
    for (uint32_t i = 0; i < model->num_layers; i++) {
        AttentionLayer* layer = &model->attention_layers[i];
        uint32_t dim = layer->num_heads * layer->head_dim;
        size_t lattice_size = dim * dim;
        printf("  Layer %u: dim=%u, lattice_size=%zu, bytes=%zu\n", 
               i, dim, lattice_size, lattice_size * sizeof(float));
    }
    
    // FF gradients
    printf("\nFeed-forward gradients:\n");
    for (uint32_t i = 0; i < model->num_layers; i++) {
        FeedForwardLayer* layer = &model->ff_layers[i];
        printf("  Layer %u:\n", i);
        printf("    w1: %u * %u = %u floats (%zu bytes)\n",
               layer->input_dim, layer->hidden_dim,
               layer->input_dim * layer->hidden_dim,
               layer->input_dim * layer->hidden_dim * sizeof(float));
        printf("    w2: %u * %u = %u floats (%zu bytes)\n",
               layer->hidden_dim, layer->output_dim,
               layer->hidden_dim * layer->output_dim,
               layer->hidden_dim * layer->output_dim * sizeof(float));
        printf("    bias1: %u floats (%zu bytes)\n",
               layer->hidden_dim, layer->hidden_dim * sizeof(float));
        printf("    bias2: %u floats (%zu bytes)\n",
               layer->output_dim, layer->output_dim * sizeof(float));
    }
    
    // LN gradients
    printf("\nLayer norm gradients:\n");
    for (uint32_t i = 0; i < model->num_layers; i++) {
        CLLMLayerNorm* layer = &model->layer_norms[i];
        printf("  Layer %u: dim=%u, bytes=%zu\n",
               i, layer->dim, layer->dim * sizeof(float));
    }
    
    printf("\nNow calling cllm_zero_all_gradients...\n");
    fflush(stdout);
    
    // Call it
    cllm_zero_all_gradients(training);
    
    printf("✓ Completed successfully\n");
    
    // Cleanup
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}
