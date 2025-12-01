#!/usr/bin/env python3
import re

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the section
old_text = """    // Process through layers
    float* layer_input = training->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        memcpy(training->layer_inputs[layer], layer_input, batch_size * seq_len * embed_dim * sizeof(float));
        
        // Apply proper multi-head attention for each batch
        AttentionLayer* attn_layer = &model->attention_layers[layer];
        for (int b = 0; b < batch_size; b++) {
            int start_idx = b * seq_len;
            float* batch_input = &layer_input[start_idx * embed_dim];
            float* batch_output = &training->attention_outputs[layer][start_idx * embed_dim];"""

new_text = """    // Process through layers - BIGFIXED IMPLEMENTATION
    // layer_input is BigFixed** (array of BigFixed pointers)
    BigFixed** layer_input = training->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Copy BigFixed pointers to layer input buffer
        // training->layer_inputs[layer] is BigFixed** [batch*seq*embed_dim]
        for (uint32_t i = 0; i < batch_size * seq_len * embed_dim; i++) {
            training->layer_inputs[layer][i] = layer_input[i];
        }
        
        // Apply proper multi-head attention for each batch
        AttentionLayer* attn_layer = &model->attention_layers[layer];
        for (int b = 0; b < batch_size; b++) {
            int start_idx = b * seq_len;
            // batch_input is BigFixed** pointing to start of this batch
            BigFixed** batch_input = &layer_input[start_idx * embed_dim];
            BigFixed** batch_output = &training->attention_outputs[layer][start_idx * embed_dim];"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Replaced layer processing section")
else:
    print("ERROR: Could not find exact match")
    print("Searching for partial match...")
    if "float* layer_input = training->input_embeddings;" in content:
        print("Found: float* layer_input = training->input_embeddings;")
    else:
        print("NOT FOUND: float* layer_input = training->input_embeddings;")