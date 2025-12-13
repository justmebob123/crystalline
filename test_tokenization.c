#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_inference.h"

int main() {
    printf("=== Tokenization Test ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    // Create inference context
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to create inference context\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Test tokenization
    const char* test_strings[] = {
        "Hello",
        "World",
        "Test",
        "ABC",
        ""
    };
    
    for (int i = 0; i < 5; i++) {
        uint32_t tokens[100];
        int num_tokens = cllm_tokenize(inference, test_strings[i], tokens, 100);
        
        printf("Input: '%s'\n", test_strings[i]);
        printf("  Tokens (%d): ", num_tokens);
        for (int j = 0; j < num_tokens; j++) {
            printf("%u ", tokens[j]);
        }
        printf("\n");
        
        // Detokenize back
        char output[1000];
        cllm_detokenize(inference, tokens, num_tokens, output, 1000);
        printf("  Detokenized: '%s'\n", output);
        printf("  Match: %s\n\n", strcmp(test_strings[i], output) == 0 ? "YES" : "NO");
    }
    
    // Test with added tokens
    printf("\n=== Testing token addition ===\n");
    uint32_t tokens[100];
    int num_tokens = cllm_tokenize(inference, "Hello", tokens, 100);
    printf("Original: ");
    for (int i = 0; i < num_tokens; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n");
    
    // Add token 452 (the one with highest probability)
    tokens[num_tokens++] = 452;
    printf("After adding token 452: ");
    for (int i = 0; i < num_tokens; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n");
    
    // Detokenize
    char output[1000];
    cllm_detokenize(inference, tokens, num_tokens, output, 1000);
    printf("Detokenized: '%s'\n", output);
    
    // Cleanup
    extern void cllm_inference_cleanup(CLLMInference* inference);
    cllm_inference_cleanup(inference);
    cllm_free_model(model);
    
    return 0;
}