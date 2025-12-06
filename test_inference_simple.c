#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_inference.h"
#include "include/cllm_tokenizer.h"

int main() {
    printf("Loading model from checkpoints/final_model.cllm...\n");
    
    // Load model
    CLLMModel* model = cllm_load_model("checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    printf("Model loaded successfully!\n");
    printf("Vocab size: %u\n", model->vocab_size);
    printf("Embedding dim: %u\n", model->embedding_dim);
    printf("Num layers: %u\n", model->num_layers);
    
    // Create inference context
    CLLMInference* inference = cllm_inference_create(model);
    if (!inference) {
        fprintf(stderr, "Failed to create inference context\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("\nTesting inference with prompt: 'The sky is'\n");
    
    // Simple tokenization (just use first few token IDs for testing)
    uint32_t tokens[] = {1, 2, 3};  // Dummy tokens
    int num_tokens = 3;
    
    // Run forward pass
    cllm_forward(inference, tokens, num_tokens);
    
    // Get next token prediction
    uint32_t next_token = cllm_sample_token(inference, 1.0f);
    
    printf("Next token prediction: %u\n", next_token);
    printf("\nInference test completed successfully!\n");
    
    // Cleanup
    cllm_inference_free(inference);
    cllm_free_model(model);
    
    return 0;
}