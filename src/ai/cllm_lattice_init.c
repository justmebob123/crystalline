#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"
#include <stdlib.h>
#include <string.h>

// Initialize weights with lattice-aware patterns
void cllm_lattice_aware_init(CLLMModel* model, float scale) {
    if (!model || !model->weights) return;
    
    size_t total_weights = model->num_weights;
    
    // Initialize with small random values scaled by lattice structure
    for (size_t i = 0; i < total_weights; i++) {
        float random_val = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        model->weights[i] = random_val * scale;
    }
}

// Initialize with crystalline structure patterns
void cllm_crystalline_init(CLLMModel* model, float base_scale) {
    if (!model || !model->weights) return;
    
    size_t vocab_size = model->vocab_size;
    size_t hidden_size = model->embedding_dim;
    
    // Create periodic patterns in weight initialization
    for (size_t i = 0; i < vocab_size; i++) {
        for (size_t j = 0; j < hidden_size; j++) {
            size_t idx = i * hidden_size + j;
            
            // Use sine wave patterns for crystalline structure
            float phase = (float)(i + j) / (float)(vocab_size + hidden_size);
            float pattern = prime_sinf(2.0f * 3.141592653589793 * phase);
            
            float random_val = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            model->weights[idx] = (pattern * 0.3f + random_val * 0.7f) * base_scale;
        }
    }
}

// Initialize with symmetry-preserving patterns
void cllm_symmetric_init(CLLMModel* model, float scale) {
    if (!model || !model->weights) return;
    
    size_t vocab_size = model->vocab_size;
    size_t hidden_size = model->embedding_dim;
    
    // Initialize first half randomly
    for (size_t i = 0; i < vocab_size / 2; i++) {
        for (size_t j = 0; j < hidden_size; j++) {
            size_t idx = i * hidden_size + j;
            float random_val = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            model->weights[idx] = random_val * scale;
        }
    }
    
    // Mirror to second half for symmetry
    for (size_t i = 0; i < vocab_size / 2; i++) {
        for (size_t j = 0; j < hidden_size; j++) {
            size_t src_idx = i * hidden_size + j;
            size_t dst_idx = (vocab_size - 1 - i) * hidden_size + j;
            model->weights[dst_idx] = model->weights[src_idx];
        }
    }
}

// Initialize with hierarchical lattice structure
void cllm_hierarchical_lattice_init(CLLMModel* model, int num_levels, float base_scale) {
    if (!model || !model->weights || num_levels <= 0) return;
    
    size_t vocab_size = model->vocab_size;
    size_t hidden_size = model->embedding_dim;
    
    // Divide weights into hierarchical levels
    size_t level_size = vocab_size / num_levels;
    
    for (int level = 0; level < num_levels; level++) {
        float level_scale = base_scale / (float)(level + 1);
        
        size_t start_idx = level * level_size;
        size_t end_idx = (level + 1) * level_size;
        if (end_idx > vocab_size) end_idx = vocab_size;
        
        for (size_t i = start_idx; i < end_idx; i++) {
            for (size_t j = 0; j < hidden_size; j++) {
                size_t idx = i * hidden_size + j;
                float random_val = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
                model->weights[idx] = random_val * level_scale;
            }
        }
    }
}