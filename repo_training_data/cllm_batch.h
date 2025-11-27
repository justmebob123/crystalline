/**
 * CLLM Batch Generation Header
 * 
 * Provides batch generation with padding and attention masking
 */

#ifndef CLLM_BATCH_H
#define CLLM_BATCH_H

#include <stdint.h>
#include <stddef.h>

/**
 * Batch Structure
 */
typedef struct {
    uint32_t* input_ids;        // [batch_size * seq_len]
    uint32_t* target_ids;       // [batch_size * seq_len]
    float* attention_mask;      // [batch_size * seq_len]
    uint32_t batch_size;
    uint32_t seq_len;
    uint32_t num_valid_tokens;  // Total non-padding tokens
} CLLMBatch;

/**
 * Batch Iterator Structure (opaque)
 */
typedef struct CLLMBatchIterator CLLMBatchIterator;

/**
 * Create Batch
 * 
 * Allocates a new batch with given dimensions
 */
CLLMBatch* cllm_batch_create(uint32_t batch_size, uint32_t seq_len);

/**
 * Free Batch
 */
void cllm_batch_free(CLLMBatch* batch);

/**
 * Create Batch Iterator
 * 
 * Creates an iterator over a token array that generates batches
 * 
 * @param tokens Source token array
 * @param num_tokens Number of tokens in array
 * @param batch_size Batch size
 * @param seq_len Sequence length
 * @param shuffle Whether to shuffle (not implemented yet)
 * @param drop_last Whether to drop incomplete last batch
 */
CLLMBatchIterator* cllm_batch_iterator_create(uint32_t* tokens, size_t num_tokens,
                                               uint32_t batch_size, uint32_t seq_len,
                                               int shuffle, int drop_last);

/**
 * Free Batch Iterator
 */
void cllm_batch_iterator_free(CLLMBatchIterator* iter);

/**
 * Reset Batch Iterator
 * 
 * Resets iterator to beginning
 */
void cllm_batch_iterator_reset(CLLMBatchIterator* iter);

/**
 * Get Next Batch
 * 
 * Returns next batch from iterator, or NULL if no more batches
 * Caller is responsible for freeing the returned batch
 */
CLLMBatch* cllm_batch_iterator_next(CLLMBatchIterator* iter);

/**
 * Get Number of Batches
 * 
 * Returns total number of batches that will be generated
 */
size_t cllm_batch_iterator_num_batches(CLLMBatchIterator* iter);

/**
 * Create Batch from Token Array
 * 
 * Simple helper to create a single batch from tokens
 */
CLLMBatch* cllm_create_batch_from_tokens(uint32_t* tokens, size_t num_tokens,
                                          uint32_t batch_size, uint32_t seq_len);

/**
 * Print Batch Statistics
 */
void cllm_batch_print_stats(CLLMBatch* batch);

/**
 * Validate Batch
 * 
 * Checks batch integrity, returns 1 if valid, 0 otherwise
 */
int cllm_batch_validate(CLLMBatch* batch);

#endif // CLLM_BATCH_H