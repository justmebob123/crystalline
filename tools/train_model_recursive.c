/**
 * CLLM Training Program with Recursive Sphere Hierarchy
 * 
 * This version supports the infinite recursive self-similar sphere architecture:
 * - Depth 0: 1 sphere (single-threaded)
 * - Depth 1: 13 spheres (1 control + 12 workers)
 * - Depth 2: 157 spheres (1 + 12 + 144)
 * - Depth 3: 1,885 spheres
 * - Depth n: (12^(n+1) - 1) / 11 spheres
 * 
 * Each sphere can recursively spawn 12 child spheres, creating a fractal tree
 * that scales infinitely across distributed systems and GPUs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_training_mt.h"
#include "../include/cllm_training_threaded.h"
#include "../include/cllm_recursive_spheres.h"
#include "../include/cllm_batch.h"
#include "../include/cllm_tokenizer.h"
#include "../include/cllm_data_loader.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_format.h"

void print_banner() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║   🌀 CRYSTALLINE LATTICE LANGUAGE MODEL TRAINER 🌀      ║\n");
    printf("║        Recursive Sphere Hierarchy Edition               ║\n");
    printf("║                                                          ║\n");
    printf("║   Infinite Self-Similar Fractal Architecture            ║\n");
    printf("║   Scales from 1 → 13 → 157 → 1,885 → ∞ spheres         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_usage(const char* program_name) {
    printf("Usage: %s <data_directory> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  --vocab-size <n>        Vocabulary size (default: 10000)\n");
    printf("  --embed-dim <n>         Embedding dimension (default: 256)\n");
    printf("  --num-layers <n>        Number of transformer layers (default: 6)\n");
    printf("  --num-heads <n>         Number of attention heads (default: 8)\n");
    printf("  --batch-size <n>        Batch size (default: 32)\n");
    printf("  --seq-len <n>           Sequence length (default: 128)\n");
    printf("  --learning-rate <f>     Learning rate (default: 0.0001)\n");
    printf("  --epochs <n>            Number of epochs (default: 100)\n");
    printf("  --recursive-depth <n>   Recursive sphere depth (default: 1)\n");
    printf("                          0 = single thread\n");
    printf("                          1 = 13 spheres (1 + 12)\n");
    printf("                          2 = 157 spheres (1 + 12 + 144)\n");
    printf("                          3 = 1,885 spheres\n");
    printf("                          4 = 22,621 spheres\n");
    printf("  --checkpoint-dir <dir>  Checkpoint directory (default: ./checkpoints)\n");
    printf("\n");
}

int main(int argc, char** argv) {
    print_banner();
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse arguments
    const char* data_dir = argv[1];
    int vocab_size = 10000;
    int embed_dim = 256;
    int num_layers = 6;
    int num_heads = 8;
    int batch_size = 32;
    int seq_len = 128;
    float learning_rate = 0.0001f;
    int num_epochs = 100;
    int recursive_depth = 1;  // Default: 13 spheres
    const char* checkpoint_dir = "./checkpoints";
    
    for (int i = 2; i < argc; i++) {
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
            num_epochs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--recursive-depth") == 0) {
            recursive_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--checkpoint-dir") == 0) {
            checkpoint_dir = argv[++i];
        }
    }
    
    // Create checkpoint directory
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", checkpoint_dir);
    system(cmd);
    
    // Calculate total spheres
    uint32_t total_spheres = sphere_hierarchy_count_at_depth(recursive_depth);
    
    printf("Configuration:\n");
    printf("  Data Directory: %s\n", data_dir);
    printf("  Vocabulary Size: %d\n", vocab_size);
    printf("  Embedding Dim: %d\n", embed_dim);
    printf("  Layers: %d\n", num_layers);
    printf("  Attention Heads: %d\n", num_heads);
    printf("  Batch Size: %d\n", batch_size);
    printf("  Sequence Length: %d\n", seq_len);
    printf("  Learning Rate: %.6f\n", learning_rate);
    printf("  Epochs: %d\n", num_epochs);
    printf("  🌀 Recursive Depth: %d (%u total spheres)\n", recursive_depth, total_spheres);
    printf("  Checkpoint Dir: %s\n", checkpoint_dir);
    printf("\n");
    
    // Load data
    printf("Loading data from %s...\n", data_dir);
    CLLMDataset* dataset = cllm_load_dataset_from_directory(data_dir, vocab_size);
    if (!dataset) {
        fprintf(stderr, "Failed to load dataset\n");
        return 1;
    }
    
    printf("Dataset loaded: %d tokens, %d vocabulary size\n", 
           dataset->num_tokens, dataset->vocab_size);
    
    // Create model
    printf("Creating model...\n");
    CLLMConfig config = {
        .vocab_size = dataset->vocab_size,
        .embed_dim = embed_dim,
        .num_layers = num_layers,
        .num_heads = num_heads,
        .max_seq_len = seq_len,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_model_create(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        cllm_dataset_free(dataset);
        return 1;
    }
    
    printf("Model created with %d parameters\n", model->total_params);
    
    // Create training state
    CLLMTrainingConfig train_config = {
        .batch_size = batch_size,
        .sequence_length = seq_len,
        .learning_rate = learning_rate,
        .num_epochs = num_epochs,
        .max_steps = 1000000,
        .warmup_steps = 1000,
        .gradient_clip = 1.0f,
        .weight_decay = 0.01f
    };
    
    CLLMTrainingState* training = cllm_training_create(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to create training state\n");
        cllm_model_free(model);
        cllm_dataset_free(dataset);
        return 1;
    }
    
    // Set training data
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                  STARTING TRAINING                       ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    time_t start_time = time(NULL);
    
    // Training loop
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("╔══════════════════════════════════════════════════════════╗\n");
        printf("║ Epoch %d/%d                                               ║\n", 
               epoch + 1, num_epochs);
        printf("╚══════════════════════════════════════════════════════════╝\n\n");
        
        float epoch_loss = 0.0f;
        
        // Create batch iterator
        CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
            training->tokens,
            training->num_tokens,
            batch_size,
            seq_len,
            0,  // shuffle = false
            0   // drop_last = false
        );
        
        if (!batch_iterator) {
            fprintf(stderr, "Failed to create batch iterator\n");
            break;
        }
        
        uint32_t total_batches = (training->num_tokens / seq_len) / batch_size;
        
        // Create recursive sphere hierarchy
        SphereHierarchy* hierarchy = sphere_hierarchy_create(
            training,
            batch_iterator,
            recursive_depth,
            total_batches
        );
        
        if (hierarchy) {
            printf("🌀 RECURSIVE SPHERE HIERARCHY ACTIVATED\n");
            printf("   Depth: %d\n", recursive_depth);
            printf("   Total Spheres: %u\n", hierarchy->total_spheres);
            printf("   Total Batches: %u\n", total_batches);
            printf("   Architecture: ");
            
            // Print hierarchy structure
            if (recursive_depth == 0) {
                printf("1 sphere (single-threaded)\n");
            } else if (recursive_depth == 1) {
                printf("1 control + 12 workers = 13 spheres\n");
            } else if (recursive_depth == 2) {
                printf("1 root + 12 control + 144 workers = 157 spheres\n");
            } else {
                printf("%u spheres in fractal tree\n", hierarchy->total_spheres);
            }
            printf("\n");
            
            // Train across the entire fractal hierarchy
            if (sphere_hierarchy_train(hierarchy) == 0) {
                // Copy accumulated gradients back to training state
                memcpy(training->model->gradients, 
                       hierarchy->global_gradients,
                       hierarchy->gradient_size * sizeof(float));
                
                // Apply optimizer step
                cllm_optimizer_step(training->optimizer, training->model);
                
                // Calculate average loss
                epoch_loss = training->current_loss;
                
                printf("\n");
                sphere_hierarchy_print_stats(hierarchy);
            } else {
                fprintf(stderr, "Recursive training failed\n");
            }
            
            sphere_hierarchy_destroy(hierarchy);
        } else {
            fprintf(stderr, "Failed to create sphere hierarchy\n");
        }
        
        cllm_batch_iterator_free(batch_iterator);
        
        printf("\nEpoch %d complete: Avg Loss = %.4f, Best Loss = %.4f\n", 
               epoch + 1, epoch_loss, training->best_loss);
        
        // Save checkpoint every 10 epochs
        if ((epoch + 1) % 10 == 0 || epoch == num_epochs - 1) {
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), 
                     "%s/checkpoint_epoch_%d.bin", checkpoint_dir, epoch + 1);
            
            printf("Saving checkpoint to %s...\n", checkpoint_path);
            cllm_save_checkpoint(model, training, checkpoint_path);
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║              TRAINING COMPLETE                           ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Total time: %.0f seconds (%.1f minutes)\n", elapsed, elapsed / 60.0);
    printf("Final loss: %.4f\n", training->current_loss);
    printf("Best loss:  %.4f\n", training->best_loss);
    printf("Total steps: %d\n", training->current_step);
    printf("\n");
    
    // Cleanup
    training->tokens = NULL;
    cllm_training_free(training);
    cllm_model_free(model);
    cllm_dataset_free(dataset);
    
    return 0;
}