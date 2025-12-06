#include "include/cllm.h"
#include "include/cllm_inference.h"
#include "include/cllm_format.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Testing Final Model Inference ===\n\n");
    
    // Load model
    printf("Loading model...\n");
    CLLMModel* model = cllm_read_model("checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded: vocab=%lu, embed_dim=%u, layers=%u\n\n", 
           model->vocab_size, model->embeddings.embedding_dim, model->num_layers);
    
    // Initialize inference
    CLLMInference* inf = cllm_inference_init(model);
    if (!inf) {
        fprintf(stderr, "Failed to init inference\n");
        cllm_free(model);
        return 1;
    }
    
    inf->temperature = 0.8;
    inf->top_k = 40;
    inf->max_tokens = 50;
    
    printf("Testing prompts from training data:\n");
    printf("====================================\n\n");
    
    const char* prompts[] = {
        "The sky is",
        "Prime numbers are",
        "Water is",
        "The sun",
        "A journey of",
        "Knowledge is"
    };
    
    for (int i = 0; i < 6; i++) {
        printf("Prompt: &quot;%s&quot;\n", prompts[i]);
        
        char output[2048] = {0};
        int result = cllm_generate(inf, prompts[i], output, sizeof(output));
        
        if (result >= 0) {
            printf("Output: %s\n", output);
            printf("Tokens generated: %d\n\n", result);
        } else {
            printf("Generation failed\n\n");
        }
    }
    
    cllm_inference_cleanup(inf);
    cllm_free(model);
    
    printf("=== Test Complete ===\n");
    return 0;
}