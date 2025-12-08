// TEMPORARY STUB FILE - cllm_format_stub.c
// These are minimal implementations to allow compilation
// TODO: Properly integrate with new CLLMModel structure

#include "cllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stub: Write model to file
int cllm_write_model(const CLLMModel* model, const char* filename) {
    if (!model || !filename) return -1;
    
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file for writing: %s\n", filename);
        return -1;
    }
    
    // Write basic header
    fwrite("CLLM", 4, 1, f);
    uint32_t version = 1;
    fwrite(&version, sizeof(uint32_t), 1, f);
    
    // Write basic model info
    fwrite(&model->vocab_size, sizeof(uint32_t), 1, f);
    fwrite(&model->embedding_dim, sizeof(uint32_t), 1, f);
    fwrite(&model->hidden_dim, sizeof(uint32_t), 1, f);
    fwrite(&model->num_layers, sizeof(uint32_t), 1, f);
    fwrite(&model->num_heads, sizeof(uint32_t), 1, f);
    fwrite(&model->max_seq_len, sizeof(uint32_t), 1, f);
    
    // TODO: Write actual model weights
    
    fclose(f);
    printf("Model written to %s (stub implementation)\n", filename);
    return 0;
}

// Stub: Read model from file
CLLMModel* cllm_read_model(const char* filename) {
    if (!filename) return NULL;
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file for reading: %s\n", filename);
        return NULL;
    }
    
    // Read header
    char magic[4];
    fread(magic, 4, 1, f);
    if (memcmp(magic, "CLLM", 4) != 0) {
        fprintf(stderr, "Error: Invalid model file format\n");
        fclose(f);
        return NULL;
    }
    
    uint32_t version;
    fread(&version, sizeof(uint32_t), 1, f);
    
    // Read basic model info
    CLLMConfig config = {0};
    fread(&config.vocab_size, sizeof(uint32_t), 1, f);
    fread(&config.embedding_dim, sizeof(uint32_t), 1, f);
    fread(&config.hidden_dim, sizeof(uint32_t), 1, f);
    fread(&config.num_layers, sizeof(uint32_t), 1, f);
    fread(&config.num_heads, sizeof(uint32_t), 1, f);
    fread(&config.max_seq_len, sizeof(uint32_t), 1, f);
    
    fclose(f);
    
    // Create model with default config
    config.solid_type = PLATONIC_CUBE;
    CLLMModel* model = cllm_create_model(&config);
    
    // TODO: Load actual model weights
    
    printf("Model loaded from %s (stub implementation)\n", filename);
    return model;
}

// Stub: Free model (alias for cllm_free_model)
void cllm_free(CLLMModel* model) {
    cllm_free_model(model);
}