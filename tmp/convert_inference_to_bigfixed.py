#!/usr/bin/env python3
"""
Convert cllm_inference.c to use BigFixed operations throughout.
This script updates the inference implementation to use BigFixed** instead of float*.
"""

import re

def convert_inference_to_bigfixed():
    with open('src/ai/cllm_inference.c', 'r') as f:
        content = f.read()
    
    # Add BigFixed includes at the top
    if '#include "algorithms/include/bigfixed_math.h"' not in content:
        # Find the last include
        last_include = content.rfind('#include')
        end_of_line = content.find('\n', last_include)
        content = content[:end_of_line+1] + '#include "algorithms/include/bigfixed_math.h"\n' + \
                  '#include "algorithms/include/matrix_ops_bigfixed.h"\n' + \
                  '#include "prime_float_math.h"\n' + content[end_of_line+1:]
    
    # Update cllm_inference_init to allocate BigFixed buffers
    old_init = r'''CLLMInference\* cllm_inference_init\(CLLMModel\* model\) \{
    if \(!model\) \{
        fprintf\(stderr, "Error: Cannot initialize inference with NULL model\\n"\);
        return NULL;
    \}
    
    CLLMInference\* inference = \(CLLMInference\*\)calloc\(1, sizeof\(CLLMInference\)\);
    if \(!inference\) \{
        fprintf\(stderr, "Error: Failed to allocate inference context\\n"\);
        return NULL;
    \}
    
    inference->model = model;
    inference->temperature = 1\.0f;
    inference->top_p = 0\.9f;
    inference->top_k = 50;
    inference->max_tokens = 50;
    
    // Allocate working memory
    uint32_t embed_dim = model->embeddings\.embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    inference->hidden_states = \(float\*\)calloc\(embed_dim, sizeof\(float\)\);
    inference->logits = \(float\*\)calloc\(vocab_size, sizeof\(float\)\);
    
    if \(!inference->hidden_states \|\| !inference->logits\) \{
        fprintf\(stderr, "Error: Failed to allocate inference buffers\\n"\);
        cllm_inference_cleanup\(inference\);
        return NULL;
    \}
    
    printf\("Inference context initialized successfully\\n"\);
    return inference;
\}'''
    
    new_init = '''CLLMInference* cllm_inference_init(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Error: Cannot initialize inference with NULL model\\n");
        return NULL;
    }
    
    CLLMInference* inference = (CLLMInference*)calloc(1, sizeof(CLLMInference));
    if (!inference) {
        fprintf(stderr, "Error: Failed to allocate inference context\\n");
        return NULL;
    }
    
    inference->model = model;
    inference->temperature = 1.0f;
    inference->top_p = 0.9f;
    inference->top_k = 50;
    inference->max_tokens = 50;
    inference->precision = 128;  // Default BigFixed precision
    
    // Allocate working memory (BigFixed)
    uint32_t embed_dim = model->embeddings.embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Allocate BigFixed buffers
    inference->hidden_states = bigfixed_array_create(embed_dim, inference->precision);
    inference->logits = bigfixed_array_create(vocab_size, inference->precision);
    
    // Allocate KV cache (if needed)
    inference->kv_cache_size = 512;  // Default cache size
    inference->kv_cache_used = 0;
    inference->key_cache = bigfixed_array_create(inference->kv_cache_size * embed_dim, inference->precision);
    inference->value_cache = bigfixed_array_create(inference->kv_cache_size * embed_dim, inference->precision);
    
    if (!inference->hidden_states || !inference->logits || !inference->key_cache || !inference->value_cache) {
        fprintf(stderr, "Error: Failed to allocate inference BigFixed buffers\\n");
        cllm_inference_cleanup(inference);
        return NULL;
    }
    
    printf("Inference context initialized successfully with BigFixed (precision=%d)\\n", inference->precision);
    return inference;
}'''
    
    # Try to replace
    if not re.search(old_init, content, re.DOTALL):
        print("⚠️  Could not find exact match for cllm_inference_init, will update manually")
    else:
        content = re.sub(old_init, new_init, content, flags=re.DOTALL)
        print("✅ Updated cllm_inference_init")
    
    # Update cleanup function
    old_cleanup = r'''void cllm_inference_cleanup\(CLLMInference\* inference\) \{
    if \(!inference\) return;
    
    if \(inference->hidden_states\) free\(inference->hidden_states\);
    if \(inference->logits\) free\(inference->logits\);
    
    free\(inference\);
\}'''
    
    new_cleanup = '''void cllm_inference_cleanup(CLLMInference* inference) {
    if (!inference) return;
    
    // Free BigFixed buffers
    if (inference->hidden_states) bigfixed_array_free(inference->hidden_states, inference->model->embeddings.embedding_dim);
    if (inference->logits) bigfixed_array_free(inference->logits, inference->model->vocab_size);
    if (inference->key_cache) bigfixed_array_free(inference->key_cache, inference->kv_cache_size * inference->model->embeddings.embedding_dim);
    if (inference->value_cache) bigfixed_array_free(inference->value_cache, inference->kv_cache_size * inference->model->embeddings.embedding_dim);
    
    free(inference);
}'''
    
    if not re.search(old_cleanup, content, re.DOTALL):
        print("⚠️  Could not find exact match for cllm_inference_cleanup, will update manually")
    else:
        content = re.sub(old_cleanup, new_cleanup, content, flags=re.DOTALL)
        print("✅ Updated cllm_inference_cleanup")
    
    # Write back
    with open('src/ai/cllm_inference.c', 'w') as f:
        f.write(content)
    
    print("\n✅ Conversion complete!")
    print("Note: cllm_forward and other functions still need manual updates")

if __name__ == '__main__':
    convert_inference_to_bigfixed()