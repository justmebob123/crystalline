# MASTER PLAN RULES (ALWAYS FOLLOW)
1. ✅ Always reread MASTER_PLAN.md before major changes
2. ✅ Reference AUDIT.md for architectural context
3. ✅ Reference SECONDARY_OBJECTIVES.md for priorities
4. ✅ Never create new .md files (use existing ones)
5. ✅ Commit and push after each major milestone
6. ✅ Follow the established architecture

## TODO: Fix BigFixed Migration - THE RIGHT WAY

### WHAT I DID WRONG
- [x] Read all three documents (MASTER_PLAN, AUDIT, SECONDARY_OBJECTIVES)
- [x] Understand the mistake: I created NEW files instead of fixing EXISTING files
- [x] The solution was simple: Use existing BigFixed functions in existing CLLM files
- [x] I violated the architecture by creating parallel implementations

### THE CORRECT APPROACH (from SECONDARY_OBJECTIVES.md)
All BigFixed operations ALREADY EXIST in algorithms library:
- matrix_multiply_bigfixed()
- dot_product_bigfixed()
- layer_norm_bigfixed()
- cross_entropy_loss_bigfixed()
- softmax_bigfixed()
- adam_step_bigfixed()
- bigfixed_exp(), bigfixed_tanh(), etc.

I just need to USE THEM in the existing CLLM files!

### PHASE 1: Delete Incorrectly Created Files ✅ COMPLETE
- [x] Delete src/ai/cllm_attention_bigfixed.c (163 lines)
- [x] Delete src/ai/cllm_feedforward_bigfixed.c (78 lines)
- [x] Delete src/ai/cllm_layernorm_bigfixed.c (34 lines)
- [x] Delete src/ai/cllm_training_bigfixed.c (274 lines)
- [x] Delete src/ai/cllm_training_stubs.c
- [x] Delete src/ai/cllm_inference_stubs.c
- [x] Delete include/cllm_bigfixed.h
- [x] Delete algorithms/include/numerical_bigfixed.h
- [x] Delete algorithms/include/loss_functions_bigfixed.h
- [x] Delete algorithms/include/optimizers_bigfixed.h

### PHASE 2: Restore Renamed Files ✅ COMPLETE
- [x] Rename cllm_crystalline_attention.c.old back to .c
- [x] Rename cllm_inference.c.old back to .c
- [x] Rename cllm_init.c.old back to .c
- [x] Rename cllm_layernorm.c.old back to .c
- [x] Rename cllm_optimizer.c.old back to .c
- [x] Rename cllm_optimizer_wrapper.c.old back to .c
- [x] Rename cllm_production.c.old back to .c
- [x] Rename cllm_training.c.old back to .c

### PHASE 3: Fix Attention (cllm_crystalline_attention.c)
- [ ] Replace lines 386-388 float arithmetic with BigFixed operations
- [ ] Use dot_product_bigfixed() instead of manual float loops
- [ ] Use existing BigFixed functions from algorithms library

### PHASE 4: Fix Feedforward (cllm_feedforward.c if exists)
- [ ] Use matrix_multiply_bigfixed()
- [ ] Use bigfixed_tanh()

### PHASE 5: Fix Layer Norm (cllm_layernorm.c)
- [ ] Use layer_norm_bigfixed()

### PHASE 6: Fix Training (cllm_training.c)
- [ ] Use cross_entropy_loss_bigfixed()
- [ ] Use adam_step_bigfixed()

### PHASE 7: Update Makefile
- [ ] Remove references to deleted files
- [ ] Ensure proper linking to algorithms library

### PHASE 8: Build and Test
- [ ] make clean && make
- [ ] Verify no NaN errors
- [ ] Test training pipeline

### PHASE 9: Commit and Push
- [ ] Commit all changes with clear message
- [ ] Push to repository