#!/usr/bin/env python3
import re

with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Fix the master_weights conversion - model->weights is already BigFixed**
old_code = '''// Convert float weights to BigFixed
                    if (model->weights) {
                        // Assuming model->weights is float* for now
                        // TODO: Update when model->weights is fully BigFixed
                        for (size_t i = 0; i < total_params; i++) {
                            big_fixed_from_double(training->master_weights[i], (double)model->weights[i]);
                        }
                    }'''

new_code = '''// Copy BigFixed weights to master_weights
                    if (model->weights) {
                        bigfixed_array_copy(training->master_weights, model->weights, total_params);
                    }'''

content = content.replace(old_code, new_code)

with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("✅ Fixed master_weights conversion")
