/*
 * Crystalline Lattice Language Model (CLLM) Format Implementation
 * Revolutionary LLM format based on prime number lattices
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm_format.h"
#include "../include/cllm_utils.h"
#include "../include/prime_float_math.h"

// Use GOLDEN_RATIO from prime_types.h (included via cllm_format.h)
#ifndef GOLDEN_RATIO
#define GOLDEN_RATIO 1.618033988749895
#endif
#define SYMMETRY_ORDER 12
#define PI 3.14159265358979323846

// Initialize CLLM header
void cllm_header_init(CLLMHeader* header, const char* model_name, const char* description) {
    memset(header, 0, sizeof(CLLMHeader));
    
    // Magic number
    memcpy(header->magic, "CLLM\x01\x00\x00\x00", 8);
    
    // Version and architecture
    header->version = 1;
    header->architecture = 1;  // Transformer-based
    
    // Model parameters (defaults)
    header->vocab_size = 50000;
    header->embedding_dim = 768;
    header->num_layers = 12;
    header->num_heads = 12;
    header->context_length = 2048;
    
    // Lattice parameters
    header->symmetry_order = SYMMETRY_ORDER;
    header->golden_ratio = GOLDEN_RATIO;
    
    // Metadata
    header->timestamp = time(NULL);
    strncpy(header->model_name, model_name, sizeof(header->model_name) - 1);
    strncpy(header->description, description, sizeof(header->description) - 1);
}

// Calculate lattice coordinates from prime number
void cllm_prime_to_lattice(uint64_t prime, float coords[3], float* angle, float* radius) {
    // Use prime factorization properties to determine position
    uint64_t p = prime;
    
    // Angle based on prime modulo symmetry order
    *angle = (2.0f * PI * (p % SYMMETRY_ORDER)) / SYMMETRY_ORDER;
    
    // Radius based on golden ratio spiral
    *radius = prime_powf(GOLDEN_RATIO, prime_logf(p) / prime_logf(2.0f));
    
    // 3D coordinates
    coords[0] = *radius * prime_cosf(*angle);
    coords[1] = *radius * prime_sinf(*angle);
    coords[2] = prime_logf(p) / prime_logf(GOLDEN_RATIO);  // Height based on prime magnitude
}

// Simple primality test helper
static bool is_prime(uint64_t num) {
    if (num < 2) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;
    
    for (uint64_t i = 3; i * i <= num; i += 2) {
        if (num % i == 0) return false;
    }
    return true;
}

// Find nearest prime for a given number
uint64_t cllm_nearest_prime(uint64_t n) {
    if (n < 2) return 2;
    
    // Search upward for next prime
    while (!is_prime(n)) n++;
    return n;
}

// Encode token as prime number
uint64_t cllm_token_to_prime(uint32_t token_id) {
    // Map token ID to prime number space
    // Use nth prime formula approximation
    if (token_id == 0) return 2;
    
    // Approximate nth prime: p_n ≈ n * ln(n)
    uint64_t approx = token_id * (uint64_t)(prime_logf(token_id + 1) + 1);
    return cllm_nearest_prime(approx);
}

// Create token entry
void cllm_token_create(CLLMToken* token, uint32_t token_id, const char* token_str) {
    memset(token, 0, sizeof(CLLMToken));
    
    token->token_id = token_id;
    token->prime_encoding = cllm_token_to_prime(token_id);
    
    // Calculate lattice position
    cllm_prime_to_lattice(token->prime_encoding, token->lattice_coords, 
                         &token->spiral_angle, &token->radial_distance);
    
    // Determine symmetry group
    token->symmetry_group = token->prime_encoding % SYMMETRY_ORDER;
    
    // Copy token string
    strncpy(token->token_str, token_str, sizeof(token->token_str) - 1);
}

// Create lattice point
void cllm_lattice_point_create(CLLMLatticePoint* point, uint32_t point_id, 
                               float x, float y, float z, uint32_t prime) {
    memset(point, 0, sizeof(CLLMLatticePoint));
    
    point->point_id = point_id;
    point->coords[0] = x;
    point->coords[1] = y;
    point->coords[2] = z;
    point->prime_factor = prime;
    point->symmetry_group = prime % SYMMETRY_ORDER;
    
    // Calculate resonance (based on prime properties)
    point->resonance = 1.0f / prime_logf(prime + 1);
}

// Calculate distance between lattice points
float cllm_lattice_distance(const CLLMLatticePoint* p1, const CLLMLatticePoint* p2) {
    float dx = p1->coords[0] - p2->coords[0];
    float dy = p1->coords[1] - p2->coords[1];
    float dz = p1->coords[2] - p2->coords[2];
    return prime_sqrtf(dx*dx + dy*dy + dz*dz);
}

// Find neighbors for a lattice point
void cllm_lattice_find_neighbors(CLLMLatticePoint* point, CLLMLatticePoint* all_points, 
                                 int num_points, float max_distance) {
    point->neighbor_count = 0;
    
    for (int i = 0; i < num_points && point->neighbor_count < 12; i++) {
        if (all_points[i].point_id == point->point_id) continue;
        
        float dist = cllm_lattice_distance(point, &all_points[i]);
        if (dist <= max_distance) {
            point->neighbors[point->neighbor_count++] = all_points[i].point_id;
        }
    }
}

// Write CLLM file

// Read CLLM file

// Validate CLLM file
bool cllm_validate(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    fclose(f);
    
    // Check magic number
    if (memcmp(header.magic, "CLLM\x01\x00\x00\x00", 8) != 0) {
        return false;
    }
    
    // Check version
    if (header.version != 1) {
        return false;
    }
    
    // Check symmetry order
    if (header.symmetry_order != SYMMETRY_ORDER) {
        return false;
    }
    
    return true;
}

// Free CLLM model
void cllm_free(CLLMModel* model) {
    if (!model) return;
    
    free(model->tokens);
    free(model->lattice_points);
    free(model->embeddings.embeddings);
    free(model->embeddings.lattice_transform);
    free(model->embeddings.inverse_transform);
    
    for (uint32_t i = 0; i < model->num_layers; i++) {
        free(model->attention_layers[i].query_lattice);
        free(model->attention_layers[i].key_lattice);
        free(model->attention_layers[i].value_lattice);
        
        free(model->ff_layers[i].w1_lattice);
        free(model->ff_layers[i].w2_lattice);
        free(model->ff_layers[i].bias1);
        free(model->ff_layers[i].bias2);
        
        free(model->layer_norms[i].gamma);
        free(model->layer_norms[i].beta);
    }
    
    free(model->attention_layers);
    free(model->ff_layers);
    free(model->layer_norms);
    
    free(model->pos_encoding.spiral_positions);
    free(model->pos_encoding.clock_positions);
    free(model->pos_encoding.prime_positions);
    free(model->pos_encoding.learned_positions);
    
    free(model);
}

/**
 * Validate CLLM Header
 */
static bool cllm_validate_header(const CLLMHeader* header) {
    if (!header) return false;
    if (strncmp(header->magic, "CLLM", 4) != 0) return false;
    if (header->version == 0 || header->version > 100) return false;
    if (header->vocab_size == 0 || header->vocab_size > 1000000) return false;
    if (header->embedding_dim == 0 || header->embedding_dim > 10000) return false;
    if (header->num_layers == 0 || header->num_layers > 100) return false;
    return true;
}

/**
 * Read CLLM Model from File
 * 
 * Loads a complete model from disk including all weights and configuration.
 */
CLLMModel* cllm_read_model(const char* filepath) {
    if (!filepath) return NULL;
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open model file: %s\n", filepath);
        return NULL;
    }
    
    // Read and validate header
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, file) != 1) {
        fprintf(stderr, "Failed to read model header\n");
        fclose(file);
        return NULL;
    }
    
    if (!cllm_validate_header(&header)) {
        fprintf(stderr, "Invalid model header\n");
        fclose(file);
        return NULL;
    }
    
    // Create model configuration from header
    CLLMConfig config = {
        .vocab_size = header.vocab_size,
        .embedding_dim = header.embedding_dim,
        .num_layers = header.num_layers,
        .num_heads = header.num_heads,
        .ff_dim = header.embedding_dim * 4,  // Standard transformer ratio
        .max_seq_len = header.context_length,
        .dropout = 0.1f
    };
    
    // Create model structure
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model structure\n");
        fclose(file);
        return NULL;
    }
    
    // Read embeddings
    if (model->embeddings.embeddings) {
        size_t emb_size = model->vocab_size * model->embedding_dim;
        if (fread(model->embeddings.embeddings, sizeof(float), emb_size, file) != emb_size) {
            fprintf(stderr, "Failed to read embeddings\n");
            cllm_free_model(model);
            fclose(file);
            return NULL;
        }
        printf("  Loaded embeddings: %zu floats\n", emb_size);
    }
    
    // Read lattice transforms
    if (model->embeddings.lattice_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        size_t read = fread(model->embeddings.lattice_transform, sizeof(float), transform_size, file);
        if (read != transform_size) {
            fprintf(stderr, "Warning: Expected %zu floats, read %zu for lattice_transform\n", transform_size, read);
        }
    }
    if (model->embeddings.inverse_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        size_t read = fread(model->embeddings.inverse_transform, sizeof(float), transform_size, file);
        if (read != transform_size) {
            fprintf(stderr, "Warning: Expected %zu floats, read %zu for inverse_transform\n", transform_size, read);
        }
    }
    
    // Read attention layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        AttentionLayer* attn = &model->attention_layers[i];
        uint32_t d_model = attn->num_heads * attn->head_dim;
        
        if (attn->query_lattice) {
            size_t read = fread(attn->query_lattice, sizeof(float), d_model * d_model, file);
            (void)read; // Suppress warning
        }
        if (attn->key_lattice) {
            size_t read = fread(attn->key_lattice, sizeof(float), d_model * d_model, file);
            (void)read;
        }
        if (attn->value_lattice) {
            size_t read = fread(attn->value_lattice, sizeof(float), d_model * d_model, file);
            (void)read;
        }
    }
    
    // Read feedforward layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        FeedForwardLayer* ff = &model->ff_layers[i];
        
        if (ff->w1_lattice) {
            size_t read = fread(ff->w1_lattice, sizeof(float), ff->input_dim * ff->hidden_dim, file);
            (void)read;
        }
        if (ff->bias1) {
            size_t read = fread(ff->bias1, sizeof(float), ff->hidden_dim, file);
            (void)read;
        }
        if (ff->w2_lattice) {
            size_t read = fread(ff->w2_lattice, sizeof(float), ff->hidden_dim * ff->output_dim, file);
            (void)read;
        }
        if (ff->bias2) {
            size_t read = fread(ff->bias2, sizeof(float), ff->output_dim, file);
            (void)read;
        }
    }
    
    fclose(file);
    printf("✓ Model loaded: %s\n", filepath);
    printf("  Vocab: %lu | Embedding: %lu | Layers: %lu\n",
           (unsigned long)header.vocab_size, (unsigned long)header.embedding_dim, 
           (unsigned long)header.num_layers);
    
    return model;
}

/**
 * Write CLLM Model to File
 */
int cllm_write_model(const CLLMModel* model, const char* filepath) {
    if (!model || !filepath) return -1;
    
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "Failed to create model file: %s\n", filepath);
        return -1;
    }
    
    // Create header
    CLLMHeader header;
    memset(&header, 0, sizeof(CLLMHeader));
    memcpy(header.magic, "CLLM", 4);  // Use memcpy instead of strncpy for non-null-terminated data
    header.version = 1;
    header.vocab_size = model->vocab_size;
    header.embedding_dim = model->embedding_dim;
    header.num_layers = model->num_layers;
    header.num_heads = 8;  // Default value
    header.context_length = 512;  // Default value
    
    // Use the model's actual weight count instead of recalculating
    header.total_params = model->num_weights;
    
    // Write header
    if (fwrite(&header, sizeof(CLLMHeader), 1, file) != 1) {
        fprintf(stderr, "Failed to write header\n");
        fclose(file);
        return -1;
    }
    
    // Write embeddings
    if (model->embeddings.embeddings) {
        size_t emb_size = model->vocab_size * model->embedding_dim;
        if (fwrite(model->embeddings.embeddings, sizeof(float), emb_size, file) != emb_size) {
            fprintf(stderr, "Failed to write embeddings\n");
            fclose(file);
            return -1;
        }
        printf("  Saved embeddings: %zu floats\n", emb_size);
    }
    
    // Write lattice transforms
    if (model->embeddings.lattice_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        fwrite(model->embeddings.lattice_transform, sizeof(float), transform_size, file);
    }
    if (model->embeddings.inverse_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        fwrite(model->embeddings.inverse_transform, sizeof(float), transform_size, file);
    }
    
    // Write attention layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        AttentionLayer* attn = &model->attention_layers[i];
        uint32_t d_model = attn->num_heads * attn->head_dim;
        
        if (attn->query_lattice) fwrite(attn->query_lattice, sizeof(float), d_model * d_model, file);
        if (attn->key_lattice) fwrite(attn->key_lattice, sizeof(float), d_model * d_model, file);
        if (attn->value_lattice) fwrite(attn->value_lattice, sizeof(float), d_model * d_model, file);
    }
    
    // Write feedforward layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        FeedForwardLayer* ff = &model->ff_layers[i];
        
        if (ff->w1_lattice) fwrite(ff->w1_lattice, sizeof(float), ff->input_dim * ff->hidden_dim, file);
        if (ff->bias1) fwrite(ff->bias1, sizeof(float), ff->hidden_dim, file);
        if (ff->w2_lattice) fwrite(ff->w2_lattice, sizeof(float), ff->hidden_dim * ff->output_dim, file);
        if (ff->bias2) fwrite(ff->bias2, sizeof(float), ff->output_dim, file);
    }
    
    fclose(file);
    printf("✓ Model saved: %s\n", filepath);
    printf("  Saved %u layers with embeddings\n", model->num_layers);
    return 0;
}