#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_utils.h"
#include "include/cllm_training.h"

int main() {
    printf("=== CLLM Weight Update Test ===\n\n");
    
    CLLMConfig config = {
        .vocab_size = 10,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("ERROR: Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n");
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.01f,
        .batch_size = 2,
        .num_epochs = 1,
        .max_steps = 10,
        .weight_decay = 0.0f,
        .gradient_clip = 1.0f,
        .sequence_length = 8
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("ERROR: Failed to initialize training\n");
        return 1;
    }
    printf("✓ Training initialized\n");
    printf("  Gradients allocated: %s\n", training->gradients ? "YES" : "NO");
    printf("  Attention grads allocated: %s\n", training->attention_grads ? "YES" : "NO");
    
    // Training data
    uint32_t input_tokens[16];
    uint32_t target_tokens[16];
    
    for (int i = 0; i < 16; i++) {
        input_tokens[i] = i % 8;
        target_tokens[i] = (i + 1) % 8;
    }
    
    printf("\n=== Testing Weight Updates ===\n");
    
    // Save initial weights
    float initial_embed = model->embeddings.embeddings[0];
    float initial_attn = model->attention_layers[0].query_lattice[0];
    
    printf("Initial embedding[0] = %.8f\n", initial_embed);
    printf("Initial attention query[0] = %.8f\n", initial_attn);
    
    // Run training steps
    printf("\nRunning 10 training steps...\n");
    for (int step = 0; step < 10; step++) {
        cllm_backward_complete(training, input_tokens, target_tokens, 2, 8);
        
        // Check if gradients were computed
        if (step == 0 && training->attention_grads) {
            printf("  Attention grad[0] after backward = %.8f\n", 
                   training->attention_grads[0].query_lattice[0]);
        }
        
        cllm_adam_step(training, train_config.learning_rate);
        training->current_step++;
        
        if (step % 3 == 0) {
            printf("  Step %d: embed[0]=%.8f, attn[0]=%.8f\n", 
                   step, model->embeddings.embeddings[0],
                   model->attention_layers[0].query_lattice[0]);
        }
    }
    
    // Check final weights
    float final_embed = model->embeddings.embeddings[0];
    float final_attn = model->attention_layers[0].query_lattice[0];
    
    printf("\nFinal embedding[0] = %.8f\n", final_embed);
    printf("Final attention query[0] = %.8f\n", final_attn);
    
    float embed_change = fabs(final_embed - initial_embed);
    float attn_change = fabs(final_attn - initial_attn);
    
    printf("\n=== Results ===\n");
    printf("Embedding weight change: %.8f\n", embed_change);
    printf("Attention weight change: %.8f\n", attn_change);
    
    int success = 1;
    if (embed_change > 1e-6) {
        printf("✓ Embeddings updated\n");
    } else {
        printf("✗ Embeddings NOT updated\n");
        success = 0;
    }
    
    if (attn_change > 1e-6) {
        printf("✓ Attention weights updated\n");
    } else {
        printf("✗ Attention weights NOT updated\n");
        success = 0;
    }
    
    if (success) {
        printf("\n🎉 SUCCESS: Full training system works!\n");
    } else {
        printf("\n✗ FAILURE: Weights not updating\n");
    }
    
    return success ? 0 : 1;
}