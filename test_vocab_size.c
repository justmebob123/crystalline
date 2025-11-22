#include <stdio.h>
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"

int main() {
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(500);
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    
    int files = cllm_data_loader_load_directory(loader, "./data/raw");
    printf("Files loaded: %d\n", files);
    
    cllm_data_loader_build_vocab(loader);
    printf("Vocabulary size: %u\n", tokenizer->vocab_size);
    
    // Test model creation with this vocab size
    printf("\nTesting model creation with vocab_size=%u\n", tokenizer->vocab_size);
    
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    return 0;
}
