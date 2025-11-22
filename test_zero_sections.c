#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing zero_all_gradients section by section...\n\n");
    
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
    
    printf("[1] Zeroing main gradients...\n");
    fflush(stdout);
    if (training->gradients) {
        memset(training->gradients, 0, model->header.total_params * sizeof(float));
    }
    printf("    OK\n");
    
    printf("[2] Zeroing attention gradients...\n");
    fflush(stdout);
    if (training->attention_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            printf("    Layer %u...\n", i);
            fflush(stdout);
            AttentionLayer* layer = &model->attention_layers[i];
            uint32_t dim = layer->num_heads * layer->head_dim;
            size_t lattice_size = dim * dim;
            
            if (training->attention_grads[i].query_lattice) {
                memset(training->attention_grads[i].query_lattice, 0, lattice_size * sizeof(float));
            }
            if (training->attention_grads[i].key_lattice) {
                memset(training->attention_grads[i].key_lattice, 0, lattice_size * sizeof(float));
            }
            if (training->attention_grads[i].value_lattice) {
                memset(training->attention_grads[i].value_lattice, 0, lattice_size * sizeof(float));
            }
        }
    }
    printf("    OK\n");
    
    printf("[3] Zeroing FF gradients...\n");
    fflush(stdout);
    if (training->ff_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            printf("    Layer %u...\n", i);
            fflush(stdout);
            FeedForwardLayer* layer = &model->ff_layers[i];
            
            if (training->ff_grads[i].w1_lattice) {
                memset(training->ff_grads[i].w1_lattice, 0, 
                       layer->input_dim * layer->hidden_dim * sizeof(float));
            }
            if (training->ff_grads[i].w2_lattice) {
                memset(training->ff_grads[i].w2_lattice, 0, 
                       layer->hidden_dim * layer->output_dim * sizeof(float));
            }
            if (training->ff_grads[i].bias1) {
                memset(training->ff_grads[i].bias1, 0, layer->hidden_dim * sizeof(float));
            }
            if (training->ff_grads[i].bias2) {
                memset(training->ff_grads[i].bias2, 0, layer->output_dim * sizeof(float));
            }
        }
    }
    printf("    OK\n");
    
    printf("[4] Zeroing LN gradients...\n");
    fflush(stdout);
    if (training->ln_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            printf("    Layer %u...\n", i);
            fflush(stdout);
            CLLMLayerNorm* layer = &model->layer_norms[i];
            
            printf("      gamma (dim=%u)...\n", layer->dim);
            fflush(stdout);
            if (training->ln_grads[i].gamma) {
                memset(training->ln_grads[i].gamma, 0, layer->dim * sizeof(float));
            }
            
            printf("      beta (dim=%u)...\n", layer->dim);
            fflush(stdout);
            if (training->ln_grads[i].beta) {
                memset(training->ln_grads[i].beta, 0, layer->dim * sizeof(float));
            }
        }
    }
    printf("    OK\n");
    
    printf("\n✓ All sections zeroed successfully\n");
    
    printf("\nSkipping cleanup to isolate issue...\n");
    
    // Don't cleanup to see if that's where the crash is
    // cllm_training_free(training);
    // cllm_free_model(model);
    // cllm_token_dataset_free(dataset);
    // cllm_free_tokenizer(tokenizer);
    
    return 0;
}
