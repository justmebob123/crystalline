/*
 * CLLM Symmetry Operations
 * Implements symmetry group transformations for lattice embeddings
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/prime_float_math.h"

#define PI 3.14159265358979323846
#define SYMMETRY_ORDER 12

/**
 * Apply rotation transformation to embedding
 * 
 * @param embedding Input/output embedding [dim]
 * @param angle Rotation angle in radians
 * @param dim Embedding dimension
 */
static void apply_rotation(float* embedding, float angle, int dim) {
    if (!embedding || dim < 2) return;
    
    float cos_a = prime_cos(angle);
    float sin_a = prime_sin(angle);
    
    // Apply rotation in pairs of dimensions
    for (int i = 0; i < dim - 1; i += 2) {
        float x = embedding[i];
        float y = embedding[i + 1];
        
        embedding[i] = cos_a * x - sin_a * y;
        embedding[i + 1] = sin_a * x + cos_a * y;
    }
}

/**
 * Apply reflection transformation to embedding
 * 
 * @param embedding Input/output embedding [dim]
 * @param axis Reflection axis (0 = x, 1 = y, 2 = z, etc.)
 * @param dim Embedding dimension
 */
static void apply_reflection(float* embedding, int axis, int dim) {
    if (!embedding || axis < 0 || axis >= dim) return;
    
    // Reflect along specified axis
    embedding[axis] = -embedding[axis];
}

/**
 * Apply scaling transformation to embedding
 * 
 * @param embedding Input/output embedding [dim]
 * @param scale Scale factor
 * @param dim Embedding dimension
 */
static void apply_scaling(float* embedding, float scale, int dim) {
    if (!embedding || dim <= 0) return;
    
    for (int i = 0; i < dim; i++) {
        embedding[i] *= scale;
    }
}

/**
 * Compute symmetry group for a prime number
 * Maps prime to one of 12 symmetry groups
 * 
 * @param prime Prime number
 * @return Symmetry group (0 to SYMMETRY_ORDER-1)
 */
uint32_t cllm_compute_symmetry_group(uint64_t prime) {
    return (uint32_t)(prime % SYMMETRY_ORDER);
}

/**
 * Apply symmetry transformation based on group
 * 
 * @param embedding Input/output embedding [dim]
 * @param symmetry_group Symmetry group (0 to SYMMETRY_ORDER-1)
 * @param dim Embedding dimension
 */
void cllm_apply_symmetry_transform(float* embedding, int symmetry_group, int dim) {
    if (!embedding || symmetry_group < 0 || symmetry_group >= SYMMETRY_ORDER || dim <= 0) {
        return;
    }
    
    // Each symmetry group applies a different transformation
    float angle = 2.0f * PI * (float)symmetry_group / (float)SYMMETRY_ORDER;
    
    switch (symmetry_group) {
        case 0:
            // Identity - no transformation
            break;
            
        case 1:
        case 5:
        case 7:
        case 11:
            // Rotation groups
            apply_rotation(embedding, angle, dim);
            break;
            
        case 2:
        case 4:
        case 8:
        case 10:
            // Rotation + reflection
            apply_rotation(embedding, angle, dim);
            apply_reflection(embedding, 0, dim);
            break;
            
        case 3:
        case 9:
            // Rotation + scaling
            apply_rotation(embedding, angle, dim);
            apply_scaling(embedding, 1.1f, dim);
            break;
            
        case 6:
            // Pure reflection (mirror symmetry)
            apply_reflection(embedding, 0, dim);
            apply_reflection(embedding, 1, dim);
            break;
            
        default:
            // Fallback: simple rotation
            apply_rotation(embedding, angle, dim);
            break;
    }
}

/**
 * Apply inverse symmetry transformation
 * 
 * @param embedding Input/output embedding [dim]
 * @param symmetry_group Symmetry group (0 to SYMMETRY_ORDER-1)
 * @param dim Embedding dimension
 */
void cllm_apply_inverse_symmetry_transform(float* embedding, int symmetry_group, int dim) {
    if (!embedding || symmetry_group < 0 || symmetry_group >= SYMMETRY_ORDER || dim <= 0) {
        return;
    }
    
    // Apply inverse transformation (reverse order, negative angles)
    float angle = -2.0f * PI * (float)symmetry_group / (float)SYMMETRY_ORDER;
    
    switch (symmetry_group) {
        case 0:
            // Identity - no transformation
            break;
            
        case 1:
        case 5:
        case 7:
        case 11:
            // Inverse rotation
            apply_rotation(embedding, angle, dim);
            break;
            
        case 2:
        case 4:
        case 8:
        case 10:
            // Inverse: reflection first, then inverse rotation
            apply_reflection(embedding, 0, dim);
            apply_rotation(embedding, angle, dim);
            break;
            
        case 3:
        case 9:
            // Inverse: scaling, then inverse rotation
            apply_scaling(embedding, 1.0f / 1.1f, dim);
            apply_rotation(embedding, angle, dim);
            break;
            
        case 6:
            // Reflection is self-inverse
            apply_reflection(embedding, 0, dim);
            apply_reflection(embedding, 1, dim);
            break;
            
        default:
            // Fallback: inverse rotation
            apply_rotation(embedding, angle, dim);
            break;
    }
}

/**
 * Compute symmetry-invariant features
 * Extracts features that are invariant under symmetry transformations
 * 
 * @param embedding Input embedding [dim]
 * @param dim Embedding dimension
 * @param features Output features [num_features]
 * @param num_features Number of features to extract
 */
void cllm_compute_symmetry_invariants(float* embedding, int dim, 
                                     float* features, int num_features) {
    if (!embedding || !features || dim <= 0 || num_features <= 0) return;
    
    // Feature 0: L2 norm (rotation invariant)
    if (num_features > 0) {
        float norm = 0.0f;
        for (int i = 0; i < dim; i++) {
            norm += embedding[i] * embedding[i];
        }
        features[0] = prime_sqrt(norm);
    }
    
    // Feature 1: Sum of absolute values (reflection invariant)
    if (num_features > 1) {
        float sum = 0.0f;
        for (int i = 0; i < dim; i++) {
            sum += (embedding[i] >= 0) ? embedding[i] : -embedding[i];
        }
        features[1] = sum;
    }
    
    // Feature 2: Product of signs (parity)
    if (num_features > 2) {
        int sign_product = 1;
        for (int i = 0; i < dim; i++) {
            if (embedding[i] < 0) sign_product *= -1;
        }
        features[2] = (float)sign_product;
    }
    
    // Feature 3: Maximum absolute value
    if (num_features > 3) {
        float max_abs = 0.0f;
        for (int i = 0; i < dim; i++) {
            float abs_val = (embedding[i] >= 0) ? embedding[i] : -embedding[i];
            if (abs_val > max_abs) max_abs = abs_val;
        }
        features[3] = max_abs;
    }
    
    // Additional features: moments
    for (int f = 4; f < num_features && f < 8; f++) {
        float moment = 0.0f;
        int power = f - 2;
        for (int i = 0; i < dim; i++) {
            float val = embedding[i];
            float powered = val;
            for (int p = 1; p < power; p++) {
                powered *= val;
            }
            moment += powered;
        }
        features[f] = moment;
    }
}

/**
 * Apply symmetry-equivariant transformation
 * Transformation that respects symmetry structure
 * 
 * @param embedding Input/output embedding [dim]
 * @param symmetry_group Symmetry group
 * @param transform_matrix Transformation matrix [dim x dim]
 * @param dim Embedding dimension
 */
void cllm_apply_equivariant_transform(float* embedding, int symmetry_group,
                                     float* transform_matrix, int dim) {
    if (!embedding || !transform_matrix || dim <= 0) return;
    
    // First apply symmetry transformation
    cllm_apply_symmetry_transform(embedding, symmetry_group, dim);
    
    // Then apply linear transformation
    float* temp = (float*)malloc(dim * sizeof(float));
    if (!temp) return;
    
    for (int i = 0; i < dim; i++) {
        float sum = 0.0f;
        for (int j = 0; j < dim; j++) {
            sum += transform_matrix[i * dim + j] * embedding[j];
        }
        temp[i] = sum;
    }
    
    memcpy(embedding, temp, dim * sizeof(float));
    free(temp);
}

/**
 * Compute symmetry group compatibility
 * Measures how well two embeddings match under symmetry transformations
 * 
 * @param embedding1 First embedding [dim]
 * @param symmetry1 First symmetry group
 * @param embedding2 Second embedding [dim]
 * @param symmetry2 Second symmetry group
 * @param dim Embedding dimension
 * @return Compatibility score (0 to 1)
 */
float cllm_symmetry_compatibility(float* embedding1, int symmetry1,
                                 float* embedding2, int symmetry2, int dim) {
    if (!embedding1 || !embedding2 || dim <= 0) return 0.0f;
    
    // Create transformed copies
    float* e1_transformed = (float*)malloc(dim * sizeof(float));
    float* e2_transformed = (float*)malloc(dim * sizeof(float));
    
    if (!e1_transformed || !e2_transformed) {
        if (e1_transformed) free(e1_transformed);
        if (e2_transformed) free(e2_transformed);
        return 0.0f;
    }
    
    memcpy(e1_transformed, embedding1, dim * sizeof(float));
    memcpy(e2_transformed, embedding2, dim * sizeof(float));
    
    // Apply symmetry transformations
    cllm_apply_symmetry_transform(e1_transformed, symmetry1, dim);
    cllm_apply_symmetry_transform(e2_transformed, symmetry2, dim);
    
    // Compute cosine similarity
    float dot = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;
    
    for (int i = 0; i < dim; i++) {
        dot += e1_transformed[i] * e2_transformed[i];
        norm1 += e1_transformed[i] * e1_transformed[i];
        norm2 += e2_transformed[i] * e2_transformed[i];
    }
    
    free(e1_transformed);
    free(e2_transformed);
    
    norm1 = prime_sqrt(norm1);
    norm2 = prime_sqrt(norm2);
    
    if (norm1 < 1e-8f || norm2 < 1e-8f) return 0.0f;
    
    float similarity = dot / (norm1 * norm2);
    
    // Map to [0, 1]
    return (similarity + 1.0f) / 2.0f;
}

/**
 * Generate symmetry-aware attention mask
 * Creates attention mask that respects symmetry structure
 * 
 * @param symmetry_groups Array of symmetry groups [seq_len]
 * @param seq_len Sequence length
 * @param mask Output attention mask [seq_len x seq_len]
 */
void cllm_generate_symmetry_attention_mask(int* symmetry_groups, int seq_len, float* mask) {
    if (!symmetry_groups || !mask || seq_len <= 0) return;
    
    for (int i = 0; i < seq_len; i++) {
        for (int j = 0; j < seq_len; j++) {
            // Tokens in same symmetry group have higher attention weight
            if (symmetry_groups[i] == symmetry_groups[j]) {
                mask[i * seq_len + j] = 1.0f;
            } else {
                // Compute group distance
                int dist = symmetry_groups[i] - symmetry_groups[j];
                if (dist < 0) dist = -dist;
                if (dist > SYMMETRY_ORDER / 2) {
                    dist = SYMMETRY_ORDER - dist;
                }
                
                // Decay based on distance
                mask[i * seq_len + j] = 1.0f / (1.0f + 0.5f * (float)dist);
            }
        }
    }
}