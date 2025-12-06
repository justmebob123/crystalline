/**
 * Thread-Safe Tokenizer with 12-Fold Symmetry
 * 
 * MASTER PLAN COMPLIANCE:
 * - 12 vocabulary partitions (one per symmetry group)
 * - Lock-free token lookup using hash-based partitioning
 * - Thread-safe token addition with per-partition locks
 * - Kissing spheres architecture for parallel vocabulary building
 */

#include "../include/cllm_tokenizer.h"
#include "../include/cllm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

// Special token IDs (must match cllm_tokenizer.c)
#define TOKEN_PAD 0
#define TOKEN_UNK 1
#define TOKEN_BOS 2
#define TOKEN_EOS 3
#define TOKEN_MASK 4

/**
 * Hash function to determine partition (0-11) for a token
 * Uses simple string hash modulo 12 for even distribution
 */
static int token_hash_to_partition(const char* token) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *token++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % 12;
}

/**
 * Add token to specific partition (thread-safe)
 * Each partition has its own lock, allowing 12 concurrent additions
 */
uint32_t cllm_add_token_to_partition(CLLMTokenizer* tokenizer, const char* token, int partition_id) {
    if (!tokenizer || !token || partition_id < 0 || partition_id >= 12) {
        return TOKEN_UNK;
    }
    
    pthread_mutex_lock(&tokenizer->partition_locks[partition_id]);
    
    // Search within this partition
    for (uint32_t i = 0; i < tokenizer->partition_sizes[partition_id]; i++) {
        if (tokenizer->vocab_partitions[partition_id][i] && 
            strcmp(tokenizer->vocab_partitions[partition_id][i], token) == 0) {
            // Token exists - increment count
            tokenizer->count_partitions[partition_id][i]++;
            pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
            return i;  // Return local partition index
        }
    }
    
    // Token doesn't exist - add it
    if (tokenizer->partition_sizes[partition_id] >= tokenizer->partition_capacities[partition_id]) {
        // Partition full
        pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
        return TOKEN_UNK;
    }
    
    uint32_t idx = tokenizer->partition_sizes[partition_id];
    tokenizer->vocab_partitions[partition_id][idx] = strdup(token);
    tokenizer->count_partitions[partition_id][idx] = 1;
    tokenizer->partition_sizes[partition_id]++;
    
    pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
    
    return idx;
}

/**
 * Add token using hash-based partitioning (thread-safe)
 * Automatically determines partition based on token hash
 */
uint32_t cllm_add_token_threadsafe(CLLMTokenizer* tokenizer, const char* token) {
    if (!tokenizer || !token) return TOKEN_UNK;
    
    int partition_id = token_hash_to_partition(token);
    return cllm_add_token_to_partition(tokenizer, token, partition_id);
}

/**
 * Consolidate all partitions into single vocabulary
 * Call this after parallel vocabulary building is complete
 */
void cllm_consolidate_vocabulary(CLLMTokenizer* tokenizer) {
    if (!tokenizer || tokenizer->consolidated) return;
    
    printf("Consolidating 12 vocabulary partitions...\n");
    
    // Count total tokens across all partitions
    uint32_t total_tokens = 0;
    for (int i = 0; i < 12; i++) {
        total_tokens += tokenizer->partition_sizes[i];
        printf("  Partition %d: %u tokens\n", i, tokenizer->partition_sizes[i]);
    }
    
    printf("Total tokens across partitions: %u\n", total_tokens);
    
    // Reset main vocabulary
    tokenizer->vocab_size = 5;  // Keep special tokens
    
    // Merge all partitions into main vocabulary
    for (int partition = 0; partition < 12; partition++) {
        for (uint32_t i = 0; i < tokenizer->partition_sizes[partition]; i++) {
            const char* token = tokenizer->vocab_partitions[partition][i];
            uint32_t count = tokenizer->count_partitions[partition][i];
            
            if (!token) continue;
            
            // Check if token already exists in main vocabulary
            uint32_t existing_idx = TOKEN_UNK;
            for (uint32_t j = 0; j < tokenizer->vocab_size; j++) {
                if (tokenizer->vocab[j] && strcmp(tokenizer->vocab[j], token) == 0) {
                    existing_idx = j;
                    break;
                }
            }
            
            if (existing_idx != TOKEN_UNK) {
                // Token exists - add counts
                tokenizer->token_counts[existing_idx] += count;
            } else {
                // New token - add to main vocabulary
                if (tokenizer->vocab_size < tokenizer->max_vocab_size) {
                    tokenizer->vocab[tokenizer->vocab_size] = strdup(token);
                    tokenizer->token_counts[tokenizer->vocab_size] = count;
                    tokenizer->vocab_size++;
                }
            }
        }
    }
    
    tokenizer->consolidated = 1;
    printf("Consolidated vocabulary: %u unique tokens\n", tokenizer->vocab_size);
}