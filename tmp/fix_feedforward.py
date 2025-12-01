#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the feedforward section
old_text = """        // Process feedforward for each position
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &training->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &training->layer_outputs[layer][idx * embed_dim];"""

new_text = """        // Process feedforward for each position - BIGFIXED IMPLEMENTATION
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                // All outputs are BigFixed** arrays
                BigFixed** attn_out = &training->attention_outputs[layer][idx * embed_dim];
                BigFixed** ff_out = &training->ff_outputs[layer][idx * embed_dim];
                BigFixed** layer_out = &training->layer_outputs[layer][idx * embed_dim];"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Replaced feedforward section")
else:
    print("ERROR: Could not find exact match for feedforward section")