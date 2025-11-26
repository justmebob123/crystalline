#include "cllm_training.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

typedef struct {
    char* checkpoint_dir;
    int max_checkpoints;
    int save_frequency;
} CheckpointConfig;

CheckpointConfig* checkpoint_config_create(const char* dir, int max_keep, int frequency) {
    CheckpointConfig* config = calloc(1, sizeof(CheckpointConfig));
    config->checkpoint_dir = strdup(dir);
    config->max_checkpoints = max_keep;
    config->save_frequency = frequency;
    
    mkdir(dir, 0755);
    
    return config;
}

// Save model checkpoint
int save_checkpoint(CLLM* model, const char* path, int epoch, float loss) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open checkpoint file: %s\n", path);
        return -1;
    }
    
    // Write header
    fwrite("CLLM", 4, 1, fp);
    uint32_t version = 1;
    fwrite(&version, sizeof(uint32_t), 1, fp);
    
    // Write metadata
    fwrite(&epoch, sizeof(int), 1, fp);
    fwrite(&loss, sizeof(float), 1, fp);
    time_t timestamp = time(NULL);
    fwrite(&timestamp, sizeof(time_t), 1, fp);
    
    // Write config
    fwrite(&model->config.vocab_size, sizeof(uint32_t), 1, fp);
    fwrite(&model->config.d_model, sizeof(uint32_t), 1, fp);
    fwrite(&model->config.n_layers, sizeof(uint32_t), 1, fp);
    fwrite(&model->config.n_heads, sizeof(uint32_t), 1, fp);
    fwrite(&model->config.d_ff, sizeof(uint32_t), 1, fp);
    fwrite(&model->config.max_seq_len, sizeof(uint32_t), 1, fp);
    
    // Write embeddings
    size_t embed_size = model->config.vocab_size * model->config.d_model;
    fwrite(model->token_embeddings, sizeof(float), embed_size, fp);
    
    // Write layer weights (simplified - would need full implementation)
    for (uint32_t i = 0; i < model->config.n_layers; i++) {
        // Write attention weights
        size_t attn_size = model->config.d_model * model->config.d_model;
        // fwrite(layer_weights, sizeof(float), attn_size, fp);
    }
    
    fclose(fp);
    printf("Checkpoint saved: %s (epoch %d, loss %.4f)\n", path, epoch, loss);
    return 0;
}

// Load model checkpoint
int load_checkpoint(CLLM* model, const char* path, int* epoch, float* loss) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open checkpoint file: %s\n", path);
        return -1;
    }
    
    // Read header
    char magic[4];
    fread(magic, 4, 1, fp);
    if (memcmp(magic, "CLLM", 4) != 0) {
        fprintf(stderr, "Invalid checkpoint file\n");
        fclose(fp);
        return -1;
    }
    
    uint32_t version;
    fread(&version, sizeof(uint32_t), 1, fp);
    
    // Read metadata
    fread(epoch, sizeof(int), 1, fp);
    fread(loss, sizeof(float), 1, fp);
    time_t timestamp;
    fread(&timestamp, sizeof(time_t), 1, fp);
    
    // Read config
    CLLMConfig config;
    fread(&config.vocab_size, sizeof(uint32_t), 1, fp);
    fread(&config.d_model, sizeof(uint32_t), 1, fp);
    fread(&config.n_layers, sizeof(uint32_t), 1, fp);
    fread(&config.n_heads, sizeof(uint32_t), 1, fp);
    fread(&config.d_ff, sizeof(uint32_t), 1, fp);
    fread(&config.max_seq_len, sizeof(uint32_t), 1, fp);
    
    // Verify config matches
    if (config.vocab_size != model->config.vocab_size ||
        config.d_model != model->config.d_model ||
        config.n_layers != model->config.n_layers) {
        fprintf(stderr, "Checkpoint config mismatch\n");
        fclose(fp);
        return -1;
    }
    
    // Read embeddings
    size_t embed_size = model->config.vocab_size * model->config.d_model;
    fread(model->token_embeddings, sizeof(float), embed_size, fp);
    
    // Read layer weights
    for (uint32_t i = 0; i < model->config.n_layers; i++) {
        // Read attention weights
        // fread(layer_weights, sizeof(float), attn_size, fp);
    }
    
    fclose(fp);
    printf("Checkpoint loaded: %s (epoch %d, loss %.4f)\n", path, *epoch, *loss);
    return 0;
}

// Save checkpoint with rotation
int save_checkpoint_with_rotation(CLLM* model, CheckpointConfig* config, 
                                  int epoch, float loss) {
    char path[512];
    snprintf(path, sizeof(path), "%s/checkpoint_epoch_%d.bin", 
             config->checkpoint_dir, epoch);
    
    int result = save_checkpoint(model, path, epoch, loss);
    
    // TODO: Implement checkpoint rotation (delete old checkpoints)
    
    return result;
}

void checkpoint_config_free(CheckpointConfig* config) {
    if (!config) return;
    free(config->checkpoint_dir);
    free(config);
}