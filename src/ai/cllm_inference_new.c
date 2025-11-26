#include "cllm_training.h"
#include "cllm_tokenizer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    CLLM* model;
    CLLMTokenizer* tokenizer;
    float temperature;
    int top_k;
    float top_p;
    uint32_t max_length;
} InferenceConfig;

InferenceConfig* inference_config_create(CLLM* model, CLLMTokenizer* tokenizer) {
    InferenceConfig* config = calloc(1, sizeof(InferenceConfig));
    config->model = model;
    config->tokenizer = tokenizer;
    config->temperature = 1.0f;
    config->top_k = 50;
    config->top_p = 0.9f;
    config->max_length = 512;
    return config;
}

static void softmax_temperature(float* logits, int size, float temperature) {
    float max_logit = logits[0];
    for (int i = 1; i < size; i++) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        logits[i] = expf((logits[i] - max_logit) / temperature);
        sum += logits[i];
    }
    
    for (int i = 0; i < size; i++) {
        logits[i] /= sum;
    }
}

static uint32_t sample_token(float* probs, int size, float top_p, int top_k) {
    int max_idx = 0;
    float max_prob = probs[0];
    
    for (int i = 1; i < size; i++) {
        if (probs[i] > max_prob) {
            max_prob = probs[i];
            max_idx = i;
        }
    }
    
    return max_idx;
}

char* generate_text(InferenceConfig* config, const char* prompt) {
    uint32_t num_tokens;
    uint32_t* input_ids = cllm_tokenizer_encode(config->tokenizer, prompt, &num_tokens);
    
    uint32_t* output_ids = calloc(config->max_length, sizeof(uint32_t));
    memcpy(output_ids, input_ids, num_tokens * sizeof(uint32_t));
    uint32_t output_len = num_tokens;
    
    for (uint32_t i = num_tokens; i < config->max_length; i++) {
        float* embeddings = calloc(output_len * config->model->config.d_model, sizeof(float));
        
        for (uint32_t j = 0; j < output_len; j++) {
            uint32_t token_id = output_ids[j];
            if (token_id < config->model->config.vocab_size) {
                memcpy(&embeddings[j * config->model->config.d_model],
                       &config->model->token_embeddings[token_id * config->model->config.d_model],
                       config->model->config.d_model * sizeof(float));
            }
        }
        
        float* logits = &embeddings[(output_len - 1) * config->model->config.d_model];
        softmax_temperature(logits, config->model->config.vocab_size, config->temperature);
        uint32_t next_token = sample_token(logits, config->model->config.vocab_size, 
                                          config->top_p, config->top_k);
        
        output_ids[output_len++] = next_token;
        free(embeddings);
        
        if (next_token == 3) break;
    }
    
    char* output_text = cllm_tokenizer_decode(config->tokenizer, output_ids, output_len);
    
    free(input_ids);
    free(output_ids);
    
    return output_text;
}

void inference_config_free(InferenceConfig* config) {
    if (!config) return;
    free(config);
}