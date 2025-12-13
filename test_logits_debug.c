#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_inference.h"

// External functions
extern int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length);
extern void cllm_inference_cleanup(CLLMInference* inference);
extern void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens);

int main() {
    printf("=== Logits Debug Test ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded (vocab_size=%u)\n", model->vocab_size);
    
    // Create inference context
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to create inference context\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Inference context created\n\n");
    
    // Tokenize a simple prompt
    uint32_t tokens[10];
    const char* prompt = "Hello";
    int num_tokens = cllm_tokenize(inference, prompt, tokens, 10);
    printf("Tokenized '%s' into %d tokens: ", prompt, num_tokens);
    for (int i = 0; i < num_tokens; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n\n");
    
    // Do forward pass
    printf("Running forward pass...\n");
    cllm_forward(inference, tokens, num_tokens);
    
    // Check logits
    printf("\nLogits analysis:\n");
    double max_logit = -1e9;
    double min_logit = 1e9;
    int max_idx = -1;
    int nan_count = 0;
    int inf_count = 0;
    
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        if (isnan(inference->logits[i])) {
            nan_count++;
        } else if (isinf(inference->logits[i])) {
            inf_count++;
        } else {
            if (inference->logits[i] > max_logit) {
                max_logit = inference->logits[i];
                max_idx = i;
            }
            if (inference->logits[i] < min_logit) {
                min_logit = inference->logits[i];
            }
        }
    }
    
    printf("  NaN logits: %d\n", nan_count);
    printf("  Inf logits: %d\n", inf_count);
    printf("  Max logit: %.6f (token %d)\n", max_logit, max_idx);
    printf("  Min logit: %.6f\n", min_logit);
    printf("  First 10 logits: ");
    for (int i = 0; i < 10 && i < model->vocab_size; i++) {
        printf("%.4f ", inference->logits[i]);
    }
    printf("\n");
    
    // Check if all logits are the same (which would explain why it outputs the same token)
    int all_same = 1;
    double first_valid_logit = inference->logits[0];
    for (uint32_t i = 1; i < model->vocab_size; i++) {
        if (!isnan(inference->logits[i]) && !isinf(inference->logits[i])) {
            if (fabs(inference->logits[i] - first_valid_logit) > 1e-6) {
                all_same = 0;
                break;
            }
        }
    }
    
    if (all_same) {
        printf("\n⚠️  WARNING: All logits are the same! This explains why generation doesn't work.\n");
    }
    
    // Apply softmax and check probabilities
    printf("\nApplying softmax...\n");
    extern void cllm_softmax(double* logits, int vocab_size);
    cllm_softmax(inference->logits, (int)model->vocab_size);
    
    printf("  First 10 probabilities: ");
    for (int i = 0; i < 10 && i < model->vocab_size; i++) {
        printf("%.6f ", inference->logits[i]);
    }
    printf("\n");
    
    double max_prob = -1;
    int max_prob_idx = -1;
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        if (inference->logits[i] > max_prob) {
            max_prob = inference->logits[i];
            max_prob_idx = i;
        }
    }
    printf("  Max probability: %.6f (token %d)\n", max_prob, max_prob_idx);
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_free_model(model);
    
    return 0;
}