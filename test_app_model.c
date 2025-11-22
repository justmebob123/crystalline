#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_utils.h"
#include "cllm_format.h"

// Declare the app function
extern CLLMModel* app_create_cllm_model_default(void);

int main() {
    printf("Creating model using app function...\n");
    CLLMModel* model = app_create_cllm_model_default();
    
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    printf("Model created!\n");
    printf("Checking pointers:\n");
    printf("  model->tokens = %p\n", (void*)model->tokens);
    printf("  model->embeddings.embeddings = %p\n", (void*)model->embeddings.embeddings);
    
    printf("\nAttempting to save...\n");
    int result = cllm_write("test_app_model.cllm", model);
    
    if (result == 0) {
        printf("SUCCESS!\n");
    } else {
        printf("FAILED!\n");
    }
    
    return 0;
}
