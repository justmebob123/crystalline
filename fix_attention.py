#!/usr/bin/env python3
"""
Fix the attention mechanism in cllm_training.c
Replaces simplified attention with proper multi-head attention
"""

def main():
    # Read the file
    with open('src/ai/cllm_training.c', 'r') as f:
        content = f.read()
    
    # Find and replace the simplified attention section
    old_attention = """        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* input = &layer_input[idx * embed_dim];
                float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &training->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &training->layer_outputs[layer][idx * embed_dim];
                
                // Attention (simplified)
                memcpy(attn_out, input, embed_dim * sizeof(float));
                for (uint32_t d = 0; d < embed_dim; d++) attn_out[d] += input[d];
                
                // FeedForward"""
    
    new_attention = """        // Apply proper multi-head attention for each batch
        AttentionLayer* attn_layer = &model->attention_layers[layer];
        for (int b = 0; b < batch_size; b++) {
            int start_idx = b * seq_len;
            float* batch_input = &layer_input[start_idx * embed_dim];
            float* batch_output = &training->attention_outputs[layer][start_idx * embed_dim];
            
            // Use proper attention mechanism
            cllm_attention_forward(attn_layer, batch_input, batch_output,
                                  NULL, NULL, seq_len);
        }
        
        // Process feedforward for each position
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &training->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &training->layer_outputs[layer][idx * embed_dim];
                
                // FeedForward"""
    
    if old_attention in content:
        content = content.replace(old_attention, new_attention)
        with open('src/ai/cllm_training.c', 'w') as f:
            f.write(content)
        print("✓ Successfully replaced attention mechanism")
        return 0
    else:
        print("✗ Could not find attention section to replace")
        print("Searching for partial matches...")
        if "// Attention (simplified)" in content:
            print("Found '// Attention (simplified)' marker")
        return 1

if __name__ == '__main__':
    exit(main())