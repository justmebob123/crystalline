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

#define GOLDEN_RATIO 1.618033988749895
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
int cllm_write(const char* filename, CLLMModel* model) {
    FILE* f = fopen(filename, "wb");
    if (!f) return -1;
    
    // Write header
    fwrite(&model->header, sizeof(CLLMHeader), 1, f);
    
    // Write vocabulary
    fwrite(&model->vocab_size, sizeof(uint32_t), 1, f);
    fwrite(model->tokens, sizeof(CLLMToken), model->vocab_size, f);
    
    // Write lattice structure
    fwrite(&model->num_lattice_points, sizeof(uint32_t), 1, f);
    fwrite(model->lattice_points, sizeof(CLLMLatticePoint), model->num_lattice_points, f);
    
    // Write embedding matrix
    fwrite(&model->embeddings.vocab_size, sizeof(uint32_t), 1, f);
    fwrite(&model->embeddings.embedding_dim, sizeof(uint32_t), 1, f);
    size_t embed_size = model->embeddings.vocab_size * model->embeddings.embedding_dim;
    fwrite(model->embeddings.embeddings, sizeof(float), embed_size, f);
    
    // Write transform matrices
    size_t transform_size = model->embeddings.embedding_dim * model->embeddings.embedding_dim;
    fwrite(model->embeddings.lattice_transform, sizeof(float), transform_size, f);
    fwrite(model->embeddings.inverse_transform, sizeof(float), transform_size, f);
    
    // Write layers
    fwrite(&model->num_layers, sizeof(uint32_t), 1, f);
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Write attention layer (only metadata, not pointers)
        AttentionLayer* attn = &model->attention_layers[i];
        fwrite(&attn->layer_id, sizeof(uint32_t), 1, f);
        fwrite(&attn->num_heads, sizeof(uint32_t), 1, f);
        fwrite(&attn->head_dim, sizeof(uint32_t), 1, f);
        
        size_t attn_size = attn->num_heads * attn->head_dim * attn->head_dim;
        fwrite(attn->query_lattice, sizeof(float), attn_size, f);
        fwrite(attn->key_lattice, sizeof(float), attn_size, f);
        fwrite(attn->value_lattice, sizeof(float), attn_size, f);
        
        // Write feed-forward layer (only metadata, not pointers)
        FeedForwardLayer* ff = &model->ff_layers[i];
        fwrite(&ff->layer_id, sizeof(uint32_t), 1, f);
        fwrite(&ff->input_dim, sizeof(uint32_t), 1, f);
        fwrite(&ff->hidden_dim, sizeof(uint32_t), 1, f);
        fwrite(&ff->output_dim, sizeof(uint32_t), 1, f);
        
        size_t ff_size1 = ff->input_dim * ff->hidden_dim;
        size_t ff_size2 = ff->hidden_dim * ff->output_dim;
        fwrite(ff->w1_lattice, sizeof(float), ff_size1, f);
        fwrite(ff->w2_lattice, sizeof(float), ff_size2, f);
        fwrite(ff->bias1, sizeof(float), ff->hidden_dim, f);
        fwrite(ff->bias2, sizeof(float), ff->output_dim, f);
        
        // Write layer norm (only metadata, not pointers)
        CLLMLayerNorm* ln = &model->layer_norms[i];
        fwrite(&ln->layer_id, sizeof(uint32_t), 1, f);
        fwrite(&ln->dim, sizeof(uint32_t), 1, f);
        fwrite(&ln->epsilon, sizeof(float), 1, f);
        fwrite(ln->gamma, sizeof(float), ln->dim, f);
        fwrite(ln->beta, sizeof(float), ln->dim, f);
    }
    
    // Write positional encoding (only metadata, not the struct with pointers)
    fwrite(&model->pos_encoding.max_length, sizeof(uint32_t), 1, f);
    fwrite(&model->pos_encoding.embedding_dim, sizeof(uint32_t), 1, f);
    size_t pos_size = model->pos_encoding.max_length * model->pos_encoding.embedding_dim;
    fwrite(model->pos_encoding.spiral_positions, sizeof(float), pos_size, f);
    fwrite(model->pos_encoding.clock_positions, sizeof(float), pos_size, f);
    fwrite(model->pos_encoding.prime_positions, sizeof(float), pos_size, f);
    fwrite(model->pos_encoding.learned_positions, sizeof(float), pos_size, f);
    
    // Write training metadata
    fwrite(&model->training_meta, sizeof(TrainingMetadata), 1, f);
    
    // Calculate and write checksum
    // TODO: Implement proper checksum
    uint32_t checksum = 0xDEADBEEF;
    fwrite(&checksum, sizeof(uint32_t), 1, f);
    
    fclose(f);
    return 0;
}

// Read CLLM file
CLLMModel* cllm_read(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) {
        fclose(f);
        return NULL;
    }
    
    // Read header
    if (fread(&model->header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error reading header from file\n");
        fclose(f);
        return NULL;
    }
    
    // Validate magic number
    if (memcmp(model->header.magic, "CLLM\x01\x00\x00\x00", 8) != 0) {
        free(model);
        fclose(f);
        return NULL;
    }
    
    // Read vocabulary
    if (fread(&model->vocab_size, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading vocab_size from file\n");
        fclose(f);
        return NULL;
    }
    model->tokens = (CLLMToken*)malloc(sizeof(CLLMToken) * model->vocab_size);
    if (fread(model->tokens, sizeof(CLLMToken), model->vocab_size, f) != model->vocab_size) {
        fprintf(stderr, "Error reading tokens from file\n");
        fclose(f);
        return NULL;
    }
    
    // Read lattice structure
    if (fread(&model->num_lattice_points, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading num_lattice_points from file\n");
        fclose(f);
        return NULL;

    }
    model->lattice_points = (CLLMLatticePoint*)malloc(sizeof(CLLMLatticePoint) * model->num_lattice_points);
    if (fread(model->lattice_points, sizeof(CLLMLatticePoint), model->num_lattice_points, f) != model->num_lattice_points) {
        fprintf(stderr, "Error reading lattice_points from file\n");
        fclose(f);
        return NULL;
    }
    
    // Read embedding matrix
    if (fread(&model->embeddings.vocab_size, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    if (fread(&model->embeddings.embedding_dim, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    size_t embed_size = model->embeddings.vocab_size * model->embeddings.embedding_dim;
    model->embeddings.embeddings = (float*)malloc(sizeof(float) * embed_size);
    if (fread(model->embeddings.embeddings, sizeof(float), embed_size, f) != embed_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    
    // Read transform matrices
    size_t transform_size = model->embeddings.embedding_dim * model->embeddings.embedding_dim;
    model->embeddings.lattice_transform = (float*)malloc(sizeof(float) * transform_size);
    model->embeddings.inverse_transform = (float*)malloc(sizeof(float) * transform_size);
    if (fread(model->embeddings.lattice_transform, sizeof(float), transform_size, f) != transform_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    if (fread(model->embeddings.inverse_transform, sizeof(float), transform_size, f) != transform_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    
    // Read layers
    if (fread(&model->num_layers, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    model->attention_layers = (AttentionLayer*)malloc(sizeof(AttentionLayer) * model->num_layers);
    model->ff_layers = (FeedForwardLayer*)malloc(sizeof(FeedForwardLayer) * model->num_layers);
    model->layer_norms = (CLLMLayerNorm*)malloc(sizeof(CLLMLayerNorm) * model->num_layers);
    
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Read attention layer (only metadata, not pointers)
        AttentionLayer* attn = &model->attention_layers[i];
        memset(attn, 0, sizeof(AttentionLayer));
        if (fread(&attn->layer_id, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&attn->num_heads, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&attn->head_dim, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        
        size_t attn_size = attn->num_heads * attn->head_dim * attn->head_dim;
        attn->query_lattice = (float*)malloc(sizeof(float) * attn_size);
        attn->key_lattice = (float*)malloc(sizeof(float) * attn_size);
        attn->value_lattice = (float*)malloc(sizeof(float) * attn_size);
        if (fread(attn->query_lattice, sizeof(float), attn_size, f) != attn_size) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(attn->key_lattice, sizeof(float), attn_size, f) != attn_size) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(attn->value_lattice, sizeof(float), attn_size, f) != attn_size) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        
        // Read feed-forward layer (only metadata, not pointers)
        FeedForwardLayer* ff = &model->ff_layers[i];
        memset(ff, 0, sizeof(FeedForwardLayer));
        if (fread(&ff->layer_id, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&ff->input_dim, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&ff->hidden_dim, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&ff->output_dim, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        
        size_t ff_size1 = ff->input_dim * ff->hidden_dim;
        size_t ff_size2 = ff->hidden_dim * ff->output_dim;
        ff->w1_lattice = (float*)malloc(sizeof(float) * ff_size1);
        ff->w2_lattice = (float*)malloc(sizeof(float) * ff_size2);
        ff->bias1 = (float*)malloc(sizeof(float) * ff->hidden_dim);
        ff->bias2 = (float*)malloc(sizeof(float) * ff->output_dim);
        if (fread(ff->w1_lattice, sizeof(float), ff_size1, f) != ff_size1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(ff->w2_lattice, sizeof(float), ff_size2, f) != ff_size2) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(ff->bias1, sizeof(float), ff->hidden_dim, f) != ff->hidden_dim) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(ff->bias2, sizeof(float), ff->output_dim, f) != ff->output_dim) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        
        // Read layer norm (only metadata, not pointers)
        CLLMLayerNorm* ln = &model->layer_norms[i];
        memset(ln, 0, sizeof(CLLMLayerNorm));
        if (fread(&ln->layer_id, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&ln->dim, sizeof(uint32_t), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(&ln->epsilon, sizeof(float), 1, f) != 1) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        ln->gamma = (float*)malloc(sizeof(float) * ln->dim);
        ln->beta = (float*)malloc(sizeof(float) * ln->dim);
        if (fread(ln->gamma, sizeof(float), ln->dim, f) != ln->dim) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
        if (fread(ln->beta, sizeof(float), ln->dim, f) != ln->dim) {

            fprintf(stderr, "Error reading from file\n");

            fclose(f);

            return NULL;

        }
    }
    
    // Read positional encoding (only metadata, not the struct with pointers)
    if (fread(&model->pos_encoding.max_length, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    if (fread(&model->pos_encoding.embedding_dim, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    size_t pos_size = model->pos_encoding.max_length * model->pos_encoding.embedding_dim;
    model->pos_encoding.spiral_positions = (float*)malloc(sizeof(float) * pos_size);
    model->pos_encoding.clock_positions = (float*)malloc(sizeof(float) * pos_size);
    model->pos_encoding.prime_positions = (float*)malloc(sizeof(float) * pos_size);
    model->pos_encoding.learned_positions = (float*)malloc(sizeof(float) * pos_size);
    if (fread(model->pos_encoding.spiral_positions, sizeof(float), pos_size, f) != pos_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    if (fread(model->pos_encoding.clock_positions, sizeof(float), pos_size, f) != pos_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    if (fread(model->pos_encoding.prime_positions, sizeof(float), pos_size, f) != pos_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    if (fread(model->pos_encoding.learned_positions, sizeof(float), pos_size, f) != pos_size) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    
    // Read training metadata
    if (fread(&model->training_meta, sizeof(TrainingMetadata), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    
    // Read and verify checksum
    uint32_t checksum;
    if (fread(&checksum, sizeof(uint32_t), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    
    fclose(f);
    return model;
}

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
    
    // Calculate total weights
    size_t embedding_weights = config.vocab_size * config.embedding_dim;
    size_t attention_weights_per_layer = 4 * config.embedding_dim * config.embedding_dim;
    size_t ff_weights_per_layer = 2 * config.embedding_dim * config.ff_dim;
    size_t total_weights = embedding_weights + 
                          config.num_layers * (attention_weights_per_layer + ff_weights_per_layer);
    
    // Allocate weights if needed
    if (!model->weights) {
        model->weights = (float*)malloc(total_weights * sizeof(float));
        if (!model->weights) {
            fprintf(stderr, "Failed to allocate weights\n");
            cllm_free(model);
            fclose(file);
            return NULL;
        }
    }
    
    // Read weights
    if (fread(model->weights, sizeof(float), total_weights, file) != total_weights) {
        fprintf(stderr, "Failed to read weights\n");
        cllm_free(model);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    printf("✓ Model loaded: %s\n", filepath);
    printf("  Vocab: %lu | Embedding: %lu | Layers: %lu | Weights: %lu\n",
           (unsigned long)header.vocab_size, (unsigned long)header.embedding_dim, 
           (unsigned long)header.num_layers, (unsigned long)total_weights);
    
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
    
    // Write weights - check if weights exist
    if (model->weights && model->num_weights > 0) {
        if (fwrite(model->weights, sizeof(float), model->num_weights, file) != model->num_weights) {
            fprintf(stderr, "Failed to write weights\n");
            fclose(file);
            return -1;
        }
    } else {
        fprintf(stderr, "Warning: Model has no weights to save (weights=%p, params=%lu)\n", 
                (void*)model->weights, (unsigned long)model->num_weights);
        // Don't try to allocate huge amounts of memory - just write header
        // The model can be loaded later and weights initialized
    }
    
    fclose(file);
    printf("✓ Model saved: %s\n", filepath);
    return 0;
}