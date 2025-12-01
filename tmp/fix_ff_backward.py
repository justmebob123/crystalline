#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the feedforward backward section
old_text = """                // FeedForward backward
                float* grad_hidden = (float*)calloc(ff->hidden_dim, sizeof(float));
                if (!grad_hidden) continue;
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        if (training->ff_grads[layer].w2_lattice) {
                            training->ff_grads[layer].w2_lattice[h * embed_dim + o] += hidden[h] * grad[o];
                        }
                        grad_hidden[h] += ff->w2_lattice[h * embed_dim + o] * grad[o];
                    }
                    if (training->ff_grads[layer].bias2) {
                        training->ff_grads[layer].bias2[o] += grad[o];
                    }
                }
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float tanh_val = hidden[h];
                    grad_hidden[h] *= (1.0f - tanh_val * tanh_val);
                }
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        if (training->ff_grads[layer].w1_lattice) {
                            training->ff_grads[layer].w1_lattice[i * ff->hidden_dim + h] += input[i] * grad_hidden[h];
                        }
                        grad[i] += ff->w1_lattice[i * ff->hidden_dim + h] * grad_hidden[h];
                    }
                    if (training->ff_grads[layer].bias1) {
                        training->ff_grads[layer].bias1[h] += grad_hidden[h];
                    }
                }
                
                free(grad_hidden);"""

new_text = """                // FeedForward backward - BIGFIXED IMPLEMENTATION
                // Allocate BigFixed array for hidden gradients
                BigFixed** grad_hidden = (BigFixed**)calloc(ff->hidden_dim, sizeof(BigFixed*));
                if (!grad_hidden) continue;
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    grad_hidden[h] = big_fixed_create(training->precision_bits);
                    big_fixed_from_int(grad_hidden[h], 0);
                }
                
                BigFixed* temp = big_fixed_create(training->precision_bits);
                
                // Backward through W2: compute W2 gradients and propagate to hidden
                for (uint32_t o = 0; o < embed_dim; o++) {
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        uint32_t w2_idx = h * embed_dim + o;
                        
                        // W2 gradient: w2_grad += hidden[h] * grad[o]
                        if (training->ff_grads[layer].w2_lattice) {
                            big_fixed_mul(temp, hidden[h], grad[o]);
                            big_fixed_add(training->ff_grads[layer].w2_lattice[w2_idx],
                                         training->ff_grads[layer].w2_lattice[w2_idx],
                                         temp);
                        }
                        
                        // Propagate gradient: grad_hidden[h] += w2[h,o] * grad[o]
                        big_fixed_mul(temp, ff->w2_lattice[w2_idx], grad[o]);
                        big_fixed_add(grad_hidden[h], grad_hidden[h], temp);
                    }
                    
                    // Bias2 gradient: bias2_grad += grad[o]
                    if (training->ff_grads[layer].bias2) {
                        big_fixed_add(training->ff_grads[layer].bias2[o],
                                     training->ff_grads[layer].bias2[o],
                                     grad[o]);
                    }
                }
                
                // Apply tanh derivative: grad_hidden *= (1 - tanh^2)
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    double tanh_val = big_fixed_to_double(hidden[h]);
                    double tanh_deriv = 1.0 - tanh_val * tanh_val;
                    double grad_h = big_fixed_to_double(grad_hidden[h]);
                    grad_h *= tanh_deriv;
                    big_fixed_from_double(grad_hidden[h], grad_h);
                }
                
                // Backward through W1: compute W1 gradients and propagate to input
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        uint32_t w1_idx = i * ff->hidden_dim + h;
                        
                        // W1 gradient: w1_grad += input[i] * grad_hidden[h]
                        if (training->ff_grads[layer].w1_lattice) {
                            big_fixed_mul(temp, input[i], grad_hidden[h]);
                            big_fixed_add(training->ff_grads[layer].w1_lattice[w1_idx],
                                         training->ff_grads[layer].w1_lattice[w1_idx],
                                         temp);
                        }
                        
                        // Propagate gradient: grad[i] += w1[i,h] * grad_hidden[h]
                        big_fixed_mul(temp, ff->w1_lattice[w1_idx], grad_hidden[h]);
                        big_fixed_add(grad[i], grad[i], temp);
                    }
                    
                    // Bias1 gradient: bias1_grad += grad_hidden[h]
                    if (training->ff_grads[layer].bias1) {
                        big_fixed_add(training->ff_grads[layer].bias1[h],
                                     training->ff_grads[layer].bias1[h],
                                     grad_hidden[h]);
                    }
                }
                
                // Cleanup
                big_fixed_free(temp);
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    big_fixed_free(grad_hidden[h]);
                }
                free(grad_hidden);"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Replaced feedforward backward with complete BigFixed implementation")
else:
    print("ERROR: Could not find exact match for feedforward backward")
    if "// FeedForward backward" in content:
        print("Found: // FeedForward backward")