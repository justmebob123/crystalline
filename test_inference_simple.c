#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_inference.h"
#include "cllm_tokenizer.h"

extern CLLMModel* cllm_read_model(const char* filename);
extern void cllm_free_model(CLLMModel* model);

int main() {
    printf("=== Testing Inference with Trained Model ===\n\n");
    
    // Load model
    printf("Loading model from checkpoints/final_model.cllm...\n");
    CLLMModel* model = cllm_read_model("checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n");
    printf("  Vocab size: %u\n", model->vocab_size);
    printf("  Embed dim: %u\n", model->embedding_dim);
    printf("  Layers: %u\n", model->num_layers);
    printf("\n");
    
    // Create inference
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to create inference\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Inference created\n\n");
    
    // Create tokenizer
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(model->vocab_size);
    
    // Test prompts
    const char* prompts[] = {
        "The sky is",
        "The grass is",
        "Water is"
    };
    
    for (int p = 0; p < 3; p++) {
        printf("Prompt: &quot;%s&quot;\n", prompts[p]);
        
        // Tokenize
        uint32_t num_tokens = 0;
        uint32_t* tokens = cllm_tokenizer_encode(tokenizer, prompts[p], &num_tokens);
        
        printf("  Tokens: ");
        for (uint32_t i = 0; i < num_tokens; i++) {
            printf("%u ", tokens[i]);
        }
        printf("\n");
        
        // Generate
        printf("  Generating 5 tokens...\n  Output tokens: ");
        for (int i = 0; i < 5; i++) {
            uint32_t next_token = cllm_inference_generate(inference, tokens, num_tokens, 1)[0];
            printf("%u ", next_token);
            
            // Add to context
            uint32_t* new_tokens = (uint32_t*)realloc(tokens, (num_tokens + 1) * sizeof(uint32_t));
            if (new_tokens) {
                tokens = new_tokens;
                tokens[num_tokens] = next_token;
                num_tokens++;
            }
        }
        printf("\n\n");
        
        free(tokens);
    }
    
    cllm_inference_cleanup(inference);
    cllm_free_model(model);
    cllm_free_tokenizer(tokenizer);
    
    printf("✓ Inference test complete\n");
    return 0;
}