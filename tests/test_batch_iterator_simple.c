#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Minimal batch iterator structure
typedef struct {
    uint32_t* tokens;
    size_t num_tokens;
    size_t current_pos;
    uint32_t batch_size;
    uint32_t seq_len;
    int shuffle;
    int drop_last;
} CLLMBatchIterator;

CLLMBatchIterator* cllm_batch_iterator_create(uint32_t* tokens, size_t num_tokens,
                                              uint32_t batch_size, uint32_t seq_len,
                                              int shuffle, int drop_last) {
    printf("ENTER cllm_batch_iterator_create\n");
    fflush(stdout);
    
    if (!tokens || num_tokens == 0) {
        printf("NULL tokens or zero num_tokens\n");
        fflush(stdout);
        return NULL;
    }
    
    printf("Allocating iterator\n");
    fflush(stdout);
    
    CLLMBatchIterator* iter = (CLLMBatchIterator*)calloc(1, sizeof(CLLMBatchIterator));
    if (!iter) {
        printf("Failed to allocate iterator\n");
        fflush(stdout);
        return NULL;
    }
    
    printf("Setting fields\n");
    fflush(stdout);
    
    iter->tokens = tokens;
    iter->num_tokens = num_tokens;
    iter->current_pos = 0;
    iter->batch_size = batch_size;
    iter->seq_len = seq_len;
    iter->shuffle = shuffle;
    iter->drop_last = drop_last;
    
    printf("EXIT cllm_batch_iterator_create: %p\n", (void*)iter);
    fflush(stdout);
    
    return iter;
}

int main() {
    printf("Starting test\n");
    fflush(stdout);
    
    // Create test tokens
    uint32_t* tokens = (uint32_t*)malloc(415454 * sizeof(uint32_t));
    for (size_t i = 0; i < 415454; i++) {
        tokens[i] = i % 1000;
    }
    
    printf("Created tokens\n");
    fflush(stdout);
    
    printf("Calling cllm_batch_iterator_create\n");
    fflush(stdout);
    
    CLLMBatchIterator* iter = cllm_batch_iterator_create(
        tokens,
        415454,
        32,
        128,
        0,
        0
    );
    
    printf("After cllm_batch_iterator_create: %p\n", (void*)iter);
    fflush(stdout);
    
    if (iter) {
        printf("SUCCESS: Iterator created\n");
        free(iter);
    } else {
        printf("FAILED: Iterator is NULL\n");
    }
    
    free(tokens);
    
    printf("Test complete\n");
    return 0;
}