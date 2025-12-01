#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Replace big_fixed_copy with big_fixed_assign
content = content.replace('big_fixed_copy(ff_hidden[h], ff->bias1[h]);', 
                          'big_fixed_assign(ff_hidden[h], ff->bias1[h]);')
content = content.replace('big_fixed_copy(ff_out[o], ff->bias2[o]);', 
                          'big_fixed_assign(ff_out[o], ff->bias2[o]);')

with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("SUCCESS: Fixed big_fixed_copy errors")