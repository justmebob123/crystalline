#include "cllm_tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_MERGE_RULES 50000
#define MAX_TOKEN_LENGTH 256

typedef struct {
    char* pair;
    int freq;
} MergePair;

typedef struct {
    char** merges;
    int num_merges;
    int capacity;
} MergeRules;

// BPE tokenizer structure
typedef struct {
    CLLMTokenizer base;
    MergeRules* rules;
    char** byte_vocab;
    int byte_vocab_size;
} BPETokenizer;

// Initialize byte-level vocabulary (256 bytes)
static void init_byte_vocab(BPETokenizer* bpe) {
    bpe->byte_vocab = calloc(256, sizeof(char*));
    bpe->byte_vocab_size = 256;
    
    for (int i = 0; i < 256; i++) {
        bpe->byte_vocab[i] = malloc(4);
        snprintf(bpe->byte_vocab[i], 4, "%02X", i);
    }
}

// Count pair frequencies in text
static void count_pairs(const char* text, MergePair* pairs, int* num_pairs) {
    *num_pairs = 0;
    size_t len = strlen(text);
    
    for (size_t i = 0; i < len - 1; i++) {
        char pair[3] = {text[i], text[i+1], '\0'};
        
        int found = 0;
        for (int j = 0; j < *num_pairs; j++) {
            if (strcmp(pairs[j].pair, pair) == 0) {
                pairs[j].freq++;
                found = 1;
                break;
            }
        }
        
        if (!found && *num_pairs < MAX_MERGE_RULES) {
            pairs[*num_pairs].pair = strdup(pair);
            pairs[*num_pairs].freq = 1;
            (*num_pairs)++;
        }
    }
}

// Find most frequent pair
static int find_max_pair(MergePair* pairs, int num_pairs) {
    int max_idx = 0;
    int max_freq = 0;
    
    for (int i = 0; i < num_pairs; i++) {
        if (pairs[i].freq > max_freq) {
            max_freq = pairs[i].freq;
            max_idx = i;
        }
    }
    
    return max_idx;
}

// Apply merge to text
static char* apply_merge(const char* text, const char* pair) {
    size_t len = strlen(text);
    char* result = malloc(len + 1);
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (i < len - 1 && text[i] == pair[0] && text[i+1] == pair[1]) {
            result[j++] = text[i];
            result[j++] = text[i+1];
            i++; // Skip next char
        } else {
            result[j++] = text[i];
        }
    }
    
    result[j] = '\0';
    return result;
}

// Train BPE on corpus
void bpe_train(BPETokenizer* bpe, const char* corpus, int num_merges) {
    if (!bpe || !corpus) return;
    
    bpe->rules = malloc(sizeof(MergeRules));
    bpe->rules->merges = calloc(num_merges, sizeof(char*));
    bpe->rules->capacity = num_merges;
    bpe->rules->num_merges = 0;
    
    char* text = strdup(corpus);
    
    for (int iter = 0; iter < num_merges; iter++) {
        MergePair* pairs = calloc(MAX_MERGE_RULES, sizeof(MergePair));
        int num_pairs = 0;
        
        count_pairs(text, pairs, &num_pairs);
        
        if (num_pairs == 0) break;
        
        int max_idx = find_max_pair(pairs, num_pairs);
        
        bpe->rules->merges[bpe->rules->num_merges++] = strdup(pairs[max_idx].pair);
        
        char* new_text = apply_merge(text, pairs[max_idx].pair);
        free(text);
        text = new_text;
        
        for (int i = 0; i < num_pairs; i++) {
            free(pairs[i].pair);
        }
        free(pairs);
    }
    
    free(text);
}

// Encode text using BPE
uint32_t* bpe_encode(BPETokenizer* bpe, const char* text, uint32_t* num_tokens) {
    if (!bpe || !text) return NULL;
    
    size_t len = strlen(text);
    uint32_t* tokens = malloc(len * sizeof(uint32_t));
    *num_tokens = 0;
    
    // Convert to byte-level tokens
    for (size_t i = 0; i < len; i++) {
        unsigned char byte = (unsigned char)text[i];
        tokens[(*num_tokens)++] = byte;
    }
    
    // Apply merge rules
    for (int i = 0; i < bpe->rules->num_merges; i++) {
        // Apply merge rule
        // (simplified - full implementation would merge tokens)
    }
    
    return tokens;
}

// Create BPE tokenizer
BPETokenizer* bpe_create(uint32_t vocab_size) {
    BPETokenizer* bpe = calloc(1, sizeof(BPETokenizer));
    if (!bpe) return NULL;
    
    bpe->base.max_vocab_size = vocab_size;
    bpe->base.vocab_size = 5; // Special tokens
    
    bpe->base.vocab = calloc(vocab_size, sizeof(char*));
    bpe->base.token_counts = calloc(vocab_size, sizeof(uint32_t));
    
    // Add special tokens
    bpe->base.vocab[0] = strdup("<PAD>");
    bpe->base.vocab[1] = strdup("<UNK>");
    bpe->base.vocab[2] = strdup("<BOS>");
    bpe->base.vocab[3] = strdup("<EOS>");
    bpe->base.vocab[4] = strdup("<MASK>");
    
    init_byte_vocab(bpe);
    
    return bpe;
}

// Free BPE tokenizer
void bpe_free(BPETokenizer* bpe) {
    if (!bpe) return;
    
    if (bpe->rules) {
        for (int i = 0; i < bpe->rules->num_merges; i++) {
            free(bpe->rules->merges[i]);
        }
        free(bpe->rules->merges);
        free(bpe->rules);
    }
    
    if (bpe->byte_vocab) {
        for (int i = 0; i < bpe->byte_vocab_size; i++) {
            free(bpe->byte_vocab[i]);
        }
        free(bpe->byte_vocab);
    }
    
    cllm_free_tokenizer(&bpe->base);
    free(bpe);
}