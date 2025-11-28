/**
 * CLLM Lattice-Based Embedding Initialization
 * 
 * Implements OBJECTIVE 14: Integrate L(n,d,k,λ) Lattice Formula into Training
 * 
 * This file replaces random embedding initialization with the crystalline
 * lattice formula L(n,d,k,λ) = 3^O(n,k,λ) · ∏cos(θ·φᵢ) · Γ(k) · ν(λ) · Γ(n,d)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/prime_lattice_core.h"
#include "../include/cllm_mathematical_constants.h"
#include "../include/prime_float_math.h"
#include "../include/cllm_angular_position.h"

/**
 * Initialize embeddings using L(n,d,k,λ) lattice formula
 * 
 * This is the CORRECT way to initialize embeddings for a crystalline lattice model.
 * Each embedding value is computed using the master lattice formula, which encodes:
 * - Prime structure (n)
 * - Dimensional frequencies φᵢ (d)
 * - Symmetry group (k)
 * - Phonetic layer (λ)
 * - Einstein's Λ correction (ω)
 * - Plimpton 322 corrections (ψ)
 * 
 * @param model CLLM model with tokens and embeddings
 */
void cllm_embeddings_init_lattice(CLLMModel* model) {
    if (!model || !model->embeddings.embeddings) {
        fprintf(stderr, "ERROR: Invalid model or embeddings\n");
        return;
    }
    
    printf("\n=== Initializing Embeddings with L(n,d,k,λ) Formula ===\n");
    printf("Vocabulary size: %lu\n", (unsigned long)model->vocab_size);
    printf("Embedding dimension: %u\n", model->embeddings.embedding_dim);
    printf("Using crystalline lattice formula...\n\n");
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    float* embeddings = model->embeddings.embeddings;
    
    // Statistics for verification
    double sum = 0.0;
    double sum_sq = 0.0;
    uint32_t count = 0;
    
    // Initialize each token's embedding
    for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
        CLLMToken* token = &model->tokens[token_id];
        
        // Get token properties
        uint64_t prime = token->prime_encoding;
        uint32_t symmetry_group = token->symmetry_group;
        const char* text = token->token_str;
        
        // Initialize each dimension using L(n,d,k,λ)
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            // Get dimensional frequency φᵢ for this dimension
            uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
            
            // Compute L(n,d,k,λ) for this token-dimension pair
            double L_value = L_lattice(
                prime,              // n: prime encoding of token
                dim,                // d: dimension index
                symmetry_group,     // k: symmetry group (0-11)
                text,               // λ: phonetic layer
                3,                  // ω: Einstein's Λ numerator (3/144000)
                prime,              // p: for Plimpton 322 correction
                phi_i               // q: dimensional frequency
            );
            
            // Normalize to reasonable range [-1, 1] using tanh
            // Scale factor of 100 chosen empirically for good distribution
            double normalized = prime_tanh(L_value / 100.0);
            
            // Store in embedding matrix
            embeddings[token_id * embedding_dim + dim] = (float)normalized;
            
            // Update statistics
            sum += normalized;
            sum_sq += normalized * normalized;
            count++;
        }
        
        // Progress indicator
        if ((token_id + 1) % 1000 == 0 || token_id == vocab_size - 1) {
            printf("  Initialized %u/%u tokens (%.1f%%)\r", 
                   token_id + 1, vocab_size, 
                   100.0 * (token_id + 1) / vocab_size);
            fflush(stdout);
        }
    }
    
    printf("\n\n");
    
    // Compute and display statistics
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double stddev = prime_sqrt(variance);
    
    printf("=== Embedding Statistics ===\n");
    printf("Mean: %.6f\n", mean);
    printf("Std Dev: %.6f\n", stddev);
    printf("Total values: %u\n", count);
    printf("\n");
    
    // Verify embeddings are in valid range
    int out_of_range = 0;
    for (uint32_t i = 0; i < vocab_size * embedding_dim; i++) {
        if (embeddings[i] < -1.0f || embeddings[i] > 1.0f) {
            out_of_range++;
        }
    }
    
    if (out_of_range > 0) {
        printf("WARNING: %d values out of [-1, 1] range\n", out_of_range);
    } else {
        printf("✓ All embeddings in valid range [-1, 1]\n");
    }
    
    printf("✓ Lattice embedding initialization complete\n\n");
}

/**
 * Verify embedding quality by checking symmetry group similarity
 * 
 * Tokens in the same symmetry group should have more similar embeddings
 * than tokens in different groups (due to shared φᵢ values).
 * 
 * @param model CLLM model with initialized embeddings
 */
void cllm_verify_embedding_symmetry(CLLMModel* model) {
    if (!model || !model->embeddings.embeddings) return;
    
    printf("\n=== Verifying Embedding Symmetry ===\n");
    
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    float* embeddings = model->embeddings.embeddings;
    
    // For each symmetry group, compute average intra-group similarity
    double group_similarities[12] = {0};
    int group_counts[12] = {0};
    
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        uint32_t group_i = model->tokens[i].symmetry_group;
        
        // Find another token in the same group
        for (uint32_t j = i + 1; j < model->vocab_size; j++) {
            uint32_t group_j = model->tokens[j].symmetry_group;
            
            if (group_i == group_j) {
                // Compute cosine similarity
                float dot = 0.0f;
                float norm_i = 0.0f;
                float norm_j = 0.0f;
                
                for (uint32_t d = 0; d < embedding_dim; d++) {
                    float val_i = embeddings[i * embedding_dim + d];
                    float val_j = embeddings[j * embedding_dim + d];
                    dot += val_i * val_j;
                    norm_i += val_i * val_i;
                    norm_j += val_j * val_j;
                }
                
                float similarity = dot / (prime_sqrtf(norm_i) * prime_sqrtf(norm_j));
                group_similarities[group_i] += similarity;
                group_counts[group_i]++;
                
                break; // Only check one pair per token
            }
        }
    }
    
    // Display results
    printf("\nIntra-group similarity by symmetry group:\n");
    for (int g = 0; g < 12; g++) {
        if (group_counts[g] > 0) {
            double avg_sim = group_similarities[g] / group_counts[g];
            printf("  Group %2d: %.4f (from %d pairs)\n", g, avg_sim, group_counts[g]);
        }
    }
    
    printf("\n✓ Symmetry verification complete\n\n");
}

/**
 * Compare lattice embeddings to random baseline
 * 
 * Useful for debugging and validation. Shows the difference between
 * random initialization and lattice-based initialization.
 * 
 * @param model CLLM model
 * @param num_samples Number of tokens to sample for comparison
 */
void cllm_compare_embedding_methods(CLLMModel* model, uint32_t num_samples) {
    if (!model || num_samples == 0) return;
    
    printf("\n=== Comparing Embedding Methods ===\n");
    
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    
    // Allocate space for random embeddings
    float* random_embeddings = (float*)calloc(num_samples * embedding_dim, sizeof(float));
    if (!random_embeddings) return;
    
    // Generate random embeddings for comparison
    for (uint32_t i = 0; i < num_samples * embedding_dim; i++) {
        random_embeddings[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    }
    
    // Compute statistics for both
    double lattice_mean = 0.0, lattice_var = 0.0;
    double random_mean = 0.0, random_var = 0.0;
    
    for (uint32_t i = 0; i < num_samples; i++) {
        for (uint32_t d = 0; d < embedding_dim; d++) {
            float lattice_val = model->embeddings.embeddings[i * embedding_dim + d];
            float random_val = random_embeddings[i * embedding_dim + d];
            
            lattice_mean += lattice_val;
            lattice_var += lattice_val * lattice_val;
            random_mean += random_val;
            random_var += random_val * random_val;
        }
    }
    
    uint32_t count = num_samples * embedding_dim;
    lattice_mean /= count;
    random_mean /= count;
    lattice_var = lattice_var / count - lattice_mean * lattice_mean;
    random_var = random_var / count - random_mean * random_mean;
    
    printf("\nLattice embeddings:\n");
    printf("  Mean: %.6f\n", lattice_mean);
    printf("  Variance: %.6f\n", lattice_var);
    printf("  Std Dev: %.6f\n", prime_sqrt(lattice_var));
    
    printf("\nRandom embeddings:\n");
    printf("  Mean: %.6f\n", random_mean);
    printf("  Variance: %.6f\n", random_var);
    printf("  Std Dev: %.6f\n", prime_sqrt(random_var));
    
    printf("\n✓ Comparison complete\n\n");
    
    free(random_embeddings);
}

/**
 * Initialize embeddings with lattice formula and verify quality
 * 
 * This is the main entry point for lattice-based embedding initialization.
 * It initializes embeddings and performs quality checks.
 * 
 * @param model CLLM model
 */
void cllm_init_embeddings_with_lattice(CLLMModel* model) {
    // Initialize with lattice formula
    cllm_embeddings_init_lattice(model);
    
    // Verify symmetry properties
    cllm_verify_embedding_symmetry(model);
    
    // Compare to random baseline (sample 100 tokens)
    uint32_t num_samples = (model->vocab_size < 100) ? model->vocab_size : 100;
    cllm_compare_embedding_methods(model, num_samples);
}