#!/usr/bin/env python3
"""
Systematically fix all BigFixed/float type mismatches in cllm_training.c
This replaces float arithmetic with proper BigFixed operations
"""

with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Fix 1 & 2: FeedForward first layer (lines 1955-1961)
old_ff1 = '''                   for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                       float sum = ff->bias1[h];
                       for (uint32_t i = 0; i < embed_dim; i++) {
                           sum += attn_out[i] * ff->w1_lattice[i * ff->hidden_dim + h];
                       }
                       ff_hidden[h] = prime_tanhf(sum);
                   }'''

new_ff1 = '''                   for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                       // PROPER BIGFIXED: W1 * input + bias1
                       BigFixed* sum = big_fixed_create(training->precision_bits);
                       big_fixed_assign(sum, ff->bias1[h]);
                       
                       for (uint32_t i = 0; i < embed_dim; i++) {
                           BigFixed* attn_bf = big_fixed_create(training->precision_bits);
                           BigFixed* prod = big_fixed_create(training->precision_bits);
                           big_fixed_from_double(attn_bf, (double)attn_out[i]);
                           big_fixed_mul(prod, attn_bf, ff->w1_lattice[i * ff->hidden_dim + h]);
                           big_fixed_add(sum, sum, prod);
                           big_fixed_free(attn_bf);
                           big_fixed_free(prod);
                       }
                       
                       BigFixed* activated = big_fixed_create(training->precision_bits);
                       bigfixed_tanh(activated, sum, training->precision_bits);
                       ff_hidden[h] = (float)big_fixed_to_double(activated);
                       big_fixed_free(sum);
                       big_fixed_free(activated);
                   }'''

content = content.replace(old_ff1, new_ff1)

# Fix 3 & 4: FeedForward second layer (lines 1963-1969)
old_ff2 = '''                   for (uint32_t o = 0; o < embed_dim; o++) {
                       float sum = ff->bias2[o];
                       for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                           sum += ff_hidden[h] * ff->w2_lattice[h * embed_dim + o];
                       }
                       ff_out[o] = sum;
                   }'''

new_ff2 = '''                   for (uint32_t o = 0; o < embed_dim; o++) {
                       // PROPER BIGFIXED: W2 * hidden + bias2
                       BigFixed* sum = big_fixed_create(training->precision_bits);
                       big_fixed_assign(sum, ff->bias2[o]);
                       
                       for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                           BigFixed* hidden_bf = big_fixed_create(training->precision_bits);
                           BigFixed* prod = big_fixed_create(training->precision_bits);
                           big_fixed_from_double(hidden_bf, (double)ff_hidden[h]);
                           big_fixed_mul(prod, hidden_bf, ff->w2_lattice[h * embed_dim + o]);
                           big_fixed_add(sum, sum, prod);
                           big_fixed_free(hidden_bf);
                           big_fixed_free(prod);
                       }
                       
                       ff_out[o] = (float)big_fixed_to_double(sum);
                       big_fixed_free(sum);
                   }'''

content = content.replace(old_ff2, new_ff2)

# Fix 5: LayerNorm (line 1985)
old_ln = '                       layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];'

new_ln = '''                       // PROPER BIGFIXED: gamma * (x - mean) / std + beta
                       BigFixed* temp = big_fixed_create(training->precision_bits);
                       BigFixed* result = big_fixed_create(training->precision_bits);
                       big_fixed_from_double(temp, (layer_out[d] - mean) / std);
                       big_fixed_mul(result, ln->gamma[d], temp);
                       big_fixed_add(result, result, ln->beta[d]);
                       layer_out[d] = (float)big_fixed_to_double(result);
                       big_fixed_free(temp);
                       big_fixed_free(result);'''

content = content.replace(old_ln, new_ln)

with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("Fixed FeedForward and LayerNorm (5 errors)")