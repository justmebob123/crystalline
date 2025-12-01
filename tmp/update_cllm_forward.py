#!/usr/bin/env python3
"""
Update cllm_forward to use BigFixed operations.
"""

def update_cllm_forward():
    with open('src/ai/cllm_inference.c', 'r') as f:
        lines = f.readlines()
    
    # Find the cllm_forward function
    start_idx = None
    end_idx = None
    
    for i, line in enumerate(lines):
        if 'void cllm_forward(CLLMInference* inference' in line:
            start_idx = i
        if start_idx is not None and line.strip() == '}' and i > start_idx + 10:
            end_idx = i
            break
    
    if start_idx is None or end_idx is None:
        print("❌ Could not find cllm_forward function")
        return
    
    # Create new implementation
    new_forward = '''void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens) {
    if (!inference || !tokens || num_tokens <= 0) return;
    
    CLLMModel* model = inference->model;
    if (!model) {
        fprintf(stderr, "Error: Model is NULL in cllm_forward\\n");
        return;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    int precision = inference->precision;
    
    // Check critical pointers
    if (!inference->hidden_states) {
        fprintf(stderr, "Error: hidden_states is NULL\\n");
        return;
    }
    if (!inference->logits) {
        fprintf(stderr, "Error: logits is NULL\\n");
        return;
    }
    if (!model->embeddings.embeddings) {
        fprintf(stderr, "Error: embeddings is NULL\\n");
        return;
    }
    
    // Get embedding for last token
    uint32_t last_token = tokens[num_tokens - 1];
    if (last_token >= model->vocab_size) {
        fprintf(stderr, "Error: token %u out of range (vocab_size=%lu)\\n", last_token, (unsigned long)model->vocab_size);
        return;
    }
    
    // Convert float embedding to BigFixed
    float* float_embedding = &model->embeddings.embeddings[last_token * embed_dim];
    for (uint32_t i = 0; i < embed_dim; i++) {
        bigfixed_from_float(inference->hidden_states[i], float_embedding[i], precision);
    }
    
    // TODO: Apply positional encoding with BigFixed
    // For now, skip positional encoding or implement BigFixed version
    
    // Pass through transformer layers using BigFixed
    if (model->attention_layers && model->ff_layers && model->layer_norms) {
        BigFixed** attn_output = bigfixed_array_create(embed_dim, precision);
        if (!attn_output) {
            fprintf(stderr, "Error: Failed to allocate attention output buffer\\n");
            return;
        }
        
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            // Layer norm (BigFixed)
            cllm_layer_norm_bigfixed(&model->layer_norms[layer], inference->hidden_states, 
                                    inference->hidden_states, precision);
            
            // Attention (BigFixed)
            AttentionLayer* attn_layer = &model->attention_layers[layer];
            cllm_attention_forward_bigfixed(attn_layer, inference->hidden_states, attn_output,
                                          NULL, NULL, 1, precision);
            
            // Copy attention output back to hidden states
            for (uint32_t i = 0; i < embed_dim; i++) {
                bigfixed_copy(inference->hidden_states[i], attn_output[i]);
            }
            
            // Feed-forward (BigFixed)
            cllm_feedforward_bigfixed(&model->ff_layers[layer], inference->hidden_states,
                                     inference->hidden_states, precision);
        }
        
        bigfixed_array_free(attn_output, embed_dim);
        
        // Final layer norm (BigFixed)
        cllm_layer_norm_bigfixed(&model->layer_norms[model->num_layers - 1], 
                                inference->hidden_states, inference->hidden_states, precision);
    }
    
    // Project to vocabulary (BigFixed dot product with embeddings)
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        bigfixed_set_zero(inference->logits[i]);
        float* token_embed = &model->embeddings.embeddings[i * embed_dim];
        
        // Compute dot product: logits[i] = sum(hidden_states[j] * token_embed[j])
        for (uint32_t j = 0; j < embed_dim; j++) {
            BigFixed* temp = bigfixed_create(precision);
            bigfixed_from_float(temp, token_embed[j], precision);
            bigfixed_mul(temp, inference->hidden_states[j], temp);
            bigfixed_add(inference->logits[i], inference->logits[i], temp);
            bigfixed_free(temp);
        }
    }
}
'''
    
    # Replace the function
    lines = lines[:start_idx] + [new_forward + '\n'] + lines[end_idx+1:]
    
    with open('src/ai/cllm_inference.c', 'w') as f:
        f.writelines(lines)
    
    print("✅ Updated cllm_forward to use BigFixed operations")

if __name__ == '__main__':
    update_cllm_forward()