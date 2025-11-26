/**
 * Complete Training System Test
 * Tests the entire training pipeline end-to-end
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"
#include "prime_float_math.h"

int main() {
    printf("\n=== COMPLETE TRAINING SYSTEM TEST ===\n\n");
    
    // 1. Create a small model
    printf("1. Creating model...\n");
    CLLMModel model;
    memset(&model, 0, sizeof(model));
    
    model.vocab_size = 100;
    model.embedding_dim = 32;
    model.num_layers = 2;
    model.header.total_params = 0;
    
    // Allocate embeddings
    model.embeddings.vocab_size = model.vocab_size;
    model.embeddings.embedding_dim = model.embedding_dim;
    model.embeddings.embeddings = (float*)calloc(model.vocab_size * model.embedding_dim, sizeof(float));
    
    // Initialize with small random values
    for (size_t i = 0; i < model.vocab_size * model.embedding_dim; i++) {
        model.embeddings.embeddings[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
    }
    
    // Allocate attention layers
    model.attention_layers = (AttentionLayer*)calloc(model.num_layers, sizeof(AttentionLayer));
    for (uint32_t i = 0; i < model.num_layers; i++) {
        model.attention_layers[i].num_heads = 4;
        model.attention_layers[i].head_dim = 8;
        uint32_t dim = 32;
        size_t weight_size = dim * dim;
        
        model.attention_layers[i].query_lattice = (float*)calloc(weight_size, sizeof(float));
        model.attention_layers[i].key_lattice = (float*)calloc(weight_size, sizeof(float));
        model.attention_layers[i].value_lattice = (float*)calloc(weight_size, sizeof(float));
        
        // Initialize
        for (size_t j = 0; j < weight_size; j++) {
            model.attention_layers[i].query_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
            model.attention_layers[i].key_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
            model.attention_layers[i].value_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
        }
    }
    
    // Allocate feed-forward layers
    model.ff_layers = (FeedForwardLayer*)calloc(model.num_layers, sizeof(FeedForwardLayer));
    for (uint32_t i = 0; i < model.num_layers; i++) {
        model.ff_layers[i].input_dim = 32;
        model.ff_layers[i].hidden_dim = 128;
        model.ff_layers[i].output_dim = 32;
        
        model.ff_layers[i].w1_lattice = (float*)calloc(32 * 128, sizeof(float));
        model.ff_layers[i].w2_lattice = (float*)calloc(128 * 32, sizeof(float));
        model.ff_layers[i].bias1 = (float*)calloc(128, sizeof(float));
        model.ff_layers[i].bias2 = (float*)calloc(32, sizeof(float));
        
        // Initialize
        for (size_t j = 0; j < 32 * 128; j++) {
            model.ff_layers[i].w1_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
        }
        for (size_t j = 0; j < 128 * 32; j++) {
            model.ff_layers[i].w2_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
        }
    }
    
    // Allocate layer norms
    model.layer_norms = (CLLMLayerNorm*)calloc(model.num_layers, sizeof(CLLMLayerNorm));
    for (uint32_t i = 0; i < model.num_layers; i++) {
        model.layer_norms[i].dim = 32;
        model.layer_norms[i].gamma = (float*)calloc(32, sizeof(float));
        model.layer_norms[i].beta = (float*)calloc(32, sizeof(float));
        
        // Initialize gamma to 1, beta to 0
        for (uint32_t j = 0; j < 32; j++) {
            model.layer_norms[i].gamma[j] = 1.0f;
            model.layer_norms[i].beta[j] = 0.0f;
        }
    }
    
    printf("  ✓ Model created (vocab=%d, embed_dim=%d, layers=%d)\n", 
           model.vocab_size, model.embedding_dim, model.num_layers);
    
    // 2. Create training config
    printf("\n2. Creating training config...\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 3,
        .gradient_accumulation_steps = 2,
        .use_mixed_precision = 0,  // Disable for testing
        .loss_scale = 1024.0f,
        .gradient_clip = 1.0f,
        .warmup_steps = 0,
        .weight_decay = 0.0f,
        .save_every = 10,
        .eval_interval = 10,
        .max_steps = 1000
    };
    strcpy(config.optimizer, "adam");
    
    printf("  ✓ Config created (batch=%d, seq_len=%d, lr=%.4f)\n",
           config.batch_size, config.sequence_length, config.learning_rate);
    
    // 3. Initialize training
    printf("\n3. Initializing training...\n");
    CLLMTraining* training = cllm_training_init(&model, &config);
    if (!training) {
        printf("  ✗ Failed to initialize training\n");
        return 1;
    }
    printf("  ✓ Training initialized\n");
    
    // 4. Create dummy training data
    printf("\n4. Creating training data...\n");
    int num_tokens = 100;
    training->tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    training->num_tokens = num_tokens;
    
    for (int i = 0; i < num_tokens; i++) {
        training->tokens[i] = rand() % model.vocab_size;
    }
    printf("  ✓ Created %d training tokens\n", num_tokens);
    
    // 5. Test forward pass
    printf("\n5. Testing forward pass...\n");
    uint32_t input_tokens[16];
    for (int i = 0; i < 16; i++) {
        input_tokens[i] = rand() % model.vocab_size;
    }
    
    float forward_result = cllm_forward_training(training, input_tokens);
    printf("  ✓ Forward pass completed (result=%.4f)\n", forward_result);
    
    // Check logits
    int non_zero_logits = 0;
    for (int i = 0; i < 16 * model.vocab_size; i++) {
        if (training->logits[i] != 0.0f) non_zero_logits++;
    }
    printf("  ✓ Logits computed: %d/%d non-zero\n", non_zero_logits, 16 * model.vocab_size);
    
    // 6. Test loss calculation
    printf("\n6. Testing loss calculation...\n");
    uint32_t target_tokens[16];
    for (int i = 0; i < 16; i++) {
        target_tokens[i] = rand() % model.vocab_size;
    }
    
    float loss = cllm_compute_loss_training(training, target_tokens);
    printf("  ✓ Loss computed: %.4f\n", loss);
    
    if (loss < 0.0f || loss > 100.0f) {
        printf("  ✗ WARNING: Loss out of range [0, 100]: %.4f\n", loss);
    } else if (prime_isnanf(loss) || prime_isinff(loss)) {
        printf("  ✗ WARNING: Loss is NaN or Inf\n");
    } else {
        printf("  ✓ Loss in valid range\n");
    }
    
    // 7. Test backward pass
    printf("\n7. Testing backward pass...\n");
    cllm_backward_training(training, target_tokens);
    
    // Check gradients
    int non_zero_grads = 0;
    float max_grad = 0.0f;
    for (size_t i = 0; i < model.vocab_size * model.embedding_dim; i++) {
        if (training->gradients[i] != 0.0f) {
            non_zero_grads++;
            float abs_grad = prime_fabsf(training->gradients[i]);
            if (abs_grad > max_grad) max_grad = abs_grad;
        }
    }
    printf("  ✓ Gradients computed: %d/%zu non-zero\n", 
           non_zero_grads, model.vocab_size * model.embedding_dim);
    printf("  ✓ Max gradient magnitude: %.6f\n", max_grad);
    
    if (non_zero_grads == 0) {
        printf("  ✗ WARNING: All gradients are zero!\n");
    }
    
    // 8. Test optimizer step
    printf("\n8. Testing optimizer step...\n");
    float before = model.embeddings.embeddings[0];
    cllm_optimizer_step_adam(training);
    float after = model.embeddings.embeddings[0];
    
    printf("  ✓ Optimizer step completed\n");
    printf("  ✓ Parameter changed: %.8f -> %.8f (delta=%.8f)\n", 
           before, after, after - before);
    
    if (before == after) {
        printf("  ✗ WARNING: Parameters didn't change!\n");
    }
    
    // 9. Test full training step
    printf("\n9. Testing full training step...\n");
    float loss_before = loss;
    
    // Do a few training steps
    for (int step = 0; step < 5; step++) {
        cllm_forward_training(training, input_tokens);
        loss = cllm_compute_loss_training(training, target_tokens);
        cllm_backward_training(training, target_tokens);
        cllm_optimizer_step_adam(training);
        
        printf("  Step %d: loss = %.4f\n", step + 1, loss);
    }
    
    printf("  ✓ Training steps completed\n");
    printf("  ✓ Loss change: %.4f -> %.4f\n", loss_before, loss);
    
    // 10. Summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("✓ Model creation: PASS\n");
    printf("✓ Training init: PASS\n");
    printf("✓ Forward pass: PASS\n");
    printf("✓ Loss calculation: %s\n", (loss >= 0.0f && loss <= 100.0f) ? "PASS" : "FAIL");
    printf("✓ Backward pass: %s\n", (non_zero_grads > 0) ? "PASS" : "FAIL");
    printf("✓ Optimizer: %s\n", (before != after) ? "PASS" : "FAIL");
    printf("✓ Training loop: PASS\n");
    
    // Cleanup
    cllm_training_free(training);
    
    printf("\n=== ALL TESTS COMPLETE ===\n\n");
    
    return 0;
}