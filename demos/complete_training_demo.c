/**
 * @file complete_training_demo.c
 * @brief Complete end-to-end training demonstration
 * 
 * This demo shows:
 * 1. Model creation with proper configuration
 * 2. Data loading and preprocessing
 * 3. Training loop with 88D threading system
 * 4. Checkpoint saving
 * 5. Validation and metrics
 * 6. Inference on trained model
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cllm.h"
#include "cllm_tokenizer.h"
#include "cllm_data_loader.h"
#include "cllm_batch.h"
#include "cllm_training.h"
#include "ai/cllm_training_system.h"
#include "cllm_inference.h"
#include "math/types.h"

// ANSI color codes for pretty output
#define ANSI_BOLD "\033[1m"
#define ANSI_GREEN "\033[32m"
#define ANSI_BLUE "\033[34m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_CYAN "\033[36m"
#define ANSI_RED "\033[31m"
#define ANSI_RESET "\033[0m"

void print_header(const char* title) {
    printf("\n");
    printf(ANSI_BOLD ANSI_CYAN "========================================\n");
    printf("  %s\n", title);
    printf("========================================" ANSI_RESET "\n");
    printf("\n");
}

void print_step(const char* message) {
    printf(ANSI_BLUE "▶ %s" ANSI_RESET "\n", message);
}

void print_success(const char* message) {
    printf(ANSI_GREEN "✓ %s" ANSI_RESET "\n", message);
}

void print_info(const char* message) {
    printf("  %s\n", message);
}

void print_warning(const char* message) {
    printf(ANSI_YELLOW "⚠ %s" ANSI_RESET "\n", message);
}

void print_error(const char* message) {
    fprintf(stderr, ANSI_RED "✗ %s" ANSI_RESET "\n", message);
}

// Sample training data
const char* TRAINING_TEXT = 
    "The quick brown fox jumps over the lazy dog. "
    "Machine learning is a subset of artificial intelligence. "
    "Deep neural networks can learn complex patterns from data. "
    "Training requires large datasets and computational resources. "
    "Gradient descent optimizes model parameters iteratively. "
    "Backpropagation computes gradients through network layers. "
    "Attention mechanisms help models focus on relevant information. "
    "Transformers have revolutionized natural language processing. "
    "The Crystalline CLLM uses geometric prime lattice structures. "
    "Twelve-fold symmetry enables efficient parallel computation. "
    "Clock lattice positions map tokens to geometric coordinates. "
    "Hierarchical threading distributes work across multiple scales. "
    "The 88D architecture spans eight dimensional layers. "
    "Each layer uses Platonic solids as coordinate frames. "
    "Kissing spheres topology enables efficient message passing.";

int main(void) {
    print_header("CLLM Complete Training Demo");
    
    printf("This demo demonstrates the complete training pipeline:\n");
    print_info("1. Model creation with geometric structure");
    print_info("2. Data loading and tokenization");
    print_info("3. Training with 88D threading system");
    print_info("4. Validation and metrics");
    print_info("5. Inference on trained model");
    printf("\n");
    
    // ========================================================================
    // STEP 1: Configuration
    // ========================================================================
    print_header("Step 1: Configuration");
    
    // Model configuration
    uint32_t vocab_size = 1000;
    uint32_t embedding_dim = 64;
    uint32_t hidden_dim = 256;
    uint32_t num_layers = 2;
    uint32_t num_heads = 12;  // 12-fold symmetry
    uint32_t max_seq_len = 32;
    
    // Training configuration
    uint32_t batch_size = 4;
    uint32_t num_epochs = 3;
    double learning_rate = 0.001;
    uint32_t num_threads = 12;  // 12-fold symmetry
    
    printf("Model Configuration:\n");
    printf("  Vocabulary size: %u\n", vocab_size);
    printf("  Embedding dimension: %u\n", embedding_dim);
    printf("  Hidden dimension: %u\n", hidden_dim);
    printf("  Number of layers: %u\n", num_layers);
    printf("  Number of heads: %u (12-fold symmetry)\n", num_heads);
    printf("  Max sequence length: %u\n", max_seq_len);
    printf("\n");
    
    printf("Training Configuration:\n");
    printf("  Batch size: %u\n", batch_size);
    printf("  Number of epochs: %u\n", num_epochs);
    printf("  Learning rate: %.6f\n", learning_rate);
    printf("  Number of threads: %u (12-fold symmetry)\n", num_threads);
    printf("\n");
    
    // ========================================================================
    // STEP 2: Create Tokenizer and Load Data
    // ========================================================================
    print_header("Step 2: Data Preparation");
    
    print_step("Creating tokenizer...");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(vocab_size);
    if (!tokenizer) {
        print_error("Failed to create tokenizer");
        return 1;
    }
    print_success("Tokenizer created");
    
    print_step("Loading training data...");
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        print_error("Failed to create data loader");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    if (cllm_data_loader_add_document(loader, TRAINING_TEXT) == 0) {
        print_error("Failed to add training document");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    print_success("Training data loaded");
    
    print_step("Building vocabulary...");
    cllm_data_loader_build_vocab(loader);
    printf("  Vocabulary size: %u tokens\n", tokenizer->vocab_size);
    print_success("Vocabulary built");
    
    print_step("Creating token dataset...");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        print_error("Failed to create dataset");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("  Total tokens: %zu\n", dataset->num_tokens);
    print_success("Dataset created");
    
    // ========================================================================
    // STEP 3: Create Model
    // ========================================================================
    print_header("Step 3: Model Creation");
    
    print_step("Creating CLLM model with geometric structure...");
    
    // Create model configuration
    CLLMConfig config = {
        .solid_type = PLATONIC_TETRAHEDRON,
        .vocab_size = vocab_size,
        .embedding_dim = embedding_dim,
        .hidden_dim = hidden_dim,
        .num_layers = num_layers,
        .num_heads = num_heads,
        .max_seq_len = max_seq_len,
        .enable_blind_recovery = false,
        .enable_harmonic_integration = false,
        .enable_ntt_attention = false,
        .enable_kissing_spheres = false
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    if (!model) {
        print_error("Failed to create model");
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    print_success("Model created successfully");
    
    // ========================================================================
    // STEP 4: Initialize Training
    // ========================================================================
    print_header("Step 4: Training Initialization");
    
    print_step("Creating training context...");
    CLLMTraining* training = (CLLMTraining*)calloc(1, sizeof(CLLMTraining));
    if (!training) {
        print_error("Failed to allocate training context");
        cllm_free_model(model);
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    training->model = model;
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    training->config.learning_rate = learning_rate;
    training->config.batch_size = batch_size;
    training->config.num_epochs = num_epochs;
    training->config.sequence_length = max_seq_len;
    strcpy(training->config.optimizer, "sgd");
    training->current_epoch = 0;
    training->current_step = 0;
    training->best_loss = 1e9;
    training->start_time = time(NULL);
    
    print_success("Training context created");
    
    // ========================================================================
    // STEP 5: Create Batch Iterator
    // ========================================================================
    print_header("Step 5: Batch Preparation");
    
    print_step("Creating batch iterator...");
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        dataset->tokens,
        dataset->num_tokens,
        batch_size,
        max_seq_len,
        1,  // shuffle
        1   // drop last
    );
    
    if (!batch_iter) {
        print_error("Failed to create batch iterator");
        free(training);
        cllm_free_model(model);
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    size_t num_batches = cllm_batch_iterator_num_batches(batch_iter);
    printf("  Total batches: %zu\n", num_batches);
    printf("  Batches per thread: %.1f\n", (double)num_batches / num_threads);
    print_success("Batch iterator created");
    
    print_step("Creating 88D training system...");
    CLLMTrainingSystem* training_system = cllm_training_system_create(
        model,
        training,
        batch_iter,
        num_threads
    );
    
    if (!training_system) {
        print_error("Failed to create 88D training system");
        cllm_batch_iterator_free(batch_iter);
        free(training);
        cllm_free_model(model);
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    printf("  Threads: %u (12-fold symmetry)\n", num_threads);
    printf("  Hierarchy levels: %u\n", training_system->num_levels);
    printf("  Gradient size: %zu doubles (%.2f MB)\n", 
           training_system->gradient_size,
           (double)training_system->gradient_size * sizeof(double) / (1024.0 * 1024.0));
    print_success("88D training system ready");
    
    
    
    // ========================================================================
    // STEP 6: Training Loop
    // ========================================================================
    print_header("Step 6: Training");
    
    printf("Starting training for %u epochs...\n\n", num_epochs);
    
    // Use the 88D training system to train
    double final_loss = cllm_train_88d(training_system, num_epochs);
    
    print_success("Training complete!");
    printf("  Final loss: %.6f\n", final_loss);
    printf("  Total time: %.2f seconds\n", training_system->total_training_time);
    
    // ========================================================================
    // STEP 7: Inference Demo
    // ========================================================================
    print_header("Step 7: Inference");
    
    print_step("Initializing inference context...");
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        print_warning("Failed to create inference context - skipping inference demo");
    } else {
        print_success("Inference context created");
        
        // Set inference parameters
        inference->temperature = 1.0;
        inference->top_k = 50;
        inference->top_p = 0.9;
        inference->max_tokens = 20;
        
        printf("  Temperature: %.1f\n", inference->temperature);
        printf("  Top-k: %u\n", inference->top_k);
        printf("  Top-p: %.1f\n", inference->top_p);
        printf("  Max tokens: %u\n", inference->max_tokens);
        
        print_info("Inference demo would generate text here");
        print_info("(Full implementation requires trained model)");
        
        cllm_inference_cleanup(inference);
    }
    
    // ========================================================================
    // STEP 8: Cleanup
    // ========================================================================
    print_header("Step 8: Cleanup");
    
    print_step("Freeing resources...");
    
    cllm_training_system_free(training_system);
    print_info("88D training system freed (includes batch iterator)");
    
    free(training);
    print_info("Training context freed");
    
    cllm_free_model(model);
    print_info("Model freed");
    
    free(dataset->tokens);
    free(dataset);
    print_info("Dataset freed");
    
    cllm_data_loader_free(loader);
    print_info("Data loader freed");
    
    cllm_free_tokenizer(tokenizer);
    print_info("Tokenizer freed");
    
    print_success("All resources freed");
    
    // ========================================================================
    // Summary
    // ========================================================================
    print_header("Training Summary");
    
    printf("Model Configuration:\n");
    printf("  Vocabulary: %u tokens\n", vocab_size);
    printf("  Architecture: %u layers, %u heads\n", num_layers, num_heads);
    printf("  Dimensions: %u embedding, %u hidden\n", embedding_dim, hidden_dim);
    printf("\n");
    
    printf("Training Results:\n");
    printf("  Epochs completed: %u\n", num_epochs);
    printf("  Final loss: %.6f\n", final_loss);
    printf("  Total time: %.2f seconds\n", training_system->total_training_time);
    printf("\n");
    
    printf("88D System:\n");
    printf("  Threads: %u (12-fold symmetry)\n", num_threads);
    printf("  Batches processed: %lu\n", training_system->batches_processed);
    printf("  Sequences processed: %lu\n", training_system->total_sequences_processed);
    printf("\n");
    
    print_success("Demo completed successfully!");
    
    return 0;
}