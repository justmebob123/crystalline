#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing for pointer overlap...\n\n");
    
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
    
    printf("Model weights buffer:\n");
    printf("  Start: %p\n", (void*)model->weights);
    printf("  End:   %p\n", (void*)(model->weights + model->num_weights));
    printf("  Size:  %lu floats\n", model->num_weights);
    
    printf("\nModel layer norm pointers:\n");
    for (uint32_t i = 0; i < model->num_layers; i++) {
        printf("  Layer %u gamma: %p\n", i, (void*)model->layer_norms[i].gamma);
        printf("  Layer %u beta:  %p\n", i, (void*)model->layer_norms[i].beta);
    }
    
    printf("\nTraining gradient buffers:\n");
    printf("  Main gradients: %p\n", (void*)training->gradients);
    
    printf("\nTraining LN gradient pointers:\n");
    if (training->ln_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            printf("  Layer %u gamma: %p\n", i, (void*)training->ln_grads[i].gamma);
            printf("  Layer %u beta:  %p\n", i, (void*)training->ln_grads[i].beta);
        }
    }
    
    printf("\nChecking for overlap...\n");
    void* model_start = (void*)model->weights;
    void* model_end = (void*)(model->weights + model->num_weights);
    
    for (uint32_t i = 0; i < model->num_layers; i++) {
        void* gamma = (void*)training->ln_grads[i].gamma;
        void* beta = (void*)training->ln_grads[i].beta;
        
        if (gamma >= model_start && gamma < model_end) {
            printf("  WARNING: LN grad gamma[%u] overlaps with model weights!\n", i);
        }
        if (beta >= model_start && beta < model_end) {
            printf("  WARNING: LN grad beta[%u] overlaps with model weights!\n", i);
        }
    }
    
    printf("\n✓ Check complete\n");
    
    printf("\nNow calling cllm_zero_all_gradients...\n");
    fflush(stdout);
    cllm_zero_all_gradients(training);
    printf("✓ Completed\n");
    
    // Cleanup
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}
