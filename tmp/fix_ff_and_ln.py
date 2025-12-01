#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the feedforward computation
old_text = """                // FeedForward
                FeedForwardLayer* ff = &model->ff_layers[layer];
                float* ff_hidden = &training->ff_hidden[layer][idx * ff->hidden_dim];
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float sum = (float)big_fixed_to_double(ff->bias1[h]);
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        sum += attn_out[i] * (float)big_fixed_to_double(ff->w1_lattice[i * ff->hidden_dim + h]);
                    }
                    ff_hidden[h] = prime_tanhf(sum);
                }
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    float sum = (float)big_fixed_to_double(ff->bias2[o]);
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        sum += ff_hidden[h] * (float)big_fixed_to_double(ff->w2_lattice[h * embed_dim + o]);
                    }
                    ff_out[o] = sum;
                }
                
                // Residual + LayerNorm
                for (uint32_t d = 0; d < embed_dim; d++) layer_out[d] = attn_out[d] + ff_out[d];
                
                CLLMLayerNorm* ln = &model->layer_norms[layer];
                float mean = 0.0f, var = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) mean += layer_out[d];
                mean /= embed_dim;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = layer_out[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                float std = prime_sqrtf(var + 1e-5f);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];
                }"""

new_text = """                // FeedForward - BIGFIXED IMPLEMENTATION
                FeedForwardLayer* ff = &model->ff_layers[layer];
                BigFixed** ff_hidden = &training->ff_hidden[layer][idx * ff->hidden_dim];
                
                // W1 * attn_out + bias1, then tanh
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    // Initialize with bias1
                    if (!ff_hidden[h]) {
                        ff_hidden[h] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_copy(ff_hidden[h], ff->bias1[h]);
                    
                    // Accumulate: sum += attn_out[i] * w1[i,h]
                    BigFixed* temp = big_fixed_create(training->precision_bits);
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        uint32_t w_idx = i * ff->hidden_dim + h;
                        big_fixed_mul(temp, attn_out[i], ff->w1_lattice[w_idx]);
                        big_fixed_add(ff_hidden[h], ff_hidden[h], temp);
                    }
                    big_fixed_free(temp);
                    
                    // Apply tanh activation
                    double val = big_fixed_to_double(ff_hidden[h]);
                    val = prime_tanhf((float)val);
                    big_fixed_from_double(ff_hidden[h], val);
                }
                
                // W2 * ff_hidden + bias2
                for (uint32_t o = 0; o < embed_dim; o++) {
                    // Initialize with bias2
                    if (!ff_out[o]) {
                        ff_out[o] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_copy(ff_out[o], ff->bias2[o]);
                    
                    // Accumulate: sum += ff_hidden[h] * w2[h,o]
                    BigFixed* temp = big_fixed_create(training->precision_bits);
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        uint32_t w_idx = h * embed_dim + o;
                        big_fixed_mul(temp, ff_hidden[h], ff->w2_lattice[w_idx]);
                        big_fixed_add(ff_out[o], ff_out[o], temp);
                    }
                    big_fixed_free(temp);
                }
                
                // Residual connection: layer_out = attn_out + ff_out
                for (uint32_t d = 0; d < embed_dim; d++) {
                    if (!layer_out[d]) {
                        layer_out[d] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_add(layer_out[d], attn_out[d], ff_out[d]);
                }
                
                // LayerNorm - BIGFIXED IMPLEMENTATION
                CLLMLayerNorm* ln = &model->layer_norms[layer];
                
                // Compute mean
                BigFixed* mean = big_fixed_create(training->precision_bits);
                big_fixed_from_int(mean, 0);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_add(mean, mean, layer_out[d]);
                }
                BigFixed* embed_dim_bf = big_fixed_create(training->precision_bits);
                big_fixed_from_int(embed_dim_bf, (int)embed_dim);
                big_fixed_div(mean, mean, embed_dim_bf);
                
                // Compute variance
                BigFixed* var = big_fixed_create(training->precision_bits);
                big_fixed_from_int(var, 0);
                BigFixed* diff = big_fixed_create(training->precision_bits);
                BigFixed* diff_sq = big_fixed_create(training->precision_bits);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_sub(diff, layer_out[d], mean);
                    big_fixed_mul(diff_sq, diff, diff);
                    big_fixed_add(var, var, diff_sq);
                }
                big_fixed_div(var, var, embed_dim_bf);
                
                // Add epsilon and compute std
                BigFixed* epsilon = big_fixed_create(training->precision_bits);
                big_fixed_from_double(epsilon, 1e-5);
                big_fixed_add(var, var, epsilon);
                
                // std = sqrt(var) - use prime_sqrtf via double conversion
                double var_val = big_fixed_to_double(var);
                double std_val = prime_sqrtf((float)var_val);
                BigFixed* std = big_fixed_create(training->precision_bits);
                big_fixed_from_double(std, std_val);
                
                // Normalize: layer_out[d] = gamma[d] * (layer_out[d] - mean) / std + beta[d]
                BigFixed* normalized = big_fixed_create(training->precision_bits);
                BigFixed* scaled = big_fixed_create(training->precision_bits);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    // (layer_out[d] - mean) / std
                    big_fixed_sub(normalized, layer_out[d], mean);
                    big_fixed_div(normalized, normalized, std);
                    
                    // gamma[d] * normalized
                    big_fixed_mul(scaled, ln->gamma[d], normalized);
                    
                    // + beta[d]
                    big_fixed_add(layer_out[d], scaled, ln->beta[d]);
                }
                
                // Cleanup
                big_fixed_free(mean);
                big_fixed_free(var);
                big_fixed_free(diff);
                big_fixed_free(diff_sq);
                big_fixed_free(epsilon);
                big_fixed_free(std);
                big_fixed_free(normalized);
                big_fixed_free(scaled);
                big_fixed_free(embed_dim_bf);"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Replaced feedforward and LayerNorm with complete BigFixed implementation")
else:
    print("ERROR: Could not find exact match")
    # Try to find partial matches
    if "FeedForwardLayer* ff = &model->ff_layers[layer];" in content:
        print("Found: FeedForwardLayer* ff = &model->ff_layers[layer];")
    if "CLLMLayerNorm* ln = &model->layer_norms[layer];" in content:
        print("Found: CLLMLayerNorm* ln = &model->layer_norms[layer];")