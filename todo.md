# TODO - BigFixed Migration

## 🔒 RULES (PERMANENT - NEVER REMOVE)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

This creates a permanent loop ensuring rules are never forgotten.

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
**SECOND HIGHEST PRIORITY RULE**

Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

This ensures all work follows the architectural design.

### RULE 2: NO STUBS AND NO SIMPLIFICATION
**CRITICAL IMPLEMENTATION RULE**

- NO stub implementations
- NO simplified versions
- NO placeholder code
- NO partial solutions
- ONLY complete, proper implementations
- Use existing BigFixed operations from algorithms layer
- Babylonian mathematics does not require floating point or fractions
- This should be applicable to every single mathematical operation
- FULL IMPLEMENTATION REQUIRED

### RULE 3: INTRODUCING MORE ERRORS IS EXPECTED
**CRITICAL PERSPECTIVE SHIFT**

- Introducing more errors may be a factor of finding additional changes you need to make
- This is a really poor way to judge success when you have a massive project to rewrite
- Breaking things while fixing the architecture is DIFFERENT from making it worse
- Additional errors often just uncover additional changes that will need to be made
- The absolute purity and precision of the arbitrary math library is paramount
- Complete the changes and reevaluate each error as simply additional unforeseen work
- The absolute purity of the mathematics trumps everything else

### RULE 4: GIT AUTHENTICATION
**ALWAYS USE CORRECT AUTHENTICATION**

```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

---

## CURRENT TASK: COMPLETE BIGFIXED MIGRATION

### Phase 1: Forward Pass Conversion [COMPLETE] ✅

**OBJECTIVE:** Rewrite cllm_forward_training() to use BigFixed operations throughout.

**STATUS: COMPLETE - NO COMPROMISES. FULL IMPLEMENTATION. NO STUBS.**

#### Step 1: Fix Embedding Copy (Lines 1919-1923) [COMPLETE] ✅
Current code (WRONG):
```c
float* embed_src = &model->embeddings.embeddings[token_id * embed_dim];
float* embed_dst = &training->input_embeddings[idx * embed_dim];
memcpy(embed_dst, embed_src, embed_dim * sizeof(float));
```

This treats BigFixed** as float* - fundamentally broken.

Need to:
- [ ] Copy BigFixed* pointers element by element
- [ ] Use proper BigFixed** indexing
- [ ] No memcpy - manual loop required

#### Step 2: Fix Layer Input Copy (Line 1928)
Current code (WRONG):
```c
float* layer_input = training->input_embeddings;
memcpy(training->layer_inputs[layer], layer_input, batch_size * seq_len * embed_dim * sizeof(float));
```

Need to:
- [ ] Remove float* cast
- [ ] Copy BigFixed* pointers properly
- [ ] Update all references to layer_input

#### Step 3: Fix Attention Forward Pass (Lines 1930-1942)
Current code (WRONG):
```c
float* batch_input = &layer_input[start_idx * embed_dim];
float* batch_output = &training->attention_outputs[layer][start_idx * embed_dim];
```

Need to:
- [ ] Use BigFixed** indexing
- [ ] Update cllm_attention_forward_training() to accept BigFixed**
- [ ] Convert all attention operations to BigFixed

#### Step 4: Fix Feedforward Pass (Lines 1944-1968)
Current code (WRONG):
```c
float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
float* ff_out = &training->ff_outputs[layer][idx * embed_dim];
```

Need to:
- [ ] Use BigFixed** indexing
- [ ] Convert feedforward operations to BigFixed
- [ ] Use matrix_multiply_bigfixed() from algorithms layer

#### Step 5: Fix LayerNorm (Lines 1970-1987)
Current code (WRONG):
```c
layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];
```

Need to:
- [ ] Use layer_norm_bigfixed() from algorithms layer
- [ ] Convert mean/variance calculations to BigFixed
- [ ] Use big_fixed_sqrt() for std calculation

#### Step 6: Fix Final Projection (Lines 1993-2006)
Current code (WRONG):
```c
float* hidden = &training->final_hidden[idx * embed_dim];
float* logits = &training->logits[idx * vocab_size];
```

Need to:
- [ ] Use BigFixed** indexing
- [ ] Convert projection to BigFixed matrix multiply
- [ ] Use matrix_multiply_bigfixed()

### Phase 2: Backward Pass Conversion [IN PROGRESS]

**OBJECTIVE:** Rewrite cllm_backward_training() to use BigFixed operations throughout.

**STATUS:** Forward pass complete. Now converting backward pass.

**REMAINING ERRORS:** 13 errors in backward pass (lines 2270-2359)

All errors are type mismatches between BigFixed* and float operations in:
- LayerNorm backward (lines 2270-2281)
- Attention gradient accumulation (lines 2313-2323)
- Feedforward W2 gradients (lines 2337-2342)
- Feedforward W1 gradients (lines 2354-2359)

#### Step 1: Fix LayerNorm Backward (Lines 2265-2285) [NEXT]
Current code uses float arithmetic for gradient accumulation.
Need to convert to BigFixed operations.

#### Step 2: Fix Attention Gradients (Lines 2310-2330)
Current code: `training->attention_grads[layer].query_lattice[...] += ...`
Need to use big_fixed_add() for gradient accumulation.

#### Step 3: Fix Feedforward Gradients (Lines 2335-2365)
Current code: `training->ff_grads[layer].w2_lattice[...] += ...`
Need to use big_fixed_mul() and big_fixed_add() for gradient accumulation.

---

## EXECUTION LOG

### Session Start
- [x] Read MASTER_PLAN.md
- [x] Read AUDIT.md  
- [x] Read SECONDARY_OBJECTIVES.md
- [x] Updated todo.md with rules
- [x] Deep bidirectional analysis complete
- [ ] Begin forward pass conversion

### Current Focus
Starting with Step 1: Fix embedding copy operations.

**NO STUBS. NO SIMPLIFICATION. FULL IMPLEMENTATION.**