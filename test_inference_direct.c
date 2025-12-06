/**
 * Direct Inference Test
 * 
 * Tests inference by directly loading a trained model
 */

#include "include/cllm.h"
#include "include/cllm_inference.h"
#include "include/cllm_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    printf("=== CLLM Direct Inference Test ===\n\n");
    
    const char* model_path = "checkpoints/final_model.cllm";
    const char* prompt = "The sky is";
    int max_tokens = 20;
    
    if (argc > 1) {
        model_path = argv[1];
    }
    if (argc > 2) {
        prompt = argv[2];
    }
    if (argc > 3) {
        max_tokens = atoi(argv[3]);
    }
    
    printf("Loading model from: %s\n", model_path);
    printf("Prompt: &quot;%s&quot;\n", prompt);
    printf("Max tokens: %d\n\n", max_tokens);
    
    // Load the trained model
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "Failed to load model from %s\n", model_path);
        return 1;
    }
    
    printf("Model loaded successfully!\n");
    printf("  Vocab size: %lu\n", model->vocab_size);
    printf("  Embedding dim: %u\n", model->embeddings.embedding_dim);
    printf("  Num layers: %u\n", model->num_layers);
    printf("  Num heads: %u\n", model->header.num_heads);
    printf("\n");
    
    // Initialize inference context
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to initialize inference context\n");
        cllm_free(model);
        return 1;
    }
    
    // Set inference parameters
    inference->temperature = 0.8f;
    inference->top_k = 40;
    inference->max_tokens = max_tokens;
    
    printf("Inference context initialized\n");
    printf("  Temperature: %.2f\n", inference->temperature);
    printf("  Top-k: %d\n", inference->top_k);
    printf("\n");
    
    // Tokenize the prompt
    uint32_t tokens[512];
    int num_tokens = cllm_tokenize(inference, prompt, tokens, 512);
    
    if (num_tokens <= 0) {
        fprintf(stderr, "Failed to tokenize prompt\n");
        cllm_inference_cleanup(inference);
        cllm_free(model);
        return 1;
    }
    
    printf("Tokenized prompt: %d tokens\n", num_tokens);
    printf("Tokens: ");
    for (int i = 0; i < num_tokens; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n\n");
    
    // Generate text
    printf("Generating text...\n");
    printf("Output: %s", prompt);
    fflush(stdout);
    
    char output[4096] = {0};
    int result = cllm_generate(inference, prompt, output, sizeof(output));
    
    if (result < 0) {
        fprintf(stderr, "\nFailed to generate text (error code: %d)\n", result);
        cllm_inference_cleanup(inference);
        cllm_free(model);
        return 1;
    }
    
    printf("%s\n", output);
    printf("\nGenerated %d tokens\n", result);
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_free(model);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}