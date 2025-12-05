/**
 * Minimal Debug Test - Find Where It Hangs
 */

#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"

// Forward declarations to avoid implicit function warnings
extern CLLMModel* cllm_create_small_model(void);
extern void cllm_free_model(CLLMModel* model);

int main() {
    printf("=== Minimal Debug Test ===\n");
    printf("Step 1: About to call cllm_create_small_model()\n");
    fflush(stdout);
    
    CLLMModel* model = cllm_create_small_model();
    
    printf("Step 2: cllm_create_small_model() returned: %p\n", (void*)model);
    fflush(stdout);
    
    if (!model) {
        fprintf(stderr, "ERROR: Model is NULL\n");
        return 1;
    }
    
    printf("Step 3: Model created successfully\n");
    printf("  Checking model structure...\n");
    fflush(stdout);
    
    // Safely check if we can access model fields
    printf("  Vocab size: %lu\n", (unsigned long)model->vocab_size);
    fflush(stdout);
    
    printf("  Embedding dim: %lu\n", (unsigned long)model->embedding_dim);
    fflush(stdout);
    
    printf("  Layers: %u\n", model->num_layers);
    fflush(stdout);
    
    printf("Step 4: About to call cllm_free_model()\n");
    fflush(stdout);
    
    cllm_free_model(model);
    
    printf("Step 5: Model freed successfully\n");
    printf("=== Test Complete ===\n");
    
    return 0;
}