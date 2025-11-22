#include "cllm.h"
#include "cllm_format.h"
#include "cllm_utils.h"
#include "cllm_inference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Testing All CLLM Operations ===\n\n");
    
    // Test 1: Create Model
    printf("Test 1: Creating model...\n");
    CLLMConfig config = {
        .vocab_size = 1000,  // Smaller for faster testing
        .embedding_dim = 128,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 128,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "FAILED: Model creation\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Test 2: Save Model
    printf("Test 2: Saving model...\n");
    system("mkdir -p test_models");
    int save_result = cllm_write_model(model, "test_models/test.cllm");
    if (save_result != 0) {
        fprintf(stderr, "FAILED: Model save\n");
        return 1;
    }
    printf("✓ Model saved\n\n");
    
    // Test 3: Load Model
    printf("Test 3: Loading model...\n");
    CLLMModel* loaded_model = cllm_read_model("test_models/test.cllm");
    if (!loaded_model) {
        fprintf(stderr, "FAILED: Model load\n");
        return 1;
    }
    printf("✓ Model loaded\n\n");
    
    // Test 4: Create Inference Context
    printf("Test 4: Creating inference context...\n");
    CLLMInference* inference = cllm_inference_init(loaded_model);
    if (!inference) {
        fprintf(stderr, "FAILED: Inference context creation\n");
        return 1;
    }
    printf("✓ Inference context created\n\n");
    
    // Test 5: Generate Text
    printf("Test 5: Generating text...\n");
    char output[1024];
    int gen_result = cllm_generate(inference, "test", output, sizeof(output));
    if (gen_result < 0) {
        fprintf(stderr, "FAILED: Text generation\n");
        return 1;
    }
    printf("✓ Text generated: %s\n\n", output);
    
    // Test 6: Cleanup
    printf("Test 6: Cleaning up...\n");
    cllm_inference_cleanup(inference);
    cllm_free_model(loaded_model);
    cllm_free_model(model);
    printf("✓ Cleanup complete\n\n");
    
    printf("=== ALL TESTS PASSED ===\n");
    return 0;
}