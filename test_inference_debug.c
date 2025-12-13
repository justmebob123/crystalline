#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_inference.h"

// External functions
extern int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length);
extern void cllm_set_temperature(CLLMInference* inference, float temperature);
extern void cllm_set_max_tokens(CLLMInference* inference, int max_tokens);
extern void cllm_inference_cleanup(CLLMInference* inference);

int main() {
    printf("=== Debug Inference Test ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n");
    
    // Load vocabulary
    if (!cllm_load_vocabulary(model, "test_checkpoints/vocab.txt")) {
        fprintf(stderr, "Failed to load vocabulary\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Vocabulary loaded\n");
    
    // Create inference context
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to create inference context\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Inference context created\n");
    
    // Set parameters
    cllm_set_temperature(inference, 0.8);
    cllm_set_max_tokens(inference, 10);  // Generate just 10 tokens for testing
    printf("✓ Parameters set (temp=0.8, max_tokens=10)\n\n");
    
    // Generate
    char output[4096];
    printf("Generating from prompt: 'Hello'\n");
    int tokens_generated = cllm_generate(inference, "Hello", output, sizeof(output));
    
    printf("\nTokens generated: %d\n", tokens_generated);
    printf("Output: '%s'\n", output);
    printf("Output length: %zu\n", strlen(output));
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_free_model(model);
    
    return 0;
}