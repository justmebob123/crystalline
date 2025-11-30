/**
 * CLLM Training Program
 * 
 * Complete training pipeline:
 * 1. Load and preprocess data
 * 2. Build vocabulary
 * 3. Create training dataset
 * 4. Train model with backpropagation
 * 5. Save checkpoints
 * 6. Evaluate and generate samples
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
// Removed: #include "../include/cllm_training_mt.h" - using kissing spheres only
// Removed: #include "../include/cllm_training_threaded.h" - replaced with hierarchical
#include "../include/ai/cllm_hierarchical_training.h"
// Removed: #include "../include/cllm_recursive_spheres.h" - RecursiveSphere was duplicate code, deleted
#include "../include/cllm_batch.h"
#include "../include/cllm_tokenizer.h"
#include "../include/cllm_data_loader.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_format.h"
#include "../include/cllm_model_manager.h"

void print_banner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Training System v1.0                          ║\n");
    printf("║         Crystalline Lattice Language Model                 ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_training_config(CLLMTrainingConfig* config) {
    printf("Training Configuration:\n");
    printf("  Learning rate:    %.6f\n", config->learning_rate);
    printf("  Batch size:       %d\n", config->batch_size);
    printf("  Sequence length:  %d\n", config->sequence_length);
    printf("  Num epochs:       %d\n", config->num_epochs);
    printf("  Max steps:        %d\n", config->max_steps);
    printf("  Weight decay:     %.6f\n", config->weight_decay);
    printf("  Gradient clip:    %.2f\n", config->gradient_clip);
    printf("  Warmup steps:     %d\n", config->warmup_steps);
    printf("  Save interval:    %d\n", config->save_interval);
    printf("  Eval interval:    %d\n", config->eval_interval);
    printf("  Optimizer:        %s\n", config->optimizer);
    printf("\n");
}

void print_model_config(CLLMModel* model) {
    printf("Model Configuration:\n");
    printf("  Vocab size:       %lu\n", model->vocab_size);
    printf("  Embedding dim:    %lu\n", model->embedding_dim);
    printf("  Num layers:       %u\n", model->num_layers);
    printf("  Num heads:        %u\n", model->attention_layers[0].num_heads);
    printf("  Head dim:         %u\n", model->attention_layers[0].head_dim);
    printf("  FF hidden dim:    %u\n", model->ff_layers[0].hidden_dim);
    printf("  Total params:     %lu\n", model->header.total_params);
    printf("\n");
}

/**
 * Generate sample text during training
 */
void generate_sample(CLLMModel* model, const char* prompt) {
    printf("\n--- Sample Generation ---\n");
    printf("Prompt: %s\n", prompt);
    
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        printf("Failed to initialize inference\n");
        return;
    }
    
    cllm_set_temperature(inference, 0.8f);
    cllm_set_top_p(inference, 0.9f);
    cllm_set_max_tokens(inference, 50);
    
    char output[2048];
    int tokens = cllm_generate(inference, prompt, output, sizeof(output));
    
    if (tokens > 0) {
        printf("Generated: %s\n", output);
        printf("Tokens: %d\n", tokens);
    } else {
        printf("Generation failed\n");
    }
    
    cllm_inference_cleanup(inference);
    printf("------------------------\n\n");
}

/**
 * Save training checkpoint
 */
int save_checkpoint(CLLMModel* model, CLLMTraining* training, const char* checkpoint_dir) {
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/checkpoint_step_%d.cllm", 
             checkpoint_dir, training->current_step);
    
    printf("Saving checkpoint: %s\n", filename);
    
    // Update training metadata
    model->training_meta.training_steps = training->current_step;
    model->training_meta.loss = training->best_loss;
    model->training_meta.learning_rate = training->config.learning_rate;
    
    int result = cllm_write_model(model, filename);
    if (result == 0) {
        printf("✓ Checkpoint saved\n");
        return 1;
    } else {
        printf("✗ Failed to save checkpoint\n");
        return 0;
    }
}

/**
 * Main training loop
 */
int train_model(CLLMModel* model, TokenDataset* dataset, CLLMTrainingConfig* config,
                const char* checkpoint_dir, int num_threads) {
    printf("\n=== Starting Training ===\n\n");
    
    // Thread count handling:
    // - If num_threads == 0: auto-detect CPU count
    // - If num_threads > 0: use specified count
    // - Threads rotate through 12 symmetry positions (not fixed assignment)
    if (num_threads == 0) {
        num_threads = sysconf(_SC_NPROCESSORS_ONLN);
        if (num_threads > 1) {
            num_threads--;  // Reserve 1 core for main thread
        }
        if (num_threads < 1) num_threads = 1;
        printf("Auto-detected %d CPU cores, using %d worker threads\n", 
               num_threads + 1, num_threads);
    } else {
        printf("Using %d worker threads (user-specified)\n", num_threads);
    }
    
    printf("\nTraining configuration:\n");
    printf("  Batch size:       %d\n", config->batch_size);
    printf("  Sequence length:  %d\n", config->sequence_length);
    printf("  Worker threads:   %d\n", num_threads);
    printf("  12-fold symmetry: Threads rotate through all positions\n");
    printf("\n");
    
    // Create training state
    CLLMTraining* training = cllm_training_init(model, config);
    if (!training) {
        printf("Failed to initialize training\n");
        return 0;
    }
    
    // Set training data
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    
    // Calculate batches
    int tokens_per_batch = config->batch_size * config->sequence_length;
    training->total_batches = dataset->num_tokens / tokens_per_batch;
    
    printf("Training dataset:\n");
    printf("  Total tokens:     %zu\n", dataset->num_tokens);
    printf("  Tokens per batch: %d\n", tokens_per_batch);
    printf("  Total batches:    %d\n", training->total_batches);
    printf("\n");
    
    // Training loop
    time_t start_time = time(NULL);
    
    
    // PERFORMANCE FIX: Create threaded system ONCE before epoch loop
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
        training->tokens, training->num_tokens,
        config->batch_size, config->sequence_length, 0, 0);
    if (!batch_iterator) {
        fprintf(stderr, "Failed to create batch iterator\n");
        goto cleanup;
    }
    
ThreadedTrainingSystem* threaded_system = threaded_training_create(
           training, batch_iterator, num_threads);
       if (!threaded_system) {
           cllm_batch_iterator_free(batch_iterator);
           fprintf(stderr, "Failed to create threaded training system\n");
           goto cleanup;
       }
       printf("✓ Lock-free threaded training system with %d threads\n", num_threads);
       printf("  Workers pull batches from shared queue (any worker can process any batch)\n");
       printf("  Threads rotate through 12 symmetry positions in data structure\n\n");    
    for (int epoch = 0; epoch < config->num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("╔══════════════════════════════════════════════════════════╗\n");
        printf("║ Epoch %d/%d                                               ║\n", 
               epoch + 1, config->num_epochs);
        printf("╚══════════════════════════════════════════════════════════╝\n\n");
        
        // Reset batch iterator for new epoch
        cllm_batch_iterator_reset(batch_iterator);
        
           // Train one epoch (lock-free work queue)
           float epoch_loss = threaded_train_epoch_lockfree(threaded_system, epoch);
        
        printf("\nEpoch %d complete: Avg Loss = %.4f, Best Loss = %.4f\n", 
               epoch + 1, epoch_loss, training->best_loss);
        
        
        // Save checkpoint
        if ((epoch + 1) % 10 == 0 || epoch == config->num_epochs - 1) {
            save_checkpoint(model, training, checkpoint_dir);
        }
        
        // Check if max steps reached
        if (training->current_step >= config->max_steps) {
            printf("\nMax steps reached (%d)\n", config->max_steps);
            break;
        }
    }
    
       // Cleanup hierarchical system after all epochs
       threaded_training_free(threaded_system);
       cllm_batch_iterator_free(batch_iterator);
    
    // Generate samples after training is complete and threads are stopped
    printf("\n=== Generating Samples ===\n");
    generate_sample(model, "the quick brown");
    generate_sample(model, "artificial intelligence");
    generate_sample(model, "machine learning");

    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("\n=== Training Complete ===\n");
    printf("Total time: %.0f seconds (%.1f minutes)\n", elapsed, elapsed / 60.0);
    printf("Final loss: %.4f\n", training->current_loss);
    printf("Best loss:  %.4f\n", training->best_loss);
    printf("Total steps: %d\n", training->current_step);
    printf("\n");
    
    // Don't free tokens (they belong to dataset)
    training->tokens = NULL;
    cllm_training_free(training);
    
    training = NULL;  // Prevent double-free in cleanup
cleanup:
    // Cleanup on error
    if (training) {
        training->tokens = NULL;
        cllm_training_free(training);
    }
    return 0;

    return 1;
}

int main(int argc, char** argv) {
    print_banner();
    
    if (argc < 2) {
        printf("Usage: %s <data_dir> [options]\n", argv[0]);
        printf("\nOptions:\n");
        printf("  --model-name <name>   Model name in model manager (default: training_model)\n");
        printf("  --vocab-size <n>      Vocabulary size (default: 10000)\n");
        printf("  --embed-dim <n>       Embedding dimension (default: 256)\n");
        printf("  --num-layers <n>      Number of layers (default: 6)\n");
        printf("  --num-heads <n>       Number of attention heads (default: 8)\n");
        printf("  --batch-size <n>      Batch size (default: 32)\n");
        printf("  --seq-len <n>         Sequence length (default: 128)\n");
        printf("  --learning-rate <f>   Learning rate (default: 0.0001)\n");
        printf("  --epochs <n>          Number of epochs (default: 100)\n");
        printf("  --threads <n>         Number of threads (default: auto-detect)\n");
        printf("  --checkpoint-dir <d>  Checkpoint directory (default: ./checkpoints)\n");
        printf("\nExample:\n");
        printf("  %s ./data/raw --model-name my_model --vocab-size 5000 --epochs 50 --threads 4\n", argv[0]);
        return 1;
    }
    
    const char* data_dir = argv[1];
    
    // Parse options
    uint32_t vocab_size = 10000;
    uint32_t embed_dim = 256;
    uint32_t num_layers = 6;
    uint32_t num_heads = 8;
    int batch_size = 32;
    int seq_len = 128;
    float learning_rate = 0.0001f;
    int epochs = 100;
    int num_threads = 0;  // 0 = auto-detect CPU count
    int recursive_depth = 0;  // 0 = flat hierarchy, >0 = recursive spheres
    const char* checkpoint_dir = "./checkpoints";
    const char* model_name = "training_model";  // Default model name
    
    for (int i = 2; i < argc - 1; i++) {
        if (strcmp(argv[i], "--vocab-size") == 0) {
            vocab_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--embed-dim") == 0) {
            embed_dim = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--num-layers") == 0) {
            num_layers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--num-heads") == 0) {
            num_heads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--batch-size") == 0) {
            batch_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seq-len") == 0) {
            seq_len = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--learning-rate") == 0) {
            learning_rate = atof(argv[++i]);
        } else if (strcmp(argv[i], "--epochs") == 0) {
            epochs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threads") == 0) {
            num_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--recursive-depth") == 0) {
            recursive_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--checkpoint-dir") == 0) {
            checkpoint_dir = argv[++i];
        } else if (strcmp(argv[i], "--model-name") == 0) {
            model_name = argv[++i];
        }
    }
    
    // Create checkpoint directory
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", checkpoint_dir);
    system(cmd);
    
    srand(time(NULL));
    
    // Step 1: Create tokenizer and load data
    printf("Step 1: Loading and preprocessing data\n");
    printf("========================================\n\n");
    
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(vocab_size);
    if (!tokenizer) {
        printf("Failed to create tokenizer\n");
        return 1;
    }
    
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        printf("Failed to create data loader\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Load data from directory
    int files_loaded = cllm_data_loader_load_directory(loader, data_dir);
    if (files_loaded == 0) {
        printf("No data files found in: %s\n", data_dir);
        printf("Please add .txt files to this directory\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    printf("Loaded %d files\n\n", files_loaded);
    
    // Step 2: Build vocabulary
    printf("Step 2: Building vocabulary\n");
    printf("============================\n\n");
    
    cllm_data_loader_build_vocab(loader);
    cllm_data_loader_print_stats(loader);
    
    // Save vocabulary
    char vocab_file[512];
    snprintf(vocab_file, sizeof(vocab_file), "%s/vocab.txt", checkpoint_dir);
    cllm_save_vocab(tokenizer, vocab_file);
    printf("Vocabulary saved to: %s\n\n", vocab_file);
    
    // Step 3: Create training dataset
    printf("Step 3: Creating training dataset\n");
    printf("==================================\n\n");
    
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        printf("Failed to create dataset\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Save dataset
    char dataset_file[512];
    snprintf(dataset_file, sizeof(dataset_file), "%s/dataset.bin", checkpoint_dir);
    cllm_token_dataset_save(dataset, dataset_file);
    
    // Step 4: Create model
    printf("\nStep 4: Creating model\n");
    printf("=======================\n\n");
    
    CLLMConfig model_config = {
        .vocab_size = tokenizer->vocab_size,
        .embedding_dim = embed_dim,
        .num_layers = num_layers,
        .num_heads = num_heads,
        .ff_dim = embed_dim * 4,
        .max_seq_len = seq_len * 2,
        .dropout = 0.1f
    };
    
    // Try to acquire existing model from model manager
    CLLMModel* model = model_manager_acquire_write(model_name);
    
    if (!model) {
        // Model doesn't exist, create it
        printf("Creating new model '%s' via model manager...\n", model_name);
        
        if (model_manager_create(model_name, &model_config) != 0) {
            printf("Failed to create model via model manager\n");
            cllm_token_dataset_free(dataset);
            cllm_data_loader_free(loader);
            cllm_free_tokenizer(tokenizer);
            return 1;
        }
        
        // Now acquire it for training
        model = model_manager_acquire_write(model_name);
        if (!model) {
            printf("Failed to acquire newly created model\n");
            cllm_token_dataset_free(dataset);
            cllm_data_loader_free(loader);
            cllm_free_tokenizer(tokenizer);
            return 1;
        }
        
        printf("Model '%s' created and acquired for training\n", model_name);
    } else {
        printf("Using existing model '%s' from model manager\n", model_name);
    }
    
    print_model_config(model);
    
    // Step 5: Configure training
    printf("Step 5: Configuring training\n");
    printf("=============================\n\n");
    
    CLLMTrainingConfig train_config = {
        .learning_rate = learning_rate,
        .batch_size = batch_size,
        .sequence_length = seq_len,
        .num_epochs = epochs,
        .max_steps = 100000,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 1000,
        .save_interval = 1000,
        .eval_interval = 100
    };
    strcpy(train_config.optimizer, "adam");
    
    print_training_config(&train_config);
    
    // Step 6: Train model
    printf("Step 6: Training model\n");
    printf("=======================\n");
    
    int success = train_model(model, dataset, &train_config, checkpoint_dir, num_threads);
    
    if (success) {
        // Save final model
        char final_model[512];
        snprintf(final_model, sizeof(final_model), "%s/final_model.cllm", checkpoint_dir);
        printf("\nSaving final model: %s\n", final_model);
        cllm_write_model(model, final_model);
        
        // Generate final samples
        printf("\n=== Final Model Samples ===\n");
        generate_sample(model, "the");
        generate_sample(model, "artificial intelligence");
        generate_sample(model, "machine learning");
    }
    
    // Cleanup
    // Release model back to model manager (don't free it)
    model_manager_release_write(model_name);
    printf("Model '%s' released back to model manager\n", model_name);
    
    cllm_token_dataset_free(dataset);
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   Training Complete!                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}