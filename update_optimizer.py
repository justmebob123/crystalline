#!/usr/bin/env python3

with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

old_code = """void cllm_optimizer_step(CLLMTraining* training) {
    if (!training) return;
    
    float lr = training->config.learning_rate;
    CLLMModel* model = training->model;
    
    // Simple SGD update (no momentum for now - just get it working)
    // Update embeddings
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    size_t embed_params = vocab_size * embedding_dim;
    
    if (model->embeddings.embeddings && training->gradients) {
        for (size_t i = 0; i < embed_params; i++) {
            model->embeddings.embeddings[i] -= lr * training->gradients[i];
        }
    }"""

new_code = """void cllm_optimizer_step(CLLMTraining* training) {
    if (!training) return;
    
    // Gradient accumulation logic
    int accum_steps = training->config.gradient_accumulation_steps;
    if (accum_steps <= 0) accum_steps = 1;
    
    training->accumulation_step++;
    
    // Only apply gradients when we've accumulated enough steps
    if (training->accumulation_step < accum_steps) {
        return;  // Continue accumulating
    }
    
    // Reset accumulation counter
    training->accumulation_step = 0;
    
    // Scale gradients by 1/accum_steps
    float gradient_scale = 1.0f / (float)accum_steps;
    
    float lr = training->config.learning_rate;
    CLLMModel* model = training->model;
    
    // Simple SGD update (no momentum for now - just get it working)
    // Update embeddings
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    size_t embed_params = vocab_size * embedding_dim;
    
    if (model->embeddings.embeddings && training->gradients) {
        for (size_t i = 0; i < embed_params; i++) {
            model->embeddings.embeddings[i] -= lr * training->gradients[i] * gradient_scale;
            training->gradients[i] = 0.0f;  // Clear gradient after update
        }
    }"""

content = content.replace(old_code, new_code)

with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("Updated optimizer_step with gradient accumulation")