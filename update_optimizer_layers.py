#!/usr/bin/env python3

with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Update attention layer gradients
old_attn = """            if (training->attention_grads[layer].query_lattice && model->attention_layers[layer].query_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].query_lattice[i] -= lr * training->attention_grads[layer].query_lattice[i];
                }
            }
            
            if (training->attention_grads[layer].key_lattice && model->attention_layers[layer].key_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].key_lattice[i] -= lr * training->attention_grads[layer].key_lattice[i];
                }
            }
            
            if (training->attention_grads[layer].value_lattice && model->attention_layers[layer].value_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].value_lattice[i] -= lr * training->attention_grads[layer].value_lattice[i];
                }
            }"""

new_attn = """            if (training->attention_grads[layer].query_lattice && model->attention_layers[layer].query_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].query_lattice[i] -= lr * training->attention_grads[layer].query_lattice[i] * gradient_scale;
                    training->attention_grads[layer].query_lattice[i] = 0.0f;
                }
            }
            
            if (training->attention_grads[layer].key_lattice && model->attention_layers[layer].key_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].key_lattice[i] -= lr * training->attention_grads[layer].key_lattice[i] * gradient_scale;
                    training->attention_grads[layer].key_lattice[i] = 0.0f;
                }
            }
            
            if (training->attention_grads[layer].value_lattice && model->attention_layers[layer].value_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].value_lattice[i] -= lr * training->attention_grads[layer].value_lattice[i] * gradient_scale;
                    training->attention_grads[layer].value_lattice[i] = 0.0f;
                }
            }"""

content = content.replace(old_attn, new_attn)

# Update feedforward layer gradients
old_ff = """            if (training->ff_grads[layer].w1_lattice && ff->w1_lattice) {
                for (uint32_t i = 0; i < input_dim * hidden_dim; i++) {
                    ff->w1_lattice[i] -= lr * training->ff_grads[layer].w1_lattice[i];
                }
            }
            
            if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
                for (uint32_t i = 0; i < hidden_dim * output_dim; i++) {
                    ff->w2_lattice[i] -= lr * training->ff_grads[layer].w2_lattice[i];
                }
            }
            
            if (training->ff_grads[layer].bias1 && ff->bias1) {
                for (uint32_t i = 0; i < hidden_dim; i++) {
                    ff->bias1[i] -= lr * training->ff_grads[layer].bias1[i];
                }
            }
            
            if (training->ff_grads[layer].bias2 && ff->bias2) {
                for (uint32_t i = 0; i < output_dim; i++) {
                    ff->bias2[i] -= lr * training->ff_grads[layer].bias2[i];
                }
            }"""

new_ff = """            if (training->ff_grads[layer].w1_lattice && ff->w1_lattice) {
                for (uint32_t i = 0; i < input_dim * hidden_dim; i++) {
                    ff->w1_lattice[i] -= lr * training->ff_grads[layer].w1_lattice[i] * gradient_scale;
                    training->ff_grads[layer].w1_lattice[i] = 0.0f;
                }
            }
            
            if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
                for (uint32_t i = 0; i < hidden_dim * output_dim; i++) {
                    ff->w2_lattice[i] -= lr * training->ff_grads[layer].w2_lattice[i] * gradient_scale;
                    training->ff_grads[layer].w2_lattice[i] = 0.0f;
                }
            }
            
            if (training->ff_grads[layer].bias1 && ff->bias1) {
                for (uint32_t i = 0; i < hidden_dim; i++) {
                    ff->bias1[i] -= lr * training->ff_grads[layer].bias1[i] * gradient_scale;
                    training->ff_grads[layer].bias1[i] = 0.0f;
                }
            }
            
            if (training->ff_grads[layer].bias2 && ff->bias2) {
                for (uint32_t i = 0; i < output_dim; i++) {
                    ff->bias2[i] -= lr * training->ff_grads[layer].bias2[i] * gradient_scale;
                    training->ff_grads[layer].bias2[i] = 0.0f;
                }
            }"""

content = content.replace(old_ff, new_ff)

with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("Updated layer gradients with accumulation scaling and clearing")