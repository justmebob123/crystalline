#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing total_params vs actual gradient buffer size...\n\n");
    
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
    
    printf("Model parameters:\n");
    printf("  header.total_params: %lu\n", model->header.total_params);
    printf("  num_weights: %lu\n", model->num_weights);
    printf("  vocab_size: %lu\n", model->vocab_size);
    printf("  embedding_dim: %lu\n", model->embedding_dim);
    
    size_t embed_size = model->vocab_size * model->embedding_dim;
    printf("\nCalculated sizes:\n");
    printf("  Embedding params: %zu\n", embed_size);
    printf("  Total params should be: %lu\n", model->num_weights);
    
    printf("\nTraining gradient buffer:\n");
    printf("  Allocated for: %zu floats\n", embed_size);
    printf("  Trying to zero: %lu floats\n", model->header.total_params);
    
    if (model->header.total_params > embed_size) {
        printf("\n  ERROR: Trying to zero MORE than allocated!\n");
        printf("  Overflow: %lu floats (%lu bytes)\n",
               model->header.total_params - embed_size,
               (model->header.total_params - embed_size) * sizeof(float));
    }
    
    // Cleanup
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}
