#ifndef CLLM_TOKENIZER_H
#define CLLM_TOKENIZER_H

#include <stdint.h>
#include <stddef.h>

/**
 * Tokenizer Structure
 */
typedef struct {
    char** vocab;
    uint32_t* token_counts;
    uint32_t vocab_size;
    uint32_t max_vocab_size;
} CLLMTokenizer;

/**
 * Create Tokenizer
 */
CLLMTokenizer* cllm_create_tokenizer(uint32_t max_vocab_size);

/**
 * Free Tokenizer
 */
void cllm_free_tokenizer(CLLMTokenizer* tokenizer);

/**
 * Find Token in Vocabulary
 */
uint32_t cllm_find_token(CLLMTokenizer* tokenizer, const char* token);

/**
 * Add Token to Vocabulary
 */
uint32_t cllm_add_token(CLLMTokenizer* tokenizer, const char* token);

/**
 * Encode Text to Token IDs
 */
uint32_t* cllm_tokenizer_encode(CLLMTokenizer* tokenizer, const char* text, uint32_t* num_tokens);

/**
 * Decode Token IDs to Text
 */
char* cllm_tokenizer_decode(CLLMTokenizer* tokenizer, uint32_t* tokens, uint32_t num_tokens);

/**
 * Build Vocabulary from Text
 */
void cllm_build_vocab(CLLMTokenizer* tokenizer, const char* text);

/**
 * Save Vocabulary to File
 */
int cllm_save_vocab(CLLMTokenizer* tokenizer, const char* filename);

/**
 * Load Vocabulary from File
 */
int cllm_load_vocab(CLLMTokenizer* tokenizer, const char* filename);

/**
 * Get Vocabulary Size
 */
uint32_t cllm_get_vocab_size(CLLMTokenizer* tokenizer);

/**
 * Get Token String
 */
const char* cllm_get_token_string(CLLMTokenizer* tokenizer, uint32_t token_id);

/**
 * Print Vocabulary Statistics
 */
void cllm_print_vocab_stats(CLLMTokenizer* tokenizer);

/**
 * Encode with Special Tokens
 */
uint32_t* cllm_encode_with_special(CLLMTokenizer* tokenizer, const char* text, uint32_t* num_tokens);

#endif // CLLM_TOKENIZER_H