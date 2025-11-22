#include <stdio.h>
#include <stdlib.h>
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"

int main() {
    printf("Testing data loader...\n\n");
    
    // Create tokenizer
    printf("Creating tokenizer...\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(1000);
    if (!tokenizer) {
        printf("Failed to create tokenizer\n");
        return 1;
    }
    printf("✓ Tokenizer created\n\n");
    
    // Create data loader
    printf("Creating data loader...\n");
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        printf("Failed to create data loader\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Data loader created\n\n");
    
    // Load files
    printf("Loading data files...\n");
    int files = cllm_data_loader_load_directory(loader, "./data/raw");
    printf("Loaded %d files\n\n", files);
    
    if (files == 0) {
        printf("No files loaded\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Build vocabulary
    printf("Building vocabulary...\n");
    cllm_data_loader_build_vocab(loader);
    cllm_data_loader_print_stats(loader);
    
    // Print vocabulary stats
    printf("\nVocabulary statistics:\n");
    cllm_print_vocab_stats(tokenizer);
    
    // Create dataset
    printf("\nCreating dataset...\n");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        printf("Failed to create dataset\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    printf("✓ Dataset created: %zu tokens\n", dataset->num_tokens);
    
    // Save dataset
    printf("\nSaving dataset...\n");
    cllm_token_dataset_save(dataset, "./data/test_dataset.bin");
    
    // Cleanup
    cllm_token_dataset_free(dataset);
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    
    printf("\n✓ All tests passed!\n");
    return 0;
}