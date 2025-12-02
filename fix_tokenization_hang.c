/**
 * CRITICAL FIX: Replace O(n²) tokenization with O(n) hash map lookup
 * 
 * Problem: cllm_load_training_data uses linear search through vocabulary
 * for EVERY token in the file, resulting in O(n²) complexity.
 * 
 * Solution: Use a hash map for O(1) token lookup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Simple hash function for strings
static uint32_t hash_string(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

/**
 * OPTIMIZED: cllm_load_training_data with hash map
 * 
 * This version builds a hash map of the vocabulary first (O(n)),
 * then uses it for O(1) lookups during tokenization.
 * 
 * Total complexity: O(vocab_size + num_tokens) instead of O(vocab_size × num_tokens)
 */
int cllm_load_training_data_optimized(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    FILE* f = fopen(filename, "r");
    if (!f) return -1;
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read file content
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(content, 1, file_size, f);
    content[bytes_read] = '\0';
    fclose(f);
    
    printf("  File loaded: %ld bytes\n", file_size);
    
    // OPTIMIZATION: Build hash map of vocabulary (O(vocab_size))
    // This is done ONCE, not for every token!
    typedef struct {
        char* token_str;
        uint32_t token_id;
    } VocabEntry;
    
    #define HASH_MAP_SIZE 65536  // Power of 2 for fast modulo
    VocabEntry** hash_map = (VocabEntry**)calloc(HASH_MAP_SIZE, sizeof(VocabEntry*));
    if (!hash_map) {
        free(content);
        return -1;
    }
    
    printf("  Building vocabulary hash map...\n");
    
    // Build hash map from vocabulary
    if (training->model->tokens) {
        for (uint32_t i = 0; i < training->model->vocab_size; i++) {
            if (training->model->tokens[i].token_str[0] != '\0') {
                uint32_t hash = hash_string(training->model->tokens[i].token_str) % HASH_MAP_SIZE;
                
                // Linear probing for collisions
                while (hash_map[hash] != NULL) {
                    hash = (hash + 1) % HASH_MAP_SIZE;
                }
                
                VocabEntry* entry = (VocabEntry*)malloc(sizeof(VocabEntry));
                entry->token_str = training->model->tokens[i].token_str;
                entry->token_id = i;
                hash_map[hash] = entry;
            }
        }
    }
    
    printf("  Hash map built\n");
    
    // Reallocate tokens array
    size_t old_num_tokens = training->num_tokens;
    size_t new_capacity = old_num_tokens + file_size;  // Overestimate
    
    uint32_t* new_tokens = (uint32_t*)realloc(training->tokens, new_capacity * sizeof(uint32_t));
    if (!new_tokens) {
        free(content);
        // Free hash map
        for (int i = 0; i < HASH_MAP_SIZE; i++) {
            if (hash_map[i]) free(hash_map[i]);
        }
        free(hash_map);
        return -1;
    }
    training->tokens = new_tokens;
    
    size_t tokens_added = 0;
    
    printf("  Tokenizing with hash map (O(n) instead of O(n²))...\n");
    
    // Tokenize using hash map (O(1) lookup per token!)
    char* token = strtok(content, " \t\n\r");
    int token_count = 0;
    
    while (token != NULL &amp;&amp; tokens_added < new_capacity) {
        token_count++;
        if (token_count % 100000 == 0) {
            printf("    Processed %d tokens...\n", token_count);
        }
        
        // Convert to lowercase
        for (char* p = token; *p; p++) {
            *p = tolower(*p);
        }
        
        // Hash map lookup (O(1) instead of O(n)!)
        uint32_t hash = hash_string(token) % HASH_MAP_SIZE;
        bool found = false;
        
        // Linear probing to find token
        int probes = 0;
        while (hash_map[hash] != NULL &amp;&amp; probes < HASH_MAP_SIZE) {
            if (strcmp(hash_map[hash]->token_str, token) == 0) {
                training->tokens[old_num_tokens + tokens_added] = hash_map[hash]->token_id;
                tokens_added++;
                found = true;
                break;
            }
            hash = (hash + 1) % HASH_MAP_SIZE;
            probes++;
        }
        
        // If not found, use hash as token ID
        if (!found) {
            uint32_t hash_id = hash_string(token) % training->model->vocab_size;
            training->tokens[old_num_tokens + tokens_added] = hash_id;
            tokens_added++;
        }
        
        token = strtok(NULL, " \t\n\r");
    }
    
    printf("  Tokenization complete: %zu tokens added\n", tokens_added);
    
    // Update total token count
    training->num_tokens = old_num_tokens + tokens_added;
    
    // Free hash map
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        if (hash_map[i]) free(hash_map[i]);
    }
    free(hash_map);
    free(content);
    
    printf("  Total tokens in training: %zu\n", training->num_tokens);
    
    return 0;
}