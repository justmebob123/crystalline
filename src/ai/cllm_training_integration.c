#include "cllm_training.h"
#include "cllm_threads.h"
#include "ai/cllm_backprop.h"
#include "ai/cllm_optimizer.h"
#include "ai/cllm_loss.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    CLLM* model;
    ThreadSystem* threads;
    OptimizerContext* optimizer;
    BackpropContext* backprop;
    LossContext* loss;
    
    float learning_rate;
    float grad_clip;
    int accumulation_steps;
    int current_step;
    
    float* gradients;
    size_t gradient_size;
} TrainingContext;

TrainingContext* training_context_create(CLLM* model, int num_threads) {
    TrainingContext* ctx = calloc(1, sizeof(TrainingContext));
    
    ctx->model = model;
    ctx->threads = threads_create(2);
    
    // Create optimizer (Adam)
    ctx->optimizer = optimizer_create(OPTIMIZER_ADAM, model->config.vocab_size * model->config.d_model);
    optimizer_set_learning_rate(ctx->optimizer, 0.0001f);
    optimizer_set_beta1(ctx->optimizer, 0.9f);
    optimizer_set_beta2(ctx->optimizer, 0.999f);
    optimizer_set_epsilon(ctx->optimizer, 1e-8f);
    
    // Create backprop context
    ctx->backprop = backprop_create(model->config.n_layers, 
                                    model->config.d_model,
                                    model->config.n_heads,
                                    model->config.d_ff);
    
    // Create loss context
    ctx->loss = loss_create(LOSS_CROSS_ENTROPY, model->config.vocab_size);
    
    ctx->learning_rate = 0.0001f;
    ctx->grad_clip = 1.0f;
    ctx->accumulation_steps = 1;
    ctx->current_step = 0;
    
    // Allocate gradient buffer
    ctx->gradient_size = model->config.vocab_size * model->config.d_model;
    ctx->gradients = calloc(ctx->gradient_size, sizeof(float));
    
    return ctx;
}

// Forward pass through model
static float forward_pass(TrainingContext* ctx, uint32_t* input_ids, 
                         uint32_t* target_ids, float* attention_mask,
                         uint32_t batch_size, uint32_t seq_len) {
    CLLM* model = ctx->model;
    
    // Get embeddings
    float* embeddings = calloc(batch_size * seq_len * model->config.d_model, sizeof(float));
    
    for (uint32_t b = 0; b < batch_size; b++) {
        for (uint32_t s = 0; s < seq_len; s++) {
            uint32_t token_id = input_ids[b * seq_len + s];
            if (token_id < model->config.vocab_size) {
                memcpy(&embeddings[(b * seq_len + s) * model->config.d_model],
                       &model->token_embeddings[token_id * model->config.d_model],
                       model->config.d_model * sizeof(float));
            }
        }
    }
    
    // Forward through layers (simplified)
    float* hidden = embeddings;
    
    // Compute logits (simplified - just use embeddings for now)
    float* logits = hidden;
    
    // Compute loss
    Tensor predictions = {
        .data = logits,
        .shape = (uint32_t[]){batch_size, seq_len, model->config.d_model},
        .ndim = 3,
        .total_size = batch_size * seq_len * model->config.d_model
    };
    
    Tensor targets = {
        .data = (float*)target_ids,
        .shape = (uint32_t[]){batch_size, seq_len},
        .ndim = 2,
        .total_size = batch_size * seq_len
    };
    
    float loss = loss_forward(ctx->loss, &predictions, &targets, NULL);
    
    return loss;
}

// Backward pass
static void backward_pass(TrainingContext* ctx, uint32_t batch_size, uint32_t seq_len) {
    // Compute gradients (simplified)
    Tensor grad_output = {
        .data = ctx->gradients,
        .shape = (uint32_t[]){batch_size, seq_len, ctx->model->config.d_model},
        .ndim = 3,
        .total_size = batch_size * seq_len * ctx->model->config.d_model
    };
    
    loss_backward(ctx->loss, &grad_output);
    
    // Backprop through layers
    backprop_backward(ctx->backprop, &grad_output);
}

// Optimizer step
static void optimizer_step(TrainingContext* ctx) {
    // Clip gradients
    float grad_norm = 0.0f;
    for (size_t i = 0; i < ctx->gradient_size; i++) {
        grad_norm += ctx->gradients[i] * ctx->gradients[i];
    }
    grad_norm = sqrtf(grad_norm);
    
    if (grad_norm > ctx->grad_clip) {
        float scale = ctx->grad_clip / grad_norm;
        for (size_t i = 0; i < ctx->gradient_size; i++) {
            ctx->gradients[i] *= scale;
        }
    }
    
    // Update weights
    optimizer_step_update(ctx->optimizer, ctx->model->token_embeddings, 
                         ctx->gradients, ctx->gradient_size);
    
    // Zero gradients
    memset(ctx->gradients, 0, ctx->gradient_size * sizeof(float));
}

// Training step
float training_step(TrainingContext* ctx, uint32_t* input_ids, 
                   uint32_t* target_ids, float* attention_mask,
                   uint32_t batch_size, uint32_t seq_len) {
    // Forward pass
    float loss = forward_pass(ctx, input_ids, target_ids, attention_mask, 
                             batch_size, seq_len);
    
    // Backward pass
    backward_pass(ctx, batch_size, seq_len);
    
    // Accumulate gradients
    ctx->current_step++;
    
    // Update weights if accumulation complete
    if (ctx->current_step >= ctx->accumulation_steps) {
        optimizer_step(ctx);
        ctx->current_step = 0;
    }
    
    return loss;
}

// Train epoch
float train_epoch(TrainingContext* ctx, DataLoader* loader, 
                 CLLMTokenizer* tokenizer, uint32_t batch_size, 
                 uint32_t seq_len) {
    float total_loss = 0.0f;
    int num_batches = 0;
    
    while (1) {
        Batch* batch = data_loader_next_batch(loader, tokenizer, batch_size, seq_len);
        if (!batch) break;
        
        float loss = training_step(ctx, batch->input_ids, batch->target_ids,
                                  batch->attention_mask, batch_size, seq_len);
        
        total_loss += loss;
        num_batches++;
        
        if (num_batches % 100 == 0) {
            printf("  Batch %d, Loss: %.4f\n", num_batches, loss);
        }
        
        batch_free(batch);
    }
    
    return (num_batches > 0) ? total_loss / num_batches : 0.0f;
}

// Full training loop
void train_model(TrainingContext* ctx, char** data_files, int num_files,
                CLLMTokenizer* tokenizer, int num_epochs, 
                uint32_t batch_size, uint32_t seq_len) {
    
    printf("Starting training...\n");
    printf("Epochs: %d\n", num_epochs);
    printf("Batch size: %d\n", batch_size);
    printf("Sequence length: %d\n", seq_len);
    printf("Learning rate: %.6f\n", ctx->learning_rate);
    
    // Start thread system
    threads_start(ctx->threads);
    
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        printf("\nEpoch %d/%d\n", epoch + 1, num_epochs);
        
        DataLoader* loader = data_loader_create(data_files, num_files);
        
        float avg_loss = train_epoch(ctx, loader, tokenizer, batch_size, seq_len);
        
        printf("Epoch %d complete. Average loss: %.4f\n", epoch + 1, avg_loss);
        
        data_loader_free(loader);
        
        // Learning rate decay
        ctx->learning_rate *= 0.95f;
        optimizer_set_learning_rate(ctx->optimizer, ctx->learning_rate);
    }
    
    threads_stop(ctx->threads);
    
    printf("\nTraining complete!\n");
}

void training_context_free(TrainingContext* ctx) {
    if (!ctx) return;
    
    threads_free(ctx->threads);
    optimizer_free(ctx->optimizer);
    backprop_free(ctx->backprop);
    loss_free(ctx->loss);
    free(ctx->gradients);
    free(ctx);
}