#!/bin/bash

echo "=== COMPREHENSIVE FLOAT/DOUBLE ANALYSIS ==="
echo ""

echo "1. CRITICAL: Training pipeline float usage"
grep -n "float\*\|float " src/ai/cllm_training.c | head -20

echo ""
echo "2. CRITICAL: Inference pipeline float usage"
grep -n "float\*\|float " src/ai/cllm_inference.c | head -20

echo ""
echo "3. Layer operations"
grep -n "float\*\|float " src/ai/cllm_layernorm.c src/ai/cllm_feedforward.c 2>/dev/null | head -20

echo ""
echo "4. Attention mechanism"
grep -n "float\*\|float " src/ai/cllm_attention.c src/ai/cllm_angular_attention.c 2>/dev/null | head -20

echo ""
echo "5. SIMD operations"
grep -n "float\*\|float " src/ai/cllm_simd_utils.c src/ai/cllm_simd_gradient_ops.c | head -20

echo ""
echo "6. Header files"
grep -n "float\*\|float " include/cllm_inference.h include/cllm.h 2>/dev/null | head -30

