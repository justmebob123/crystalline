/**
 * Training Parameter Validation and Auto-Adjustment
 * 
 * Validates training parameters against dataset size and automatically
 * adjusts parameters to prevent thread hanging on small datasets.
 */

#include "ai/cllm_training_validation.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Calculate number of batches for given parameters
 */
size_t calculate_num_batches(size_t num_tokens, int batch_size, int seq_len) {
    if (num_tokens == 0 || batch_size == 0 || seq_len == 0) {
        return 0;
    }
    
    size_t tokens_per_batch = (size_t)batch_size * (size_t)seq_len;
    
    // Need num_tokens - 1 because each token needs a next token for target
    size_t usable_tokens = (num_tokens > 0) ? num_tokens - 1 : 0;
    
    return usable_tokens / tokens_per_batch;
}

/**
 * Calculate optimal thread count for dataset size
 * 
 * Returns the maximum number of Level-2 worker threads that can be
 * effectively utilized given the dataset size.
 * 
 * Returns -1 if dataset is too small even for minimal threading (13 threads)
 */
int calculate_optimal_thread_count(size_t num_tokens, int batch_size, int seq_len) {
    size_t num_batches = calculate_num_batches(num_tokens, batch_size, seq_len);
    
    // Hierarchical system needs:
    // - 1 root control thread (Node Zero)
    // - 12 Level-1 control threads (one per symmetry group)
    // - N Level-2 worker threads
    // Total = 13 + N
    
    // We want at least 2 batches per thread for reasonable utilization
    const int BATCHES_PER_THREAD = 2;
    
    // Calculate maximum threads we can support
    int max_total_threads = (int)(num_batches / BATCHES_PER_THREAD);
    
    // Need at least 13 threads (1 root + 12 Level-1)
    const int MIN_THREADS = 13;
    
    if (max_total_threads < MIN_THREADS) {
        return -1;  // Dataset too small
    }
    
    // Calculate Level-2 workers (total - root - Level-1)
    int level2_workers = max_total_threads - MIN_THREADS;
    
    return level2_workers;
}

/**
 * Adjust parameters to fit dataset size
 * 
 * Tries to adjust batch_size and seq_len to create enough batches
 * for at least minimal threading (13 threads).
 * 
 * Returns 1 if successful, 0 if dataset is too small even with adjustments
 */
int adjust_parameters_for_dataset(size_t num_tokens, 
                                  int* batch_size, 
                                  int* seq_len,
                                  int* num_threads) {
    const int MIN_BATCH_SIZE = 4;
    const int MIN_SEQ_LEN = 32;
    const int MIN_BATCHES = 26;  // 13 threads * 2 batches per thread
    
    int original_batch_size = *batch_size;
    int original_seq_len = *seq_len;
    
    printf("\nAttempting parameter adjustment for small dataset...\n");
    printf("Original parameters: batch_size=%d, seq_len=%d\n", 
           original_batch_size, original_seq_len);
    
    // Try reducing batch size first
    while (*batch_size > MIN_BATCH_SIZE) {
        size_t num_batches = calculate_num_batches(num_tokens, *batch_size, *seq_len);
        
        if (num_batches >= MIN_BATCHES) {
            break;  // Found workable parameters
        }
        
        *batch_size /= 2;
    }
    
    // If still not enough, reduce sequence length
    while (*seq_len > MIN_SEQ_LEN) {
        size_t num_batches = calculate_num_batches(num_tokens, *batch_size, *seq_len);
        
        if (num_batches >= MIN_BATCHES) {
            break;  // Found workable parameters
        }
        
        *seq_len /= 2;
    }
    
    // Check if we succeeded
    size_t final_batches = calculate_num_batches(num_tokens, *batch_size, *seq_len);
    
    if (final_batches < MIN_BATCHES) {
        printf("❌ Failed to adjust parameters - dataset too small\n");
        return 0;
    }
    
    // Calculate optimal thread count with new parameters
    int optimal_threads = calculate_optimal_thread_count(num_tokens, *batch_size, *seq_len);
    
    if (optimal_threads < 0) {
        printf("❌ Failed to find workable thread count\n");
        return 0;
    }
    
    *num_threads = optimal_threads;
    
    printf("✓ Adjusted parameters successfully:\n");
    printf("  batch_size: %d → %d\n", original_batch_size, *batch_size);
    printf("  seq_len: %d → %d\n", original_seq_len, *seq_len);
    printf("  threads: %d (Level-2 workers)\n", *num_threads);
    printf("  total_batches: %zu\n", final_batches);
    
    return 1;
}

/**
 * Validate training parameters against dataset size
 * 
 * Returns 1 if valid, 0 if invalid
 * Prints detailed error messages if invalid
 */
int validate_training_parameters(size_t num_tokens,
                                 int batch_size,
                                 int seq_len,
                                 int num_threads) {
    size_t tokens_per_batch = (size_t)batch_size * (size_t)seq_len;
    size_t num_batches = calculate_num_batches(num_tokens, batch_size, seq_len);
    
    // Calculate total threads (1 root + 12 Level-1 + N Level-2)
    int total_threads = 13 + num_threads;
    
    // Need at least 2 batches per thread
    int min_batches = total_threads * 2;
    
    if (num_batches < (size_t)min_batches) {
        fprintf(stderr, "\n");
        fprintf(stderr, "═══════════════════════════════════════════════════════════\n");
        fprintf(stderr, "❌ ERROR: Dataset too small for training!\n");
        fprintf(stderr, "═══════════════════════════════════════════════════════════\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Current configuration:\n");
        fprintf(stderr, "  Dataset tokens:        %zu\n", num_tokens);
        fprintf(stderr, "  Batch size:            %d\n", batch_size);
        fprintf(stderr, "  Sequence length:       %d\n", seq_len);
        fprintf(stderr, "  Tokens per batch:      %zu\n", tokens_per_batch);
        fprintf(stderr, "  Total batches:         %zu\n", num_batches);
        fprintf(stderr, "  Thread count:          %d (1 root + 12 Level-1 + %d Level-2)\n", 
                total_threads, num_threads);
        fprintf(stderr, "  Minimum batches:       %d (2 per thread)\n", min_batches);
        fprintf(stderr, "\n");
        fprintf(stderr, "Why this fails:\n");
        fprintf(stderr, "  The hierarchical training system creates %d threads\n", total_threads);
        fprintf(stderr, "  but your dataset only has %zu batches.\n", num_batches);
        fprintf(stderr, "  Most threads will wait forever for work that never arrives.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Solutions:\n");
        fprintf(stderr, "  1. Increase dataset size to at least %zu tokens (~%zu lines)\n",
                (size_t)min_batches * tokens_per_batch,
                ((size_t)min_batches * tokens_per_batch) / 10);
        fprintf(stderr, "  2. Reduce batch size (current: %d, try: %d)\n", 
                batch_size, batch_size / 2);
        fprintf(stderr, "  3. Reduce sequence length (current: %d, try: %d)\n", 
                seq_len, seq_len / 2);
        fprintf(stderr, "  4. Reduce thread count (current: %d Level-2 workers)\n", 
                num_threads);
        fprintf(stderr, "\n");
        fprintf(stderr, "The system can automatically adjust parameters.\n");
        fprintf(stderr, "This validation prevents thread hanging.\n");
        fprintf(stderr, "\n");
        
        return 0;
    }
    
    return 1;
}

/**
 * Print dataset size recommendations
 */
void print_dataset_requirements(int batch_size, int seq_len) {
    size_t tokens_per_batch = (size_t)batch_size * (size_t)seq_len;
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Dataset Size Requirements (batch_size=%d, seq_len=%d)\n", 
           batch_size, seq_len);
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Minimum requirements for different thread counts:\n");
    printf("\n");
    
    // 13 threads (minimal: 1 root + 12 Level-1, no Level-2)
    int threads_13 = 13;
    size_t batches_13 = threads_13 * 2;
    size_t tokens_13 = batches_13 * tokens_per_batch;
    size_t lines_13 = tokens_13 / 10;  // Assuming 10 tokens per line
    
    printf("  13 threads (minimal):  %zu batches, %zu tokens, ~%zu lines\n",
           batches_13, tokens_13, lines_13);
    
    // 24 threads (1 root + 12 Level-1 + 11 Level-2)
    int threads_24 = 24;
    size_t batches_24 = threads_24 * 2;
    size_t tokens_24 = batches_24 * tokens_per_batch;
    size_t lines_24 = tokens_24 / 10;
    
    printf("  24 threads (default):  %zu batches, %zu tokens, ~%zu lines\n",
           batches_24, tokens_24, lines_24);
    
    // 62 threads (1 root + 12 Level-1 + 49 Level-2)
    int threads_62 = 62;
    size_t batches_62 = threads_62 * 2;
    size_t tokens_62 = batches_62 * tokens_per_batch;
    size_t lines_62 = tokens_62 / 10;
    
    printf("  62 threads (maximum):  %zu batches, %zu tokens, ~%zu lines\n",
           batches_62, tokens_62, lines_62);
    
    printf("\n");
    printf("If your dataset is smaller, the system will:\n");
    printf("  1. Automatically reduce thread count\n");
    printf("  2. Adjust batch size and sequence length if needed\n");
    printf("  3. Provide clear error if still too small\n");
    printf("\n");
}

/**
 * Auto-adjust training parameters for dataset
 * 
 * Main entry point for parameter validation and adjustment.
 * 
 * Returns 1 if parameters are valid or successfully adjusted
 * Returns 0 if dataset is too small even with adjustments
 */
int auto_adjust_training_parameters(size_t num_tokens,
                                    int* batch_size,
                                    int* seq_len,
                                    int* num_threads) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Training Parameter Validation\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Dataset: %zu tokens\n", num_tokens);
    printf("Requested: batch_size=%d, seq_len=%d, threads=%d\n",
           *batch_size, *seq_len, *num_threads);
    printf("\n");
    
    // First, check if current parameters are valid
    if (validate_training_parameters(num_tokens, *batch_size, *seq_len, *num_threads)) {
        printf("✓ Parameters validated successfully\n");
        printf("✓ Dataset size is sufficient for %d threads\n", *num_threads);
        return 1;
    }
    
    // Parameters invalid - try automatic adjustment
    printf("\n⚠️  Parameters invalid - attempting automatic adjustment...\n");
    
    // Try reducing thread count first
    int optimal_threads = calculate_optimal_thread_count(num_tokens, *batch_size, *seq_len);
    
    if (optimal_threads >= 0) {
        printf("\n✓ Solution found: Reduce thread count\n");
        printf("  Original threads: %d Level-2 workers\n", *num_threads);
        printf("  Adjusted threads: %d Level-2 workers\n", optimal_threads);
        *num_threads = optimal_threads;
        
        // Validate new parameters
        if (validate_training_parameters(num_tokens, *batch_size, *seq_len, *num_threads)) {
            printf("✓ Adjusted parameters validated successfully\n");
            return 1;
        }
    }
    
    // Thread reduction didn't work - try parameter adjustment
    printf("\n⚠️  Thread reduction insufficient - adjusting batch parameters...\n");
    
    if (adjust_parameters_for_dataset(num_tokens, batch_size, seq_len, num_threads)) {
        // Validate final parameters
        if (validate_training_parameters(num_tokens, *batch_size, *seq_len, *num_threads)) {
            printf("✓ Final parameters validated successfully\n");
            return 1;
        }
    }
    
    // All attempts failed
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("❌ FATAL: Dataset too small for training\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your dataset has %zu tokens.\n", num_tokens);
    printf("Even with minimal parameters, this is insufficient.\n");
    printf("\n");
    printf("Minimum dataset size: ~1,000 tokens (~100 lines)\n");
    printf("\n");
    printf("Please:\n");
    printf("  1. Add more training data\n");
    printf("  2. Combine multiple files\n");
    printf("  3. Use a larger corpus\n");
    printf("\n");
    
    return 0;
}