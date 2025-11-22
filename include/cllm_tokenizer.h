#ifndef CLLM_TOKENIZER_H
#define CLLM_TOKENIZER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * CLLM Tokenizer Structure
 */
typedef struct {
    char** vocab;           // Vocabulary (array of strings)
    uint32_t vocab_size;    // Size of vocabulary
    uint32_t* token_counts; // Token frequency counts
    uint32_t max_vocab_size; // Maximum vocabulary size
} CLLMTokenizer;

/* Tokenizer functions */
CLLMTokenizer* cllm_create_tokenizer(uint32_t max_vocab_size);
void cllm_free_tokenizer(CLLMTokenizer* tokenizer);
uint32_t cllm_tokenize(CLLMTokenizer* tokenizer, const char* text);
const char* cllm_detokenize(CLLMTokenizer* tokenizer, uint32_t token_id);
bool cllm_add_to_vocab(CLLMTokenizer* tokenizer, const char* token);
uint32_t cllm_get_vocab_size(CLLMTokenizer* tokenizer);

#endif /* CLLM_TOKENIZER_H */