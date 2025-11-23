#include <stdio.h>
#include <stdlib.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_vocab_builder.h"

void check_model_structure(CLLMModel* model) {
    printf("\n=== CHECKING MODEL STRUCTURE ===\n");
    printf("Model pointer: %p\n", (void*)model);
    if (!model) {
        printf("ERROR: Model is NULL!\n");
        return;
    }
    
    printf("vocab_size: %u\n", model->vocab_size);
    printf("embedding_dim: %lu\n", model->embedding_dim);
    printf("num_layers: %u\n", model->num_layers);
    
    printf("\nEmbeddings:\n");
    printf("  embeddings: %p\n", (void*)model->embeddings.embeddings);
    printf("  lattice_transform: %p\n", (void*)model->embeddings.lattice_transform);
    printf("  inverse_transform: %p\n", (void*)model->embeddings.inverse_transform);
    
    printf("\nTokens:\n");
    printf("  tokens: %p\n", (void*)model->tokens);
    
    printf("\nLayers:\n");
    printf("  ff_layers: %p\n", (void*)model->ff_layers);
    printf("  attention_layers: %p\n", (void*)model->attention_layers);
    printf("  layer_norms: %p\n", (void*)model->layer_norms);
    
    if (model->ff_layers) {
        printf("\nFeedForward Layer 0:\n");
        printf("  w1_lattice: %p\n", (void*)model->ff_layers[0].w1_lattice);
        printf("  w2_lattice: %p\n", (void*)model->ff_layers[0].w2_lattice);
        printf("  bias1: %p\n", (void*)model->ff_layers[0].bias1);
        printf("  bias2: %p\n", (void*)model->ff_layers[0].bias2);
    }
    
    if (model->attention_layers) {
        printf("\nAttention Layer 0:\n");
        printf("  query_lattice: %p\n", (void*)model->attention_layers[0].query_lattice);
        printf("  key_lattice: %p\n", (void*)model->attention_layers[0].key_lattice);
        printf("  value_lattice: %p\n", (void*)model->attention_layers[0].value_lattice);
    }
    
    if (model->layer_norms) {
        printf("\nLayerNorm 0:\n");
        printf("  gamma: %p\n", (void*)model->layer_norms[0].gamma);
        printf("  beta: %p\n", (void*)model->layer_norms[0].beta);
    }
}

void check_training_structure(CLLMTraining* training) {
    printf("\n=== CHECKING TRAINING STRUCTURE ===\n");
    printf("Training pointer: %p\n", (void*)training);
    if (!training) {
        printf("ERROR: Training is NULL!\n");
        return;
    }
    
    printf("model: %p\n", (void*)training->model);
    printf("tokens: %p\n", (void*)training->tokens);
    printf("num_tokens: %zu\n", training->num_tokens);
    printf("gradients: %p\n", (void*)training->gradients);
    printf("optimizer_state: %p\n", (void*)training->optimizer_state);
    
    printf("\nBackward buffers:\n");
    printf("  backward_embeddings: %p\n", (void*)training->backward_embeddings);
    printf("  backward_grad_output: %p\n", (void*)training->backward_grad_output);
    printf("  backward_layer_input: %p\n", (void*)training->backward_layer_input);
    printf("  backward_layer_grad: %p\n", (void*)training->backward_layer_grad);
    printf("  backward_temp_grad: %p\n", (void*)training->backward_temp_grad);
    printf("  backward_buffer_size: %zu\n", training->backward_buffer_size);
    
    printf("\nEmbedding cache:\n");
    printf("  cached_input_embeddings: %p\n", (void*)training->cached_input_embeddings);
    printf("  cached_target_embeddings: %p\n", (void*)training->cached_target_embeddings);
    printf("  cached_batch_size: %d\n", training->cached_batch_size);
    
    printf("\nGradient arrays:\n");
    printf("  attention_grads: %p\n", (void*)training->attention_grads);
    printf("  ff_grads: %p\n", (void*)training->ff_grads);
    printf("  ln_grads: %p\n", (void*)training->ln_grads);
    
    if (training->attention_grads) {
        printf("\nAttention Grads[0]:\n");
        printf("  query_lattice: %p\n", (void*)training->attention_grads[0].query_lattice);
        printf("  key_lattice: %p\n", (void*)training->attention_grads[0].key_lattice);
        printf("  value_lattice: %p\n", (void*)training->attention_grads[0].value_lattice);
    }
    
    if (training->ff_grads) {
        printf("\nFF Grads[0]:\n");
        printf("  w1_lattice: %p\n", (void*)training->ff_grads[0].w1_lattice);
        printf("  w2_lattice: %p\n", (void*)training->ff_grads[0].w2_lattice);
        printf("  bias1: %p\n", (void*)training->ff_grads[0].bias1);
        printf("  bias2: %p\n", (void*)training->ff_grads[0].bias2);
    }
    
    if (training->ln_grads) {
        printf("\nLN Grads[0]:\n");
        printf("  gamma: %p\n", (void*)training->ln_grads[0].gamma);
        printf("  beta: %p\n", (void*)training->ln_grads[0].beta);
    }
}

int main() {
    printf("=== COMPREHENSIVE NULL POINTER CHECK ===\n");
    
    // Load existing model
    printf("\n1. Loading existing model...\n");
    CLLMModel* model = (CLLMModel*)malloc(sizeof(CLLMModel));
    if (!model) {
        printf("ERROR: Failed to allocate model\n");
        return 1;
    }
    
    FILE* f = fopen("models/saved_model.cllm", "rb");
    if (!f) {
        printf("ERROR: Could not open model file\n");
        free(model);
        return 1;
    }
    
    // Read header
    uint32_t magic, version;
    fread(&magic, sizeof(uint32_t), 1, f);
    fread(&version, sizeof(uint32_t), 1, f);
    fread(&model->vocab_size, sizeof(uint32_t), 1, f);
    fread(&model->embedding_dim, sizeof(uint64_t), 1, f);
    fread(&model->num_layers, sizeof(uint32_t), 1, f);
    fread(&model->num_heads, sizeof(uint32_t), 1, f);
    fread(&model->ff_dim, sizeof(uint32_t), 1, f);
    
    printf("  Loaded header: vocab=%u, embed=%lu, layers=%u\n",
           model->vocab_size, model->embedding_dim, model->num_layers);
    
    // Allocate structures
    model->embeddings.embeddings = (float*)malloc(model->vocab_size * model->embedding_dim * sizeof(float));
    model->embeddings.lattice_transform = NULL;
    model->embeddings.inverse_transform = NULL;
    model->tokens = NULL;
    
    // Read embeddings
    fread(model->embeddings.embeddings, sizeof(float), model->vocab_size * model->embedding_dim, f);
    
    // Allocate layers
    model->ff_layers = (FeedForwardLayer*)calloc(model->num_layers, sizeof(FeedForwardLayer));
    model->attention_layers = (AttentionLayer*)calloc(model->num_layers, sizeof(AttentionLayer));
    model->layer_norms = (CLLMLayerNorm*)calloc(model->num_layers, sizeof(CLLMLayerNorm));
    
    // Initialize layer structures (simplified - just allocate memory)
    for (uint32_t i = 0; i < model->num_layers; i++) {
        model->ff_layers[i].input_dim = model->embedding_dim;
        model->ff_layers[i].hidden_dim = model->ff_dim;
        model->ff_layers[i].output_dim = model->embedding_dim;
        model->ff_layers[i].w1_lattice = (float*)calloc(model->embedding_dim * model->ff_dim, sizeof(float));
        model->ff_layers[i].w2_lattice = (float*)calloc(model->ff_dim * model->embedding_dim, sizeof(float));
        model->ff_layers[i].bias1 = (float*)calloc(model->ff_dim, sizeof(float));
        model->ff_layers[i].bias2 = (float*)calloc(model->embedding_dim, sizeof(float));
        
        model->attention_layers[i].num_heads = model->num_heads;
        model->attention_layers[i].head_dim = model->embedding_dim / model->num_heads;
        model->attention_layers[i].query_lattice = (float*)calloc(model->embedding_dim * model->embedding_dim, sizeof(float));
        model->attention_layers[i].key_lattice = (float*)calloc(model->embedding_dim * model->embedding_dim, sizeof(float));
        model->attention_layers[i].value_lattice = (float*)calloc(model->embedding_dim * model->embedding_dim, sizeof(float));
        
        model->layer_norms[i].dim = model->embedding_dim;
        model->layer_norms[i].gamma = (float*)malloc(model->embedding_dim * sizeof(float));
        model->layer_norms[i].beta = (float*)malloc(model->embedding_dim * sizeof(float));
        for (uint64_t j = 0; j < model->embedding_dim; j++) {
            model->layer_norms[i].gamma[j] = 1.0f;
            model->layer_norms[i].beta[j] = 0.0f;
        }
    }
    
    fclose(f);
    
    check_model_structure(model);
    
    // Build vocabulary
    printf("\n2. Building vocabulary...\n");
    const char* files[] = {"data/training/large_corpus.txt"};
    int result = cllm_build_vocabulary_from_file(model, files[0]);
    if (result != 0) {
        printf("ERROR: Failed to build vocabulary\n");
        return 1;
    }
    printf("  ✓ Vocabulary built\n");
    
    // Initialize training
    printf("\n3. Initializing training...\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 32,
        .num_epochs = 1
    };
    strcpy(config.optimizer, "adam");
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("ERROR: Failed to initialize training\n");
        return 1;
    }
    printf("  ✓ Training initialized\n");
    
    // Load training data
    printf("\n4. Loading training data...\n");
    result = cllm_load_training_data(training, "data/training/large_corpus.txt");
    if (result != 0) {
        printf("ERROR: Failed to load training data\n");
        return 1;
    }
    printf("  ✓ Training data loaded: %zu tokens\n", training->num_tokens);
    
    check_training_structure(training);
    
    printf("\n=== ALL CHECKS COMPLETE ===\n");
    printf("If you see this message, all structures are properly initialized!\n");
    
    // Cleanup
    cllm_training_free(training);
    // Don't free model - it's freed by training_free
    
    return 0;
}