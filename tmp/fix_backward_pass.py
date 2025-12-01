#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the backward pass section
old_text = """    for (int layer = model->num_layers - 1; layer >= 0; layer--) {
        float* attn_output = training->attention_outputs[layer];
        float* ff_hidden = training->ff_hidden[layer];
        FeedForwardLayer* ff = &model->ff_layers[layer];
        CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* grad = &grad_layer[idx * embed_dim];
                float* input = &attn_output[idx * embed_dim];
                float* hidden = &ff_hidden[idx * ff->hidden_dim];
                
                // LayerNorm backward
                float mean = 0.0f, var = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) mean += input[d];
                mean /= embed_dim;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = input[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                float std = prime_sqrtf(var + 1e-5f);
                
                float grad_var = 0.0f, grad_mean = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float x_norm = (input[d] - mean) / std;
                    if (training->ln_grads[layer].gamma) {
                        training->ln_grads[layer].gamma[d] += grad[d] * x_norm;
                    }
                    if (training->ln_grads[layer].beta) {
                        training->ln_grads[layer].beta[d] += grad[d];
                    }
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad_var += grad_x_norm * (input[d] - mean) * -0.5f * prime_powf(std, -3.0f);
                    grad_mean += grad_x_norm * (-1.0f / std);
                }
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad[d] = grad_x_norm / std + grad_var * 2.0f * (input[d] - mean) / embed_dim + grad_mean / embed_dim;
                }"""

new_text = """    for (int layer = model->num_layers - 1; layer >= 0; layer--) {
        BigFixed** attn_output = training->attention_outputs[layer];
        BigFixed** ff_hidden = training->ff_hidden[layer];
        FeedForwardLayer* ff = &model->ff_layers[layer];
        CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                BigFixed** grad = &grad_layer[idx * embed_dim];
                BigFixed** input = &attn_output[idx * embed_dim];
                BigFixed** hidden = &ff_hidden[idx * ff->hidden_dim];
                
                // LayerNorm backward - BIGFIXED IMPLEMENTATION
                // Compute mean
                BigFixed* mean = big_fixed_create(training->precision_bits);
                big_fixed_from_int(mean, 0);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_add(mean, mean, input[d]);
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
                    big_fixed_sub(diff, input[d], mean);
                    big_fixed_mul(diff_sq, diff, diff);
                    big_fixed_add(var, var, diff_sq);
                }
                big_fixed_div(var, var, embed_dim_bf);
                
                // Compute std
                BigFixed* epsilon = big_fixed_create(training->precision_bits);
                big_fixed_from_double(epsilon, 1e-5);
                big_fixed_add(var, var, epsilon);
                double var_val = big_fixed_to_double(var);
                double std_val = prime_sqrtf((float)var_val);
                BigFixed* std = big_fixed_create(training->precision_bits);
                big_fixed_from_double(std, std_val);
                
                // Compute gradients for gamma and beta
                BigFixed* grad_var = big_fixed_create(training->precision_bits);
                BigFixed* grad_mean = big_fixed_create(training->precision_bits);
                big_fixed_from_int(grad_var, 0);
                big_fixed_from_int(grad_mean, 0);
                
                BigFixed* x_norm = big_fixed_create(training->precision_bits);
                BigFixed* temp = big_fixed_create(training->precision_bits);
                BigFixed* temp2 = big_fixed_create(training->precision_bits);
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    // x_norm = (input[d] - mean) / std
                    big_fixed_sub(x_norm, input[d], mean);
                    big_fixed_div(x_norm, x_norm, std);
                    
                    // Accumulate gamma gradient: gamma_grad += grad[d] * x_norm
                    if (training->ln_grads[layer].gamma) {
                        big_fixed_mul(temp, grad[d], x_norm);
                        big_fixed_add(training->ln_grads[layer].gamma[d], 
                                     training->ln_grads[layer].gamma[d], temp);
                    }
                    
                    // Accumulate beta gradient: beta_grad += grad[d]
                    if (training->ln_grads[layer].beta) {
                        big_fixed_add(training->ln_grads[layer].beta[d], 
                                     training->ln_grads[layer].beta[d], grad[d]);
                    }
                    
                    // grad_x_norm = grad[d] * gamma[d]
                    BigFixed* grad_x_norm = big_fixed_create(training->precision_bits);
                    big_fixed_mul(grad_x_norm, grad[d], ln->gamma[d]);
                    
                    // Accumulate variance gradient
                    // grad_var += grad_x_norm * (input[d] - mean) * -0.5 * std^(-3)
                    big_fixed_sub(temp, input[d], mean);
                    big_fixed_mul(temp, temp, grad_x_norm);
                    double std_pow = prime_powf((float)std_val, -3.0f);
                    big_fixed_from_double(temp2, -0.5 * std_pow);
                    big_fixed_mul(temp, temp, temp2);
                    big_fixed_add(grad_var, grad_var, temp);
                    
                    // Accumulate mean gradient
                    // grad_mean += grad_x_norm * (-1.0 / std)
                    big_fixed_from_double(temp, -1.0 / std_val);
                    big_fixed_mul(temp, grad_x_norm, temp);
                    big_fixed_add(grad_mean, grad_mean, temp);
                    
                    big_fixed_free(grad_x_norm);
                }
                
                // Propagate gradient through normalization
                for (uint32_t d = 0; d < embed_dim; d++) {
                    // grad_x_norm = grad[d] * gamma[d]
                    BigFixed* grad_x_norm = big_fixed_create(training->precision_bits);
                    big_fixed_mul(grad_x_norm, grad[d], ln->gamma[d]);
                    
                    // grad[d] = grad_x_norm / std + grad_var * 2.0 * (input[d] - mean) / embed_dim + grad_mean / embed_dim
                    BigFixed* term1 = big_fixed_create(training->precision_bits);
                    BigFixed* term2 = big_fixed_create(training->precision_bits);
                    BigFixed* term3 = big_fixed_create(training->precision_bits);
                    
                    // term1 = grad_x_norm / std
                    big_fixed_div(term1, grad_x_norm, std);
                    
                    // term2 = grad_var * 2.0 * (input[d] - mean) / embed_dim
                    big_fixed_sub(temp, input[d], mean);
                    big_fixed_from_double(temp2, 2.0);
                    big_fixed_mul(temp, temp, temp2);
                    big_fixed_mul(term2, grad_var, temp);
                    big_fixed_div(term2, term2, embed_dim_bf);
                    
                    // term3 = grad_mean / embed_dim
                    big_fixed_div(term3, grad_mean, embed_dim_bf);
                    
                    // grad[d] = term1 + term2 + term3
                    big_fixed_add(grad[d], term1, term2);
                    big_fixed_add(grad[d], grad[d], term3);
                    
                    big_fixed_free(grad_x_norm);
                    big_fixed_free(term1);
                    big_fixed_free(term2);
                    big_fixed_free(term3);
                }
                
                // Cleanup
                big_fixed_free(mean);
                big_fixed_free(var);
                big_fixed_free(diff);
                big_fixed_free(diff_sq);
                big_fixed_free(epsilon);
                big_fixed_free(std);
                big_fixed_free(grad_var);
                big_fixed_free(grad_mean);
                big_fixed_free(x_norm);
                big_fixed_free(temp);
                big_fixed_free(temp2);
                big_fixed_free(embed_dim_bf);"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Replaced LayerNorm backward with complete BigFixed implementation")
else:
    print("ERROR: Could not find exact match for LayerNorm backward")
    if "// LayerNorm backward" in content:
        print("Found: // LayerNorm backward")