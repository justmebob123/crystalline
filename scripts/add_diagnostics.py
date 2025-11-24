#!/usr/bin/env python3
"""
Add diagnostic output to training loop
"""

def main():
    with open('src/ai/cllm_training.c', 'r') as f:
        lines = f.readlines()
    
    # Find the line with "if (tokens == 0) break;"
    for i, line in enumerate(lines):
        if 'if (tokens == 0) break;' in line:
            # Add weight diagnostics after this line
            weight_diag = """        
        // DIAGNOSTIC: Check weight initialization (first batch only)
        if (training->current_epoch == 0 && num_batches == 0) {
            float sum_embed = 0.0f, sum_attn = 0.0f, sum_ff = 0.0f;
            int count = 100;
            
            for (int i = 0; i < count; i++) {
                sum_embed += fabsf(model->embeddings.embeddings[i]);
            }
            
            if (model->attention_layers && model->attention_layers[0].query_lattice) {
                for (int i = 0; i < count; i++) {
                    sum_attn += fabsf(model->attention_layers[0].query_lattice[i]);
                }
            }
            
            if (model->ff_layers && model->ff_layers[0].w1_lattice) {
                for (int i = 0; i < count; i++) {
                    sum_ff += fabsf(model->ff_layers[0].w1_lattice[i]);
                }
            }
            
            printf("  Weight magnitudes: embed=%.6f, attn=%.6f, ff=%.6f\\n",
                   sum_embed/count, sum_attn/count, sum_ff/count);
        }
        
"""
            lines.insert(i+1, weight_diag)
            break
    
    # Find the line with "cllm_backward_training"
    for i, line in enumerate(lines):
        if 'cllm_backward_training(training, target_tokens);' in line:
            # Add gradient diagnostics after this line
            grad_diag = """        
        // DIAGNOSTIC: Check gradient magnitudes
        if (num_batches == 1 || num_batches % 5 == 0) {
            float max_embed_grad = 0.0f, sum_embed_grad = 0.0f;
            int nonzero_embed = 0;
            size_t embed_size = model->vocab_size * model->embedding_dim;
            
            for (size_t i = 0; i < embed_size && i < 10000; i++) {
                float g = fabsf(training->gradients[i]);
                if (g > 1e-10f) {
                    nonzero_embed++;
                    sum_embed_grad += g;
                    if (g > max_embed_grad) max_embed_grad = g;
                }
            }
            
            float max_attn_grad = 0.0f;
            int nonzero_attn = 0;
            if (training->attention_grads && model->num_layers > 0) {
                size_t size = model->embedding_dim * model->embedding_dim;
                for (size_t i = 0; i < size && i < 10000; i++) {
                    float g = fabsf(training->attention_grads[0].query_lattice[i]);
                    if (g > 1e-10f) {
                        nonzero_attn++;
                        if (g > max_attn_grad) max_attn_grad = g;
                    }
                }
            }
            
            printf("    Gradients: embed=%d (max=%.2e, avg=%.2e), attn=%d (max=%.2e)\\n",
                   nonzero_embed, max_embed_grad,
                   nonzero_embed > 0 ? sum_embed_grad / nonzero_embed : 0.0f,
                   nonzero_attn, max_attn_grad);
        }
        
"""
            lines.insert(i+1, grad_diag)
            break
    
    with open('src/ai/cllm_training.c', 'w') as f:
        f.writelines(lines)
    
    print("✓ Added diagnostic output")
    return 0

if __name__ == '__main__':
    exit(main())