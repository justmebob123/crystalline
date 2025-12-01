#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the final projection section
old_text = """    // Copy final hidden
    memcpy(training->final_hidden, layer_input, batch_size * seq_len * embed_dim * sizeof(float));
    
    // Project to vocabulary
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* hidden = &training->final_hidden[idx * embed_dim];
            float* logits = &training->logits[idx * vocab_size];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float* vocab_embed = &model->embeddings.embeddings[v * embed_dim];
                float score = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    score += hidden[d] * vocab_embed[d];
                }
                logits[v] = score;
            }"""

new_text = """    // Copy final hidden - BIGFIXED IMPLEMENTATION
    // Copy BigFixed pointers from layer_input to final_hidden
    for (uint32_t i = 0; i < batch_size * seq_len * embed_dim; i++) {
        training->final_hidden[i] = layer_input[i];
    }
    
    // Project to vocabulary - BIGFIXED IMPLEMENTATION
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            BigFixed** hidden = &training->final_hidden[idx * embed_dim];
            BigFixed** logits = &training->logits[idx * vocab_size];
            
            // Compute logits: logits[v] = dot(hidden, vocab_embed[v])
            for (uint32_t v = 0; v < vocab_size; v++) {
                // Initialize logit to zero
                if (!logits[v]) {
                    logits[v] = big_fixed_create(training->precision_bits);
                }
                big_fixed_from_int(logits[v], 0);
                
                // Compute dot product with vocabulary embedding
                BigFixed* temp = big_fixed_create(training->precision_bits);
                
                if (model->crystalline_embeddings && model->crystalline_embeddings->token_positions) {
                    // Use crystalline embeddings (BigFixed-based)
                    for (uint32_t d = 0; d < embed_dim; d++) {
                        uint32_t vocab_idx = v * embed_dim + d;
                        big_fixed_mul(temp, hidden[d], model->crystalline_embeddings->token_positions[vocab_idx]);
                        big_fixed_add(logits[v], logits[v], temp);
                    }
                } else {
                    // Fallback: Convert deprecated float embeddings to BigFixed
                    BigFixed* vocab_bf = big_fixed_create(training->precision_bits);
                    for (uint32_t d = 0; d < embed_dim; d++) {
                        uint32_t vocab_idx = v * embed_dim + d;
                        big_fixed_from_double(vocab_bf, (double)model->embeddings.embeddings[vocab_idx]);
                        big_fixed_mul(temp, hidden[d], vocab_bf);
                        big_fixed_add(logits[v], logits[v], temp);
                    }
                    big_fixed_free(vocab_bf);
                }
                
                big_fixed_free(temp);
            }"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Replaced final projection with complete BigFixed implementation")
else:
    print("ERROR: Could not find exact match")
    if "// Copy final hidden" in content:
        print("Found: // Copy final hidden")
    if "// Project to vocabulary" in content:
        print("Found: // Project to vocabulary")