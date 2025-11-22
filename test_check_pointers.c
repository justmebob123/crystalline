#include <stdio.h>
#include <stdlib.h>
#include "cllm.h"
#include "cllm_utils.h"

int main() {
    printf("Creating CLLM model...\n");
    CLLMModel* model = cllm_create_small_model();
    
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    printf("Model created successfully!\n");
    printf("Checking pointers:\n");
    printf("  model->tokens = %p\n", (void*)model->tokens);
    printf("  model->embeddings.embeddings = %p\n", (void*)model->embeddings.embeddings);
    printf("  model->lattice_points = %p\n", (void*)model->lattice_points);
    printf("  model->weights = %p\n", (void*)model->weights);
    
    return 0;
}
