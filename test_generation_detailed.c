#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_inference.h"

#define MAX_SEQUENCE_LENGTH 512

int main() {
    printf("=== Detailed Generation Test ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n");
    
    // Create inference context
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to create inference context\n");
        cllm_free_model(model);
        return 1;
    }
    inference->temperature = 0.8;
    inference->max_tokens = 10;
    printf("✓ Inference context created (temp=0.8, max_tokens=10)\n\n");
    
    // Tokenize prompt
    const char* prompt = "Hello";
    uint32_t tokens[MAX_SEQUENCE_LENGTH];
    int num_tokens = cllm_tokenize(inference, prompt, tokens, MAX_SEQUENCE_LENGTH);
    
    printf("Prompt: '%s'\n", prompt);
    printf("Initial tokens (%d): ", num_tokens);
    for (int i = 0; i < num_tokens; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n\n");
    
    // Manual generation loop with detailed output
    printf("=== Generation Loop ===\n");
    extern void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens);
    extern void cllm_apply_temperature(double* logits, int vocab_size, double temperature);
    extern void cllm_softmax(double* logits, int vocab_size);
    extern uint32_t cllm_sample_top_p(double* probs, int vocab_size, double p);
    
    int tokens_generated = 0;
    while (tokens_generated < inference->max_tokens && num_tokens < MAX_SEQUENCE_LENGTH) {
        printf("\nStep %d:\n", tokens_generated + 1);
        
        // Forward pass
        cllm_forward(inference, tokens, num_tokens);
        
        // Find max logit before temperature
        double max_logit = inference->logits[0];
        int max_idx = 0;
        for (uint32_t i = 1; i < model->vocab_size; i++) {
            if (inference->logits[i] > max_logit) {
                max_logit = inference->logits[i];
                max_idx = i;
            }
        }
        printf("  Max logit: %.4f (token %d)\n", max_logit, max_idx);
        
        // Apply temperature
        cllm_apply_temperature(inference->logits, (int)model->vocab_size, (double)inference->temperature);
        
        // Softmax
        cllm_softmax(inference->logits, (int)model->vocab_size);
        
        // Sample next token
        uint32_t next_token = cllm_sample_top_p(inference->logits, (int)model->vocab_size, 0.9);
        printf("  Sampled token: %u\n", next_token);
        
        // Detokenize just this token
        char token_char[10];
        uint32_t single_token[1] = {next_token};
        cllm_detokenize(inference, single_token, 1, token_char, 10);
        printf("  Character: '%s'\n", token_char);
        
        // Add to sequence
        tokens[num_tokens++] = next_token;
        tokens_generated++;
    }
    
    printf("\n=== Final Result ===\n");
    printf("Total tokens: %d\n", num_tokens);
    printf("All tokens: ");
    for (int i = 0; i < num_tokens; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n");
    
    // Detokenize full sequence
    char output[4096];
    cllm_detokenize(inference, tokens, num_tokens, output, 4096);
    printf("Full output: '%s'\n", output);
    printf("Output length: %zu\n", strlen(output));
    
    // Cleanup
    extern void cllm_inference_cleanup(CLLMInference* inference);
    cllm_inference_cleanup(inference);
    cllm_free_model(model);
    
    return 0;
}