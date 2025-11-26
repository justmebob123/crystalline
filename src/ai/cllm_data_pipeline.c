#include "cllm_training.h"
#include "cllm_tokenizer.h"
#include "cllm_threads.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

typedef struct {
    char** files;
    int num_files;
    int current_file;
    FILE* current_fp;
    pthread_mutex_t mutex;
} DataLoader;

typedef struct {
    uint32_t* input_ids;
    uint32_t* target_ids;
    float* attention_mask;
    uint32_t seq_len;
    uint32_t batch_size;
} Batch;

// Create data loader
DataLoader* data_loader_create(char** files, int num_files) {
    DataLoader* loader = calloc(1, sizeof(DataLoader));
    loader->files = files;
    loader->num_files = num_files;
    loader->current_file = 0;
    loader->current_fp = NULL;
    pthread_mutex_init(&loader->mutex, NULL);
    return loader;
}

// Load next batch
Batch* data_loader_next_batch(DataLoader* loader, CLLMTokenizer* tokenizer, 
                               uint32_t batch_size, uint32_t seq_len) {
    pthread_mutex_lock(&loader->mutex);
    
    if (!loader->current_fp) {
        if (loader->current_file >= loader->num_files) {
            pthread_mutex_unlock(&loader->mutex);
            return NULL;
        }
        loader->current_fp = fopen(loader->files[loader->current_file], "r");
        loader->current_file++;
    }
    
    Batch* batch = calloc(1, sizeof(Batch));
    batch->batch_size = batch_size;
    batch->seq_len = seq_len;
    batch->input_ids = calloc(batch_size * seq_len, sizeof(uint32_t));
    batch->target_ids = calloc(batch_size * seq_len, sizeof(uint32_t));
    batch->attention_mask = calloc(batch_size * seq_len, sizeof(float));
    
    char line[4096];
    for (uint32_t i = 0; i < batch_size; i++) {
        if (!fgets(line, sizeof(line), loader->current_fp)) {
            fclose(loader->current_fp);
            loader->current_fp = NULL;
            if (i == 0) {
                free(batch->input_ids);
                free(batch->target_ids);
                free(batch->attention_mask);
                free(batch);
                pthread_mutex_unlock(&loader->mutex);
                return NULL;
            }
            break;
        }
        
        // Tokenize
        uint32_t num_tokens;
        uint32_t* tokens = cllm_tokenizer_encode(tokenizer, line, &num_tokens);
        
        // Copy to batch (truncate or pad)
        uint32_t copy_len = (num_tokens < seq_len) ? num_tokens : seq_len;
        memcpy(&batch->input_ids[i * seq_len], tokens, copy_len * sizeof(uint32_t));
        
        // Target is input shifted by 1
        if (copy_len > 0) {
            memcpy(&batch->target_ids[i * seq_len], &tokens[1], (copy_len - 1) * sizeof(uint32_t));
        }
        
        // Attention mask
        for (uint32_t j = 0; j < copy_len; j++) {
            batch->attention_mask[i * seq_len + j] = 1.0f;
        }
        
        free(tokens);
    }
    
    pthread_mutex_unlock(&loader->mutex);
    return batch;
}

// Free batch
void batch_free(Batch* batch) {
    if (!batch) return;
    free(batch->input_ids);
    free(batch->target_ids);
    free(batch->attention_mask);
    free(batch);
}

// Free data loader
void data_loader_free(DataLoader* loader) {
    if (!loader) return;
    if (loader->current_fp) fclose(loader->current_fp);
    pthread_mutex_destroy(&loader->mutex);
    free(loader);
}

// Training pipeline with threading
typedef struct {
    ThreadSystem* threads;
    DataLoader* loader;
    CLLMTokenizer* tokenizer;
    CLLM* model;
    uint32_t batch_size;
    uint32_t seq_len;
    int num_epochs;
    float learning_rate;
} TrainingPipeline;

TrainingPipeline* pipeline_create(char** data_files, int num_files, 
                                  CLLM* model, int num_threads) {
    TrainingPipeline* pipeline = calloc(1, sizeof(TrainingPipeline));
    
    pipeline->threads = threads_create(2); // 2 levels: root + 12 workers
    pipeline->loader = data_loader_create(data_files, num_files);
    pipeline->tokenizer = cllm_create_tokenizer(50000);
    pipeline->model = model;
    pipeline->batch_size = 32;
    pipeline->seq_len = 512;
    pipeline->num_epochs = 10;
    pipeline->learning_rate = 0.0001f;
    
    return pipeline;
}

void pipeline_train(TrainingPipeline* pipeline) {
    threads_start(pipeline->threads);
    
    for (int epoch = 0; epoch < pipeline->num_epochs; epoch++) {
        printf("Epoch %d/%d\n", epoch + 1, pipeline->num_epochs);
        
        int batch_idx = 0;
        while (1) {
            Batch* batch = data_loader_next_batch(pipeline->loader, 
                                                  pipeline->tokenizer,
                                                  pipeline->batch_size,
                                                  pipeline->seq_len);
            if (!batch) break;
            
            // Distribute to threads
            uint64_t work_item = batch_idx;
            threads_distribute_work(pipeline->threads, &work_item, 1);
            
            // Process batch (forward + backward)
            // TODO: Actual training step
            
            batch_free(batch);
            batch_idx++;
        }
        
        printf("Processed %d batches\n", batch_idx);
    }
    
    threads_stop(pipeline->threads);
}

void pipeline_free(TrainingPipeline* pipeline) {
    if (!pipeline) return;
    threads_free(pipeline->threads);
    data_loader_free(pipeline->loader);
    cllm_free_tokenizer(pipeline->tokenizer);
    free(pipeline);
}