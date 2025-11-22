#include <stdio.h>
#include <stdlib.h>
#include "cllm.h"
#include "cllm_utils.h"
#include "cllm_format.h"

int main() {
    printf("Creating CLLM model...\n");
    CLLMModel* model = cllm_create_small_model();
    
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    printf("Model created successfully!\n");
    printf("Attempting to save model...\n");
    
    int result = cllm_write("test_model.cllm", model);
    
    if (result == 0) {
        printf("SUCCESS: Model saved!\n");
    } else {
        printf("FAILED: Could not save model\n");
    }
    
    return 0;
}
