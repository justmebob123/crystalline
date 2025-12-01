#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_crystalline_attention.c', 'r') as f:
    content = f.read()

old_text = """                  // Compute dot products using BigFixed (NO FLOATS!)
                  // For now, initialize to zero (proper implementation needs input_embeddings as BigFixed**)
                  big_fixed_from_int(q_sum, 0);
                  big_fixed_from_int(k_sum, 0);
                  big_fixed_from_int(v_sum, 0);
                  
                  // TODO: Implement proper dot product when input_embeddings is BigFixed**
                  // dot_product_bigfixed(weight_q, input_head, q_sum, head_dim, 64);"""

new_text = """                  // Compute dot products using BigFixed (NO FLOATS!)
                  // input_vec is float*, need to convert to BigFixed for computation
                  big_fixed_from_int(q_sum, 0);
                  big_fixed_from_int(k_sum, 0);
                  big_fixed_from_int(v_sum, 0);
                  
                  // Compute Q, K, V using BigFixed operations
                  BigFixed* temp = big_fixed_create(64);
                  BigFixed* input_bf = big_fixed_create(64);
                  
                  for (uint32_t i = 0; i < head_dim; i++) {
                      // Convert input to BigFixed
                      big_fixed_from_double(input_bf, (double)input_vec[h * head_dim + i]);
                      
                      // Q: q_sum += input[i] * weight_q[i,d]
                      uint32_t q_idx = (h * head_dim + i) * head_dim + d;
                      big_fixed_mul(temp, input_bf, layer->query_lattice[q_idx]);
                      big_fixed_add(q_sum, q_sum, temp);
                      
                      // K: k_sum += input[i] * weight_k[i,d]
                      uint32_t k_idx = (h * head_dim + i) * head_dim + d;
                      big_fixed_mul(temp, input_bf, layer->key_lattice[k_idx]);
                      big_fixed_add(k_sum, k_sum, temp);
                      
                      // V: v_sum += input[i] * weight_v[i,d]
                      uint32_t v_idx = (h * head_dim + i) * head_dim + d;
                      big_fixed_mul(temp, input_bf, layer->value_lattice[v_idx]);
                      big_fixed_add(v_sum, v_sum, temp);
                  }
                  
                  big_fixed_free(temp);
                  big_fixed_free(input_bf);"""

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('src/ai/cllm_crystalline_attention.c', 'w') as f:
        f.write(content)
    print("SUCCESS: Fixed crystalline attention Q,K,V computation")
else:
    print("ERROR: Could not find exact match")