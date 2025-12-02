#!/bin/bash

# Fix all cllm_check_numerical_stability calls to use the _double version
# when passing double* arrays

sed -i 's/cllm_check_numerical_stability(model->embeddings\.embeddings/cllm_check_numerical_stability_double(model->embeddings.embeddings/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->attention_layers\[i\]\.query_lattice/cllm_check_numerical_stability_double(model->attention_layers[i].query_lattice/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->attention_layers\[i\]\.key_lattice/cllm_check_numerical_stability_double(model->attention_layers[i].key_lattice/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->attention_layers\[i\]\.value_lattice/cllm_check_numerical_stability_double(model->attention_layers[i].value_lattice/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->ff_layers\[i\]\.w1_lattice/cllm_check_numerical_stability_double(model->ff_layers[i].w1_lattice/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->ff_layers\[i\]\.w2_lattice/cllm_check_numerical_stability_double(model->ff_layers[i].w2_lattice/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->layer_norms\[i\]\.gamma/cllm_check_numerical_stability_double(model->layer_norms[i].gamma/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->layer_norms\[i\]\.beta/cllm_check_numerical_stability_double(model->layer_norms[i].beta/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->pos_encoding\.spiral_positions/cllm_check_numerical_stability_double(model->pos_encoding.spiral_positions/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->pos_encoding\.clock_positions/cllm_check_numerical_stability_double(model->pos_encoding.clock_positions/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->pos_encoding\.prime_positions/cllm_check_numerical_stability_double(model->pos_encoding.prime_positions/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(model->pos_encoding\.learned_positions/cllm_check_numerical_stability_double(model->pos_encoding.learned_positions/g' src/ai/cllm_validate.c
sed -i 's/cllm_check_numerical_stability(gradients/cllm_check_numerical_stability_double(gradients/g' src/ai/cllm_validate.c

echo "Fixed all validation calls"